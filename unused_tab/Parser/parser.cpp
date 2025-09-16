#include "nodes.hpp"
#include "parser.hpp"
#include "../Lexer/token.hpp"
#include "../../../HelperFunctions.hpp"
#include <iostream>
#include <unordered_set>
#include <print>

/*

    ⚠️TRIGGER WARNING!⚠️
    This code may annoy you so much you'll combust,
    because it was made by a high school graduate that doesn't know C++ well yet.

    Please proceed with caution.

*/
void Parser::printModule() {
    // as empty as my meaning of life
}

// ==== Main parsing ====
MemoryPtr<ModuleNode> Parser::parseModule() {
    /*
    создать узел ModuleNode
    пока не достиг конца:
        прочитай узел stmt с типом ASTNode
        если узла нет:
            ошибка
        иначе:
            добавить узел stmt в узел ModuleNode
    вернуть ModuleNode
    */
    auto moduleNode = makeMemoryPtr<ModuleNode>();

    while (!isAtEnd()) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in module");
            break;
        }

        moduleNode->body.push_back(std::move(stmt));
    }

    return moduleNode;
}

// ==== Statement parsing ====
MemoryPtr<ASTNode> Parser::parseStatement() {
    /* (это отстой)
    получить токен
    если токен имеет тип Keyword:
        если токен if -> обработай условие if
        если токен switch -> обработай поиск switch
        если токен try -> прочитай блок и поймай ошибку
        если токен for -> пройдись по циклу n-раз
        если токен while -> пройдись по циклу пока условие не выполнилось
        если токен return -> оформи возврат значения
        если токен throw -> вызови ошибку программы и уничтожение
        если токен break -> вызови остановку цикла
        если токен continue -> продолжи с следующей итерации цикла
        если токен fn или function -> пройдись по функции
        если токен class -> пройдись по классу
        если токен # -> обработай препроцессор
        если токен @ -> обработай декоратор
    
    если токен это идентификатор:
        посмотри следующий токен
        если это оператор -> проведи операцию присвоения

        НЕТУ ОБРАБОТКИ ФУНКЦИЙ И ПРИСВОЕНИЕ ПЕРЕМЕННЫХ, И ДРУГОЕ!!!
    
    если это блок и начинается с { -> отпарси блок

    иначе спарси бинарную операцию
    */
    Token token = curToken();

    // === Control Flow Keywords ===
    if (token.type == TokenType::Keyword) {
        if (token.value == "if") return parseIf();
        if (token.value == "switch") return parseSwitch();
        if (token.value == "try") return parseTryCatch();
        if (token.value == "for") return parseFor();
        if (token.value == "while") return parseWhile();

        if (token.value == "return") {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Neoluma/Parser] Expected expression after 'return'";
                return nullptr;
            }
            return makeMemoryPtr<ReturnStatementNode>(expr);
        }

        if (token.value == "throw") {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Neoluma/Parser] Expected expression after 'throw'";
                return nullptr;
            }
            return makeMemoryPtr<ThrowStatementNode>(expr);
        }

        if (token.value == "break") {
            next();
            return makeMemoryPtr<BreakStatementNode>();
        }

        if (token.value == "continue") {
            next();
            return makeMemoryPtr<ContinueStatementNode>();
        }

        if (token.value == "fn" || token.value == "function") return parseFunction();
        if (token.value == "class") return parseClass();
        if (token.value == "import") return parseImport();
        if (token.value == "#") return parsePreprocessor();
        if (token.value == "@") return parseDecorator();
    }

    // === Assignments ===
    if (token.type == TokenType::Identifier) {
        Token nextTok = lookupNext();

        if (nextTok.type == TokenType::Operator && (
            nextTok.value == "=" || nextTok.value == "+=" || nextTok.value == "-=" ||
            nextTok.value == "*=" || nextTok.value == "/=" || nextTok.value == "%=" ||
            nextTok.value == "^=")) {
            return parseAssignment();
        }

        // TODO: Maybe function call? detect "(" after identifier
        // TODO: Variable declaration later?
    }

    // === Block ===
    if (token.type == TokenType::Delimeter && token.value == "{") {
        return parseBlock();
    }

    // === Fallback ===
    return parseExpression(); // Default to expression statement
}

// ==== Expression parsing ====
MemoryPtr<ASTNode> Parser::parsePrimary(){
    /*
    получить токен
    если это оператор отрицания, т.е. унарный:
        оператор является значением токена
        следующий токен
        вернуть парсинг унарных выражений
    иначе если токен это разделитель и начинается с ( :
        пропускаем (
        парсим выражение внутри
        если не закрывается скобка с ), ошибка
        следующий токен (???)
        возврат выражения
    иначе если токен или число или строка или логическое выражение или ничего:
        пропуск (???)
        создаем и возвращаем значение буквальное
    иначе если это идентификатор или переменная:
        пропускаем токен

        если скобка открывается:
            пропуск
            создаем список аргументов из узлов ParameterNode
            пока скобка не закрылась и программа не закончилась:
                парсим бинарное выражение
                если оно существует, добавляем как ParameterNode в список аргументов

                если есть запятая, дальше
                иначе цикл ломается
            если скобка не закрыта, ошибка

            создаем VariableNode (???)
            возвращаем CallExpressionNode VariableNode'а (???)
        возвращаем создание идентификатора (???)
    если ничего не подошло, значит парсить нечего, возвращай ошибку
    */
    Token token = curToken();

    // Unary operations
    if (token.type == TokenType::Operator && 
        (token.value == "!" || token.value == "-" || token.value == "not")) {
        std::string op = token.value;
        next();
        return parseUnary(op);
    } 

    // Parenthesis
    else if (token.type == TokenType::Delimeter && token.value == "(") {
        next();
        MemoryPtr<ASTNode> expr = parseExpression();
        if (!match(TokenType::Delimeter, ")")) {
            std::println(std::cerr, "[Neoluma/Parser] Expected ')' after expression");
            return nullptr;
        }
        next();
        return expr;
    } 
    // Data type
    else if (token.type == TokenType::Number || token.type == TokenType::String || 
               token.type == TokenType::Boolean || token.type == TokenType::NullLiteral) {
        next();
        return makeMemoryPtr<LiteralNode>(token.value);
    } 
    // Identifier or variable
    else if (token.type == TokenType::Identifier) {
        Token id = next();
        
        if (match(TokenType::Delimeter, "(")) {
            next();
            std::vector<MemoryPtr<ParameterNode>> args;

            while (!match(TokenType::Delimeter, ")") && !isAtEnd()) {
                auto arg = parseExpression();
                if (arg) args.push_back(makeMemoryPtr<ParameterNode>(std::move(arg)));
                
                if (match(TokenType::Delimeter, ",")) next();
                else break;
            }
            if (!match(TokenType::Delimeter, ")")) {
                std::println(std::cerr, "[Neoluma/Parser] Expected ')' after arguments");
                return nullptr;
            }

            auto callee = makeMemoryPtr<VariableNode>(id.value); // Variable or identifier
            return makeMemoryPtr<CallExpressionNode>(std::move(callee), args);
        }

        return makeMemoryPtr<VariableNode>(id.value); // Variable or identifier
    }

    std::println(std::cerr, "[Neoluma/Parser] Unexpected token in primary expression: {}", token.value);
    return nullptr;
}

MemoryPtr<ASTNode> Parser::parseExpression() {
    /*
    Просто парсинг бинарной операции. Не совсем хороший вариант...
    */
    return parseBinary(0);
}

MemoryPtr<ASTNode> Parser::parseBinary(int prevPredecence = 0) {
    /*
    парсинг левого выражения
    если его нет, возвращаем ничего (???)

    бесконечный цикл:
        берем токен оператора
        смотрим какой оператор по приоритету
        если это не оператор или текущий приоритет меньше прошлого, ломай цикл
        парсим оператор
        далее
        парсим рекурсивно бинарные выражения справа, парся бинарную операцию с приоритетом +1
        если правого выражения нет, возвращаем ничего (???)

        создаем узел BinaryOperationNode с выражением левого, оператора, правого разрядов
        двигаем бинарное выражение в левое выражение
    возвращаем левое выражение
    */
    MemoryPtr<ASTNode> left = parsePrimary();
    if (!left) return nullptr;

    while (true) {
        Token token = curToken();
        int predecence = getOperatorPrecedence(token.value);
        if (token.type != TokenType::Operator || predecence < prevPredecence) break;
        std::string op = token.value;
        next();

        MemoryPtr<ASTNode> right = parseBinary(predecence+1);
        if (!right) return nullptr;

        MemoryPtr<BinaryOperationNode> binaryOpNode = makeMemoryPtr<BinaryOperationNode>(std::move(left), op, std::move(right));
        left = std::move(binaryOpNode);
    }
    return left;
}

MemoryPtr<UnaryOperationNode> Parser::parseUnary(const std::string& op) {
    /*
    Парсим первичное выражение, операнд
    если операнда нет, ошибку что отсутствует унарный оператор (???)
    Возвращаем узел UnaryOperationNode
    */
    MemoryPtr<ASTNode> operand = parsePrimary();
    if (!operand) {
        std::println(std::cerr, "[Neoluma/Parser] Missing operand after unary operator: {}", op);
        return nullptr;
    }
    return makeMemoryPtr<UnaryOperationNode>(op, std::move(operand));
}

MemoryPtr<AssignmentNode> Parser::parseAssignment(){
    /*
    берем токен
    если это не идентификатор, то ошибка (??? а что с variable???)
    вычленяем имя из идентификатора
    далее
    операторы присвоения в множестве (??? нахера)
    берем токен
    если это не оператор или оператор не найден в множестве, то ошибка
    вычленяем оператор
    далее
    парсим бинарную операцию (???)
    если ничего нет, то ошибка

    возвращаем AssignmentNode с названием, оператором и значением
    */
    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected identifier for assignment");
        return nullptr;
    }
    std::string variableName = token.value;
    next();

    static const std::unordered_set<std::string> assignmentOps = {
        "=", "+=", "-=", "*=", "/=", "%=", "^="
    };

    token = curToken();
    if ((!match(TokenType::Operator)) || assignmentOps.find(token.value) == assignmentOps.end()) {
        std::println(std::cerr, "[Neoluma/Parser] Expected assignment operator (=, +=, -=, and etc.)");
        return nullptr;
    }

    std::string op = token.value;
    next();

    MemoryPtr<ASTNode> value = parseExpression();
    if (!value) {
        std::println(std::cerr, "[Neoluma/Parser] Failed to parse expression after assignment");
        return nullptr;
    }

    return makeMemoryPtr<AssignmentNode>(variableName, op, std::move(value));
}

MemoryPtr<ReturnStatementNode> Parser::parseReturn(){
    /*
    пропускаем
    если текущий тип это ;, то возвращаем ничего в ReturnStatementNode (???)
    парсим бинарную операцию
    если ее нет, то ошибка
    если return не заканчивается ;, то ошибка (???)
    далее
    возвращаем ReturnStatementNode
    */
    next();

    if (curToken().type == TokenType::Delimeter && curToken().value == ";") {
        return makeMemoryPtr<ReturnStatementNode>(nullptr);
    }

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid expression in return statement");
        return nullptr;
    }

    if (curToken().type != TokenType::Delimeter || curToken().value != ";") {
        std::println(std::cerr, "[Neoluma/Parser] Expected ';' after return expression");
        return nullptr;
    }
    next();

    return makeMemoryPtr<ReturnStatementNode>(std::move(expr));
}

// ==== Control flow ====
MemoryPtr<IfNode> Parser::parseIf() {
    /*
    пропускаем
    если скобки нет, ошибка
    далее
    парсим условие (бинарное выражение) (???)
    если условия нет, ошибка
    если условие не закрыто скобкой, ошибка

    парсим блок или утверждение в ifBlock
    делаем пустой elseBlock

    если есть else:
        двигаем
        парсим блок или утверждение в elseBlock
    возвращаем IfNode с условием и двумя блоками
    */
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'if'");
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser] Expected condition expression after 'if'");
        return nullptr;
    }
    if (!match(TokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after condition in 'if'");
        return nullptr;
    }

    MemoryPtr<ASTNode> ifBlock = parseBlockorStatement();
    MemoryPtr<ASTNode> elseBlock = nullptr;

    if (match(TokenType::Keyword, "else")) {
        next();
        elseBlock = parseBlockorStatement();
    }

    return makeMemoryPtr<IfNode>(condition, ifBlock, elseBlock);
}

MemoryPtr<SwitchNode> Parser::parseSwitch() {
    /*
    пропускаем
    если скобки открывающейся нет, ошибка
    далее
    парсим бинарное выражение (???)
    если его нет, ошибка
    если закрывающейся скобки нет, ошибка
    если открывающейся { скобки нет, ошибка
    далее
    создаем список CaseNode 
    создаем пустое условие по умолчанию SCDefaultNode

    пока не закрыто через }:
        берем токен
        если слово case:
            пропускаем
            парсим условие (???)
            если нет :, ошибка
            далее
            делаем ноду блока (??? а почему так? это требует `case: {}`)
            добавляем в кейсы ноду с условием и блоком
        иначе если слово default:
            пропуск
            если нет :, ошибка
            далее
            делаем ноду блока (???)
            добавляем в кейсы ноду с условием и блоком
        иначе ошибка
    возвращаем SwitchNode с выражением, кейсами и дефолтом
    */
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'switch'");
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid expression in switch");
        return nullptr;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after switch expression");
        return nullptr;
    }
    next();

    if (!match(TokenType::Delimeter, "{")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{' after switch");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase = nullptr;

    while (!match(TokenType::Delimeter, "}")) {
        Token tok = curToken();

        if (tok.type == TokenType::Keyword && tok.value == "case") {
            next();
            MemoryPtr<ASTNode> condition = parseExpression();
            if (!match(TokenType::Delimeter, ":")) {
                std::println(std::cerr, "[Neoluma/Parser] Expected ':' after case condition");
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            cases.push_back(makeMemoryPtr<CaseNode>(condition, body));
        }
        else if (tok.type == TokenType::Keyword && tok.value == "default") {
            next();
            if (!match(TokenType::Delimeter, ":")) {
                std::println(std::cerr, "[Neoluma/Parser] Expected ':' after default");
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            defaultCase = makeMemoryPtr<SCDefaultNode>(body);
        }
        else {
            std::println(std::cerr, "[Neoluma/Parser] Unexpected token in switch");
            return nullptr;
        }
    }

    return makeMemoryPtr<SwitchNode>(expr, cases, defaultCase);
}

MemoryPtr<TryCatchNode> Parser::parseTryCatch() {
    /*
    пропуск
    парсим блок или утверждение try
    если нет блока, ошибка
    если нет catch, ошибка
    далее
    парсим блок или утверждение catch
    если нет блока, ошибка
    возвращаем TryCatchNode с блоками
    */
    next();

    auto tryBlock = as<BlockNode>(parseBlockorStatement());
    if (!tryBlock) {
        std::println(std::cerr, "[Neoluma/Parser] Expected try block");
        return nullptr;
    }

    if (!match(TokenType::Keyword, "catch")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected 'catch' after 'try'");
        return nullptr;
    }
    next();

    auto catchBlock = as<BlockNode>(parseBlockorStatement());
    if (!catchBlock) {
        std::println(std::cerr, "[Neoluma/Parser] Expected catch block");
        return nullptr;
    }

    return makeMemoryPtr<TryCatchNode>(tryBlock, catchBlock);
}

MemoryPtr<ForLoopNode> Parser::parseFor() {
    /*
    пропускаем
    если нет скобки, ошибка
    далее
    если нет идентификатора (???), ошибка
    записываем значение variable
    делаем ноду Variable с типом Void (???)
    далее
    если нет оператора :, ошибка
    парсим бинарное выражение (???)
    если нет выражения, ошибка
    если условие не закрыто ), ошибка
    парсим блок или утверждение
    если блока нет, ошибка
    делаем ForLoopNode c переменной, итеративным элементом и блоком выполнения
    */
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'for'");
        return nullptr;
    }
    next();

    if (curToken().type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected identifier in for loop");
        return nullptr;
    }

    std::string varName = curToken().value;
    auto varNode = makeMemoryPtr<VariableNode>(varName, ASTVariableType::Void);
    next();

    if (!match(TokenType::Operator, ":")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ':' after loop variable in for");
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid iterable in for loop");
        return nullptr;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after for condition");
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        std::println(std::cerr, "[Neoluma/Parser] Expected block or statement after for");
        return nullptr;
    }

    return makeMemoryPtr<ForLoopNode>(std::move(varNode), std::move(iterable), std::move(body));
}

MemoryPtr<WhileLoopNode> Parser::parseWhile() {
    /*
    пропускаем
    если нет скобки, ошибка
    далее
    парсим бинарное выражение (???)
    если нет выражения, ошибка
    если условие не закрыто ), ошибка
    парсим блок или утверждение
    если блока нет, ошибка
    делаем WhileLoopNode c условием и блоком выполнения
    */
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'while'");
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid condition in while loop");
        return nullptr;
    }
    if (!match(TokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after while condition");
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        std::println(std::cerr, "[Neoluma/Parser] Expected block or statement after while");
        return nullptr;
    }

    return makeMemoryPtr<WhileLoopNode>(std::move(condition), std::move(body));
}

// ==== Declarations ====
MemoryPtr<FunctionNode> Parser::parseFunction() {
    /*
    пропускаем
    берем токен
    если токен не идентификатор, ошибка
    вычленяем имя функции
    дальше
    если нет открывающейся скобки, ошибка
    дальше
    создаем список узлов параметров ParameterNode
    пока не закрывается скобка:
        берем имя параметра
        если параметр не является идентификатором, то ошибка
        далее
        тип параметра - void (надо добавить детекцию)
        добавляем ParameterNode c переменной и типом в список параметров
        если есть запятая, продолжаем парсить следующий.
        иначе ломаем цикл
    если нет закрывающейся скобки, ошибка
    далее
    парсим тело функции BlockNode
    если тела нет, возвращаем ничего (??? пусть ошибка будет, почему?)
    возвращаем FunctionNode с именем, параметрами и телом функции
    */
    next();

    Token nameToken = curToken();
    if (nameToken.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected function name");
        return nullptr;
    }
    std::string funcName = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after function name");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    while (!match(TokenType::Delimeter, ")")) {
        Token paramName = curToken();
        if (paramName.type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser] Expected parameter name");
            return nullptr;
        }
        next();

        ASTVariableType type = ASTVariableType::Void; // TODO: detect type later
        params.push_back(makeMemoryPtr<ParameterNode>(paramName.value, type));

        if (match(TokenType::Delimeter, ",")) next();
        else break;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after function parameters");
        return nullptr;
    }
    next();

    MemoryPtr<BlockNode> body = parseBlock();
    if (!body) return nullptr;

    return makeMemoryPtr<FunctionNode>(funcName, params, body);
}

MemoryPtr<ClassNode> Parser::parseClass() {
    /*
    пропускаем
    получаем токен
    если токен не идентификатор, ошибка
    сохраняем имя класса
    далее
    если нет {, ошибка (??? так это блок по факту, с хера ли?)
    далее
    создаем список полей в классе (VariableNode)
    создаем список методов в классе (FunctionNode)
    пока не закрытая фигурная скобка:
        берем токен
        если начинается с кейворда функции:
            парсим функцию
            добавляем в список методов (??? а проверки нет что если краш парсинга функции)
        иначе если это идентификатор:
            берем имя
            далее
            присваиваем дефолтное значение void (надо исправить)
            добавляем в список полей.
            если есть ;, идем дальше (??? обработки нет ошибки, а если она крашнется?)
        иначе ошибка
    далее
    возвращаем узел ClassNode с названием, полями и методами
    */
    next();

    Token nameToken = curToken();
    if (nameToken.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected class name");
        return nullptr;
    }
    std::string className = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, "{")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{' to start class body");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<VariableNode>> fields;
    std::vector<MemoryPtr<FunctionNode>> methods;

    while (!match(TokenType::Delimeter, "}")) {
        Token t = curToken();

        if (t.type == TokenType::Keyword && t.value == "function") {
            auto method = parseFunction();
            if (method) methods.push_back(std::move(method));
        }
        else if (t.type == TokenType::Identifier) {
            std::string fieldName = t.value;
            next();
            ASTVariableType varType = ASTVariableType::Void;
            fields.push_back(makeMemoryPtr<VariableNode>(fieldName, varType));
            if (match(TokenType::Delimeter, ";")) next();
        }
        else {
            std::println(std::cerr, "[Neoluma/Parser] Unexpected token in class body");
            break;
        }
    }

    next();
    return makeMemoryPtr<ClassNode>(className, fields, methods);
}

MemoryPtr<BlockNode> Parser::parseBlock() {
    /*
    если не начинается с {, ошибка
    далее
    создаем BlockNode
    пока не закрывается через }:
        парсим утверждения
        если их нет, ошибка (??? до этого точно дойдет?)
        добавляем в блок утверждение
        если заканчивается через ;, двигаем дальше
    пропускаем закрывающуюся скобку
    возвращаем блок
    */
    if (!match(TokenType::Delimeter, "{")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{' to start block");
        return nullptr;
    }
    next();

    auto block = makeMemoryPtr<BlockNode>();

    while (!match(TokenType::Delimeter, "}")) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in block");
            return nullptr;
        }
        block->statements.push_back(std::move(stmt));
        if (match(TokenType::Delimeter, ";")) next();
    }

    next();
    return block;
}

MemoryPtr<DecoratorNode> Parser::parseDecorator() {
    /*
    пропускаем @
    если нет идентификатора, ошибка
    берем имя
    далее
    создаем список узлов параметров ParameterNode
    если начинается с (:
        пропускаем токен
        пока не закрывается с ):
            берем токен
            если это не идентификатор, ломаем цикл
            добавляем в список параметр с типом Void (опять)
            далее
            если есть запятая, продолжаем
            иначе ломает цикл
        если скобка не закрыта, ошибка
        далее
    возвращаем ноду декоратора с именем, параметрами, и...что за херня?! это не закончено, как я мог упустить такое?!
    */
    next(); // consume '@'

    Token nameToken = curToken();
    if (nameToken.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected decorator name");
        return nullptr;
    }
    std::string name = nameToken.value;
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    if (match(TokenType::Delimeter, "(")) {
        next(); // consume '('
        while (!match(TokenType::Delimeter, ")")) {
            Token p = curToken();
            if (p.type != TokenType::Identifier) break;
            ASTVariableType dummy = ASTVariableType::Void;
            params.push_back(makeMemoryPtr<ParameterNode>(p.value, dummy));
            next();
            if (match(TokenType::Delimeter, ",")) next();
            else break;
        }
        if (!match(TokenType::Delimeter, ")")) {
            std::println(std::cerr, "[Neoluma/Parser] Unterminated decorator params");
            return nullptr;
        }
        next(); // consume ')'
    }

    return makeMemoryPtr<DecoratorNode>(name, params, nullptr);
}

MemoryPtr<PreprocessorDirectiveNode> Parser::parsePreprocessor() {
    /*
    пропуск #
    берем токен
    если не идентификатор, ошибка
    ставим импорт (??? надо переделать под корень)
    возвращаем препроцессор ноду с типом и значением
    */
    next(); // consume '#'
    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected preprocessor directive");
        return nullptr;
    }

    ASTPreprocessorDirectiveType type = ASTPreprocessorDirectiveType::Import; // TODO: Map properly
    return makeMemoryPtr<PreprocessorDirectiveNode>(type, token.value);
}

// ==== Helper functions ====

// Parses either a block or a single statement after an 'if' condition.
MemoryPtr<ASTNode> Parser::parseBlockorStatement() {
    if (match(TokenType::Delimeter, "{")) {
        MemoryPtr<BlockNode> block = parseBlock();
        guardCheck(std::move(block), "[Neoluma/Parser] Expected block after 'if/elif/else' condition");
        return std::move(block);
    }
    else {
        MemoryPtr<ASTNode> block = parseStatement();
        guardCheck(block, "[Neoluma/Parser] Expected statement after 'if/elif/else' condition");
        return std::move(block);
    }
}

// Simplifies guard conditions to one line. Condition is unary reversed inside condition. Pass without !.
void guardCheck(const MemoryPtr<ASTNode>& condition, const std::string& errorMsg) {
    if (!condition) {
        std::println(std::cerr, errorMsg);
        return;
    }
}