#include "../nodes.hpp"
#include "parser.hpp"
#include "../token.hpp"
#include "../../../HelperFunctions.hpp"
#include "ASTBuilder.hpp"
#include <iostream>
#include <unordered_set>
#include <print>
#include <typeinfo>

// ==== Print the parser output ====
void Parser::printModule(int indentation) {
    parseModule();
    if (!moduleSource) {
        std::cerr << "[Neoluma/Parser] No module to print.";
        return;
    }

    std::println("{}", moduleSource->toString(indentation));
}

// ==== Main parsing ====
void Parser::parseModule() {
    auto moduleNode = ASTBuilder::createModule(moduleName);
    auto dn = getDelimeterNames();

    while (!isAtEnd()) {
        if (curToken().type == TokenType::Delimeter && curToken().value == dn[Delimeters::Semicolon]) { next(); continue; }

        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in module {} (L{}:{}, char '{}')", moduleName, curToken().line, curToken().column, curToken().value);
            break;
        }

        moduleNode->body.push_back(std::move(stmt));
    }

    moduleSource = std::move(moduleNode);
}

// ==== Statement parsing ====
MemoryPtr<ASTNode> Parser::parseStatement() {
    Token token = curToken();
    auto km = getKeywordMap();
    auto dm = getDelimeterNames();

    std::vector<MemoryPtr<CallExpressionNode>> decorators = {};

    if (match(TokenType::Preprocessor)) return parsePreprocessor();
    if (match(TokenType::Decorator)) decorators = parseDecoratorCalls();

    auto modifiers = parseModifiers();

    token = curToken();
    // === Control Flow Keywords ===
    if (token.type == TokenType::Keyword) {
        if (km[token.value] == Keywords::If) return parseIf();
        if (km[token.value] == Keywords::Switch) return parseSwitch();
        if (km[token.value] == Keywords::Try) return parseTryCatch();
        if (km[token.value] == Keywords::For) return parseFor();
        if (km[token.value] == Keywords::While) return parseWhile();
        if (km[token.value] == Keywords::Return) {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Neoluma/Parser] Expected expression after 'return'";
                return nullptr;
            }
            return ASTBuilder::createReturnStatement(std::move(expr));
        }
        if (km[token.value] == Keywords::Throw) {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Neoluma/Parser] Expected expression after 'throw'";
                return nullptr;
            }
            return ASTBuilder::createThrowStatement(std::move(expr));
        }
        if (km[token.value] == Keywords::Break) return ASTBuilder::createBreakStatement();
        if (km[token.value] == Keywords::Continue) return ASTBuilder::createContinueStatement();

        // for modifier affected structures
        if (km[token.value] == Keywords::Function) return parseFunction(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Class) return parseClass(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Enum) return parseEnum(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Interface) return parseInterface(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Decorator) return parseDecorator(std::move(decorators), std::move(modifiers));
        if (!modifiers.empty()) { std::println(std::cerr, "[Neoluma/Parser] Unexpected modifier before {} (L{}:{})", token.value, token.line, token.column); }
    }

    // === Block ===
    if (match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        return parseBlock();
    }

    // === Fallback ===
    return parseExpression(); // Default to expression statement
}

// ==== Expression parsing ====
MemoryPtr<ASTNode> Parser::parsePrimary() {
    Token token = curToken();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    // Parenthesis, lambdas, arrays, sets, dicts
    if (token.type == TokenType::Delimeter) {
        // Parenthesis / lambdas
        if (token.value == dn[Delimeters::LeftParen]) {
            next();
            std::vector<MemoryPtr<ASTNode>> exprs;
            while (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                MemoryPtr<ASTNode> expr = parseExpression();
                if (!expr) {
                    std::println(std::cerr, "[Neoluma/Parser] Expected expression after '(' in lambda expression (L{}:{})", token.line, token.column);
                    return nullptr;
                }
                exprs.push_back(std::move(expr));
                if (curToken().type == TokenType::Delimeter && isNextLine()) next();
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser] Expected ')' after expressions in lambda expression (L{}:{})", token.line, token.column);
                return nullptr;
            }
            next();
            // Check for lambda expressions here
            if (match(TokenType::Operator, on[Operators::AssignmentArrow])) {
                next();
                auto block = parseBlock();
                if (!block) {
                    std::println(std::cerr, "[Neoluma/Parser] Could not find a block of code after assignment arrow. (L{}:{})", token.line, token.column);
                    return nullptr;
                }
                return ASTBuilder::createLambda(std::move(exprs), std::move(block));
            }
            return std::move(exprs[0]);
        }
        // Arrays
        if (token.value == dn[Delimeters::LeftBracket]) {
            next();
            std::vector<MemoryPtr<ASTNode>> e;

            while (!match(TokenType::Delimeter, dn[Delimeters::RightBracket])) {
                e.push_back(parseExpression());
                if (curToken().type == TokenType::Delimeter && isNextLine()) next(); // To allow multiline expressions of arrays. I think this would be absolutely neat sugar for everybody.
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
            }
            next();
            return ASTBuilder::createArray(std::move(e)/*, typeHint miss*/);
        }
        // Sets / dicts
        if (token.value == dn[Delimeters::LeftBraces]) {
            next();
            bool isDict = (lookupNext().type == TokenType::Delimeter && lookupNext().value == dn[Delimeters::Colon]);

            if (isDict) {
                std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> e;
                while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                    auto key = parseExpression();
                    next();
                    if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
                        std::println(std::cerr, "[Neoluma/Parser] Could not find colon after key '{}' (L{}:{})", key->value, token.line, token.column);
                        return nullptr;
                    }
                    next();
                    auto val = parseExpression();
                    e.push_back({std::move(key), std::move(val)});
                    if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                    else break;
                }
                return ASTBuilder::createDict(std::move(e)/*, typeHint miss*/);
            }

            std::vector<MemoryPtr<ASTNode>> e;
            while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                e.push_back(parseExpression());
                next(); // todo: check if needed later
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            next();
            return ASTBuilder::createSet(std::move(e)/*, typeHint miss*/);
        }
        if (isNextLine()) {
            next();
            //std::println("skip newline L{}:{}", token.line, token.column);
            return nullptr;
        }
    } 
    // Data type + Booleans
    else if ((token.type == TokenType::Number || token.type == TokenType::String) 
    || (token.type == TokenType::Identifier && (token.value == "true" || token.value == "false"))) {
        next();
        return ASTBuilder::createLiteral(token.value);
    } 
    // Null
    else if (token.type == TokenType::Null) {
        next();
        return ASTBuilder::createLiteral("null");
    }

    // Identifier/variable or function call
    else if (token.type == TokenType::Identifier) {
        Token id = next();
        
        // If function call
        if (match(TokenType::Delimeter, dn[Delimeters::LeftParen])) {
            next();
            std::vector<MemoryPtr<ASTNode>> args;

            while (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                auto arg = parseExpression();
                if (arg) args.push_back(std::move(arg));
                
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser] Expected ')' after arguments (L{}:{})", token.line, token.column);
                return nullptr;
            }

            auto callee = ASTBuilder::createVariable(id.value);
            return ASTBuilder::createCallExpression(std::move(callee), std::move(args));
        }

        // else identifier
        return ASTBuilder::createVariable(id.value);
    }

    std::println(std::cerr, "[Neoluma/Parser] Unexpected token in primary expression: {} (L{}:{})", token.value, token.line, token.column);
    return nullptr;
}

MemoryPtr<ASTNode> Parser::parseExpression() {
    Token token = curToken();
    auto om = getOperatorMap();
    auto dm = getDelimeterMap();

    // Assignments
    if (auto nextToken = lookupNext(); token.type == TokenType::Identifier &&
                                       (om[nextToken.value] == Operators::Assign || dm[nextToken.value] == Delimeters::Colon
                                        || om[nextToken.value] == Operators::Nullable)) {
        return parseAssignment();
    }
    // Unary operations
    if (om.find(token.value) != om.end()) {
        Operators op = om[token.value];
        if (op == Operators::LogicalNot || op == Operators::Subtract) {
            next();
            return parseUnary(token.value);
        }
    }
    // Binary operation fallback
    return parseBinary(0);
}

MemoryPtr<ASTNode> Parser::parseBinary(int prevPredecence) {
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

        left = ASTBuilder::createBinaryOperation(std::move(left), op, std::move(right));
    }
    return left;
}

MemoryPtr<UnaryOperationNode> Parser::parseUnary(const std::string& op) {
    MemoryPtr<ASTNode> operand = parsePrimary();
    if (!operand) {
        std::println(std::cerr, "[Neoluma/Parser] Missing operand after unary operator: {} (L{}:{})", op, curToken().line, curToken().column);
        return nullptr;
    }
    return ASTBuilder::createUnaryOperation(op, std::move(operand));
}

MemoryPtr<AssignmentNode> Parser::parseAssignment(){
    auto om = getOperatorMap();
    auto on = getOperatorNames();
    auto dn = getDelimeterNames();

    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected identifier for assignment (L{}:{})", token.line, token.column);
        return nullptr;
    }
    std::string variableName = token.value;
    next();

    bool isNullable = false;
    if (match(TokenType::Operator, on[Operators::Nullable])) { isNullable = true; next(); }

    std::string rawType = "None";
    if (match(TokenType::Delimeter, dn[Delimeters::Colon])) {
        next();
        if (curToken().type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser] Expected type after ':' in variable declaration (L{}:{})", token.line, token.column);
            return nullptr;
        }

        rawType = curToken().value;
        next();
    }

    token = curToken();
    if ((!match(TokenType::Operator)) || (om[token.value] == Operators::AddAssign || om[token.value] == Operators::SubAssign || om[token.value] == Operators::MulAssign || om[token.value] == Operators::DivAssign || om[token.value] == Operators::ModAssign || om[token.value] == Operators::PowerAssign )) {
        std::println(std::cerr, "[Neoluma/Parser] Expected assignment operator (=, +=, -=, and etc.) (L{}:{})", token.line, token.column);
        return nullptr;
    }

    std::string op = token.value;
    next();

    MemoryPtr<ASTNode> value = parseExpression();
    if (!value) {
        std::println(std::cerr, "[Neoluma/Parser] Failed to parse expression after assignment (L{}:{})", token.line, token.column);
        return nullptr;
    }

    return ASTBuilder::createAssignment(ASTBuilder::createVariable(variableName, rawType, isNullable), op, std::move(value));
}

MemoryPtr<ReturnStatementNode> Parser::parseReturn(){
    next();

    if (curToken().type == TokenType::Delimeter && isNextLine()) {
        return ASTBuilder::createReturnStatement(nullptr);
    }

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        return nullptr;
    }

    if (curToken().type != TokenType::Delimeter || !isNextLine()) {
        return nullptr;
    }
    next();

    return ASTBuilder::createReturnStatement(std::move(expr));
}

// ==== Control flow ====
MemoryPtr<IfNode> Parser::parseIf() {
    next();

    auto dm = getDelimeterNames();
    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        return nullptr;
    }

    MemoryPtr<ASTNode> ifBlock = parseBlockorStatement();
    MemoryPtr<ASTNode> elseBlock = nullptr;

    auto km = getKeywordNames();
    if (match(TokenType::Keyword, km[Keywords::Else])) {
        next();
        elseBlock = parseBlockorStatement();
    }

    return ASTBuilder::createIf(std::move(condition), std::move(ifBlock), std::move(elseBlock));
}

MemoryPtr<SwitchNode> Parser::parseSwitch() {
    next();
    auto dm = getDelimeterNames();
    auto km = getKeywordNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        return nullptr;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        return nullptr;
    }
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase = nullptr;

    while (!match(TokenType::Delimeter, dm[Delimeters::RightBracket])) {
        Token tok = curToken();

        if (match(TokenType::Keyword, km[Keywords::Case])) {
            next();
            MemoryPtr<ASTNode> condition = parseExpression();
            if (!match(TokenType::Delimeter, dm[Delimeters::Colon])) {
                return nullptr;
            }
            next();
            auto body = parseBlock();
            cases.push_back(ASTBuilder::createCase(std::move(condition), std::move(body)));
        }
        else if (match(TokenType::Keyword, km[Keywords::Default])) {
            next();
            if (!match(TokenType::Delimeter, dm[Delimeters::Colon])) {
                return nullptr;
            }
            next();
            auto body = parseBlock();
            defaultCase = ASTBuilder::createDefaultCase(std::move(body));
        }
        else {
            return nullptr;
        }
    }

    return ASTBuilder::createSwitch(std::move(expr), std::move(cases), std::move(defaultCase));
}

MemoryPtr<TryCatchNode> Parser::parseTryCatch() {
    next();
    auto km = getKeywordNames();

    auto tryBlock = parseBlock();
    if (!tryBlock) {
        return nullptr;
    }

    if (!match(TokenType::Keyword, km[Keywords::Catch])) {
        return nullptr;
    }
    next();

    auto catchBlock = parseBlock();
    if (!catchBlock) {
        return nullptr;
    }

    return ASTBuilder::createTryCatch(std::move(tryBlock), std::move(catchBlock));
}

MemoryPtr<ForLoopNode> Parser::parseFor() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        return nullptr;
    }
    next();

    if (curToken().type != TokenType::Identifier) {
        return nullptr;
    }

    std::string varName = curToken().value;
    auto varNode = ASTBuilder::createVariable(varName);
    next();

    if (!match(TokenType::Operator, dm[Delimeters::Colon])) {
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        return nullptr;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        return nullptr;
    }

    auto body = parseBlock();
    if (!body) {
        return nullptr;
    }

    return ASTBuilder::createForLoop(std::move(varNode), std::move(iterable), std::move(body));
}

MemoryPtr<WhileLoopNode> Parser::parseWhile() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        return nullptr;
    }

    return ASTBuilder::createWhileLoop(std::move(condition), std::move(body));
}

// ==== Declarations ====
MemoryPtr<FunctionNode> Parser::parseFunction(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dm = getDelimeterNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser] Expected function name (L{}:{})", nameToken.line, nameToken.column);
        return nullptr;
    }
    std::string funcName = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after function name (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        Token paramName = curToken();
        if (paramName.type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser] Expected parameter name (L{}:{})", paramName.line, paramName.column);
            return nullptr;
        }
        next();

        auto type = parsePrimary(); // TODO: detect type PROPERLY later I need this to compile at least
        params.push_back(makeMemoryPtr<ParameterNode>(paramName.value, type->value)); // TODO: Add default value parsing

        if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
        else break;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after function parameters (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }
    next();

    MemoryPtr<BlockNode> body = parseBlock();
    if (!body) return nullptr;

    return ASTBuilder::createFunction(funcName, std::move(params), std::move(body), std::move(decorators), std::move(modifiers));
}

MemoryPtr<ClassNode> Parser::parseClass(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dm = getDelimeterNames();
    auto km = getKeywordNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser] Expected class name (L{}:{})", nameToken.line, nameToken.column);
        return nullptr;
    }
    std::string className = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{{' to start class body (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<VariableNode>> fields;
    std::vector<MemoryPtr<FunctionNode>> methods;

    while (!match(TokenType::Delimeter, dm[Delimeters::RightBraces])) {
        Token t = curToken();

        std::vector<MemoryPtr<CallExpressionNode>> funcdec = {};
        if (t.value == "@") funcdec = parseDecoratorCalls();
        auto modifs = parseModifiers();

        if (match(TokenType::Keyword, km[Keywords::Function])) {
            auto method = parseFunction(std::move(funcdec), std::move(modifs));
            if (method) methods.push_back(std::move(method));
        }
        else if (t.type == TokenType::Identifier) {
            std::string fieldName = t.value;
            next();
            //ASTVariableType varType = ASTVariableType::Undefined;
            fields.push_back(ASTBuilder::createVariable(fieldName));
            if (curToken().type == TokenType::Delimeter && isNextLine()) next();
        }
        else {
            std::println(std::cerr, "[Neoluma/Parser] Unexpected token in class body (L{}:{})", curToken().line, curToken().column);
            break;
        }
    }

    next();
    return ASTBuilder::createClass(className, std::move(fields), std::move(methods), std::move(decorators), std::move(modifiers));
}

MemoryPtr<BlockNode> Parser::parseBlock() {
    auto dn = getDelimeterNames();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{{' to start block (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }
    next();
    if (match(TokenType::Delimeter, dn[Delimeters::Semicolon])) next();

    std::vector<MemoryPtr<ASTNode>> block = {};

    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in block (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        }
        block.push_back(std::move(stmt));
        if (curToken().type == TokenType::Delimeter && isNextLine()) next();
    }
    if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '}}' to end block (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }

    next();
    return ASTBuilder::createBlock(std::move(block));
}

MemoryPtr<ASTNode> Parser::parseDecorator(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers, bool isCall) {
    auto dm = getDelimeterNames();
    Token nameToken = curToken();
    if (!match(TokenType::Decorator)) {
        std::println(std::cerr, "[Neoluma/Parser] Expected decorator name (L{}:{})", nameToken.line, nameToken.column);
        return nullptr;
    }
    std::string name = nameToken.value;
    next();

    if (!isCall) {
        std::vector<MemoryPtr<ParameterNode>> params;
        if (match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
            next();
            while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
                Token tok = curToken();
                if (!match(TokenType::Identifier)) break;
                params.push_back(ASTBuilder::createParameter(tok.value));
                next();
                if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
                else break;
            }
            if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser] Unterminated decorator params (L{}:{})", curToken().line, curToken().column);
                return nullptr;
            }
            next();
        }
        auto block = parseBlock();
        return ASTBuilder::createDecorator(name, std::move(params), std::move(block), std::move(decorators), std::move(modifiers));
    }

    std::vector<MemoryPtr<ASTNode>> args;
    if (match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        next();
        while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
            auto arg = parseExpression();
            if (arg) args.push_back(std::move(arg));

            if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
            else break;
        }
        if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
            std::println(std::cerr, "[Neoluma/Parser] Unterminated decorator params (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        }
        next();
    }
    return ASTBuilder::createCallExpression(ASTBuilder::createVariable(name), std::move(args));
}

std::vector<MemoryPtr<CallExpressionNode>> Parser::parseDecoratorCalls() {
    std::vector<MemoryPtr<CallExpressionNode>> calls;
    while (match(TokenType::Decorator)) {
        auto node = parseDecorator({}, {}, true);
        if (!node) {
            std::println(std::cerr, "[Neoluma/Parser] Failed to parse decorator call (L{}:{})", curToken().line, curToken().column);
            break;
        }
        auto call = as<CallExpressionNode>(std::move(node));
        if (!call) {
            std::println(std::cerr, "[Neoluma/Parser] Decorator parsed to a non-call node (L{}:{})", curToken().line, curToken().column);
            break;
        }
        calls.push_back(std::move(call));
        next();
    }

    return calls;
}

std::vector<MemoryPtr<ModifierNode>> Parser::parseModifiers() {
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    auto kn = getKeywordNames();
    
    while (curToken().type == TokenType::Keyword) {
        if (match(TokenType::Keyword, kn[Keywords::Static])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Static));
        else if (match(TokenType::Keyword, kn[Keywords::Const])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Const));
        else if (match(TokenType::Keyword, kn[Keywords::Public])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Public));
        else if (match(TokenType::Keyword, kn[Keywords::Protected])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Protected));
        else if (match(TokenType::Keyword, kn[Keywords::Private])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Private));
        else if (match(TokenType::Keyword, kn[Keywords::Override])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Override));
        else if (match(TokenType::Keyword, kn[Keywords::Async])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Async));
        else if (match(TokenType::Keyword, kn[Keywords::Debug])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Debug));
        else break;
        next();
    }

    return modifiers;
}

MemoryPtr<ASTNode> Parser::parsePreprocessor() {
    auto pn = getPreprocessorNames();

    if (match(TokenType::Preprocessor, pn[Preprocessors::Import])) {
        next();
        if (!match(TokenType::String)) {
            std::println(std::cerr, "[Neoluma/Parser] Couldn't find what to import after #import (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        }
        auto moduleName = curToken().value;
        ASTImportType importType = ASTImportType::Native;
        if ((moduleName.contains("/") || moduleName.contains(".")) && moduleName.contains(":")) importType = ASTImportType::ForeignRelative;
        else if (moduleName.contains("/") || moduleName.contains(".")) importType = ASTImportType::Relative;
        else if (moduleName.contains(":")) importType = ASTImportType::Foreign;

        std::string alias = "";
        next();
        if (match(TokenType::Preprocessor, pn[Preprocessors::As])) {
            next();
            alias = curToken().value;
        }
        return makeMemoryPtr<ImportNode>(moduleName, alias, importType);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Macro])) {
        next();
        if (!match(TokenType::Identifier)) {
            std::println(std::cerr, "[Neoluma/Parser] Couldn't find identifier after #macro{} (L{}:{})", "", curToken().line, curToken().column);
            return nullptr;
        }
        next();
        std::string value = "";
        while (!isNextLine()) {
            value = value + curToken().value;
            next();
        }
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Macro, value);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Baremetal])) {
        next();
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Baremetal);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Unsafe])) {
        next();
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Unsafe);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Float])) {
        next();
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Float);
    }

    ASTPreprocessorDirectiveType type = ASTPreprocessorDirectiveType::None; // Fallback to none, if this somehow got out of hand.
    return makeMemoryPtr<PreprocessorDirectiveNode>(type);
}

MemoryPtr<EnumNode> Parser::parseEnum(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser] Enum does not have a name (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }
    next();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser] Missing '{{' after enum name (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    } 
    next();

    std::vector<MemoryPtr<EnumMemberNode>> elements;
    
    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        if (curToken().type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser] A non-identifier found in enum (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        } 
        auto name = curToken().value;
        MemoryPtr<LiteralNode> value = nullptr;
    
        next();
        if (match(TokenType::Operator, on[Operators::Assign])) {
            auto tmp = parsePrimary();
            if (typeid(tmp) != typeid(MemoryPtr<LiteralNode>)) {
                std::println(std::cerr, "[Neoluma/Parser] Non-literal value detected in enum after identifier '{}' (L{}:{})", name, curToken().line, curToken().column);
                return nullptr;
            }
            value = as<LiteralNode>(std::move(tmp));
        }
        elements.push_back(ASTBuilder::createEnumMember(name, std::move(value)));
        next();

        if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
        else if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
            std::println(std::cerr, "[Neoluma/Parser] Delimeter at enum found that is not either comma or right braces. (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        }
        else break;
    }

    return ASTBuilder::createEnum(std::move(elements), std::move(decorators), std::move(modifiers));
}

MemoryPtr<InterfaceNode> Parser::parseInterface(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser] Interface does not have a name (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    }
    next();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser] Missing '{{' after interface name (L{}:{})", curToken().line, curToken().column);
        return nullptr;
    } 
    next();

    std::vector<MemoryPtr<InterfaceFieldNode>> elements;
    
    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        if (curToken().type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser] A non-identifier found in interface (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        } 
        auto name = curToken().value;
        bool isNullable = false;
        next();
        if (match(TokenType::Operator, on[Operators::Nullable])) isNullable = true;

        if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
           std::println(std::cerr, "[Neoluma/Parser] No colon after identifier in an interface (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        }

        MemoryPtr<LiteralNode> vartype = nullptr;
        // this is a very bad setup imo. it must be fixed later
        next();
        if (match(TokenType::Identifier)) {
            auto tmp = parsePrimary();
            if (typeid(tmp) != typeid(MemoryPtr<LiteralNode>)) {
                std::println(std::cerr, "[Neoluma/Parser] Non-variable value detected in interface after identifier '{}' (L{}:{})", name, curToken().line, curToken().column);
                return nullptr;
            }
            vartype = as<LiteralNode>(std::move(tmp));
        }

        elements.push_back(ASTBuilder::createInterfaceField(name, vartype->value, isNullable));
        next();
        if (isNextLine()) next();
        else if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
           std::println(std::cerr, "[Neoluma/Parser] Delimeter at enum found that is not either '\\n', ';', or right braces. (L{}:{})", curToken().line, curToken().column);
            return nullptr;
        }
         else break;
    }

    return ASTBuilder::createInterface(std::move(elements), std::move(decorators), std::move(modifiers));
}

// ==== Helper functions ====

// Parses either a block or a single statement after an 'if' condition.
MemoryPtr<ASTNode> Parser::parseBlockorStatement() {
    auto dm = getDelimeterNames();

    if (match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        MemoryPtr<BlockNode> block = parseBlock();
        if(!block) {
            //std::println("[Neoluma/Parser] Expected block after 'if/elif/else' condition{}", "");
            return nullptr;
        }
        return std::move(block);
    } else {
        MemoryPtr<ASTNode> block = parseStatement();
        if(!block) {
            //std::println("[Neoluma/Parser] Expected statement after 'if/elif/else' condition{}", "");
            return nullptr;
        }
        return std::move(block);
    }
}

// Detects nextline expression
bool Parser::isNextLine(){
    auto dn = getDelimeterNames();
    if (match(TokenType::Delimeter, dn[Delimeters::Semicolon])) return true;
    return false;
}

// to make math order
int getOperatorPrecedence(const std::string& op){
    auto on = getOperatorNames();
    if (op == on[Operators::Power]) return 7;
    else if (op == on[Operators::Multiply] || op == on[Operators::Divide] || op == on[Operators::Modulo]) return 6;
    else if (op == on[Operators::Add] || op == on[Operators::Subtract]) return 5;
    else if (op == on[Operators::BitwiseLeftShift] || op == on[Operators::BitwiseRightShift]) return 4;
    else if (op == on[Operators::BitwiseAnd]) return 3;
    else if (op == on[Operators::BitwiseXOr]) return 2;
    else if (op == on[Operators::BitwiseOr]) return 1;
    else if (op == on[Operators::Equal] || op == on[Operators::NotEqual] || op == on[Operators::LessThan] || op == on[Operators::GreaterThan] || op == on[Operators::LessThanOrEqual] || op == on[Operators::GreaterThanOrEqual]) return 0;
    else if (op == on[Operators::LogicalAnd]) return -1;
    else if (op == on[Operators::LogicalOr]) return -2;
    return -3;
}