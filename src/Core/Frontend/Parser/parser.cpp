#include "../nodes.hpp"
#include "parser.hpp"
#include "../token.hpp"
#include "../../../HelperFunctions.hpp"
#include <iostream>
#include <unordered_set>
#include <print>
#include <typeinfo>

// ==== Main parsing ====
MemoryPtr<ModuleNode> Parser::parseModule() {
    auto moduleNode = makeMemoryPtr<ModuleNode>(moduleName);

    while (!isAtEnd()) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            //std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in module {}", moduleName);
            break;
        }

        moduleNode->body.push_back(std::move(stmt));
    }

    return moduleNode;
}

// ==== Statement parsing ====
MemoryPtr<ASTNode> Parser::parseStatement() {
    Token token = curToken();
    auto km = getKeywordMap();
    auto dm = getDelimeterNames();

    std::vector<MemoryPtr<CallExpressionNode>> decorators = {};
    
    if (token.value == "#") return parsePreprocessor();
    if (token.value == "@") decorators = parseDecoratorCalls();
    
    auto modifiers = parseModifiers();

    // === Control Flow Keywords ===
    if (token.type == TokenType::Keyword) {
        if (km[token.value] == Keywords::If) return parseIf();
        else if (km[token.value] == Keywords::Switch) return parseSwitch();
        else if (km[token.value] == Keywords::Try) return parseTryCatch();
        else if (km[token.value] == Keywords::For) return parseFor();
        else if (km[token.value] == Keywords::While) return parseWhile();
        else if (km[token.value] == Keywords::Return) {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Neoluma/Parser] Expected expression after 'return'";
                return nullptr;
            }
            return makeMemoryPtr<ReturnStatementNode>(expr);
        }
        else if (km[token.value] == Keywords::Throw) {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Neoluma/Parser] Expected expression after 'throw'";
                return nullptr;
            }
            return makeMemoryPtr<ThrowStatementNode>(expr);
        }
        else if (km[token.value] == Keywords::Break) return makeMemoryPtr<BreakStatementNode>();
        else if (km[token.value] == Keywords::Continue) return makeMemoryPtr<ContinueStatementNode>();

        // for modifier affected structures
        if (km[token.value] == Keywords::Function) return parseFunction(std::move(decorators), std::move(modifiers));
        else if (km[token.value] == Keywords::Class) return parseClass(std::move(decorators), std::move(modifiers));
        else if (km[token.value] == Keywords::Enum) return parseEnum(std::move(decorators), std::move(modifiers));
        else if (km[token.value] == Keywords::Interface) return parseInterface(std::move(decorators), std::move(modifiers));
        else if (km[token.value] == Keywords::Decorator) return parseDecorator(std::move(decorators), std::move(modifiers));
        else if (!modifiers.empty()) std::cout <<'e'; /*std::println(std::cerr, "[Neoluma/Parser] Unexpected modifier before {}", token.value)*/
    }

    // === Block ===
    if (match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        return parseBlock();
    }

    // === Fallback ===
    return parseExpression(); // Default to expression statement
}

// ==== Expression parsing ====
MemoryPtr<ASTNode> Parser::parsePrimary(){
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
                    //std::println(std::cerr, "[Neoluma/Parser] Expected expression after '(' in lambda expression", ")");
                    return nullptr;
                }
                exprs.push_back(std::move(expr));
                if (curToken().type == TokenType::Delimeter && isNextLine()) next();
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            next();
            // Check for lambda expressions here
            if (match(TokenType::Operator, on[Operators::AssignmentArrow])) {
                next();
                auto block = parseBlock();
                if (!block) {
                    //std::println(std::cerr, "[Neoluma/Parser] Could not find a block of code after assignment arrow. {}", "");
                    return nullptr;
                }
                return makeMemoryPtr<LambdaNode>(std::move(exprs), block);
            }
            return std::move(exprs[0]);
        }
        // Arrays
        else if (token.value == dn[Delimeters::LeftBracket]) {
            next();
            std::vector<MemoryPtr<ASTNode>> e;

            while (!match(TokenType::Delimeter, dn[Delimeters::RightBracket])) {
                e.push_back(parseExpression());
                if (curToken().type == TokenType::Delimeter && isNextLine()) next(); // To allow multiline expressions of arrays. I think this would be absolutely neat sugar for everybody.
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
            }
            next();
            return makeMemoryPtr<ArrayNode>(e/*, typeHint miss*/);
        }
        // Sets / dicts
        else if (token.value == dn[Delimeters::LeftBraces]) {
            next();
            bool isDict = (lookupNext().type == TokenType::Delimeter && lookupNext().value == dn[Delimeters::Colon]);

            if (isDict) {
                std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> e;
                while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                    auto key = parseExpression();
                    next();
                    if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
                        //std::println(std::cerr, "[Neoluma/Parser] Could not find colon after key '{}'", key->value);
                        return nullptr;
                    }
                    next();
                    auto val = parseExpression();
                    e.push_back({std::move(key), std::move(val)});
                    if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                    else break;
                }
                return makeMemoryPtr<DictNode>(e/*, typeHint miss*/);
            } else {
                std::vector<MemoryPtr<ASTNode>> e;
                while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                    e.push_back(parseExpression());
                    next(); // todo: check if needed later
                    if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                    else break;
                }
                next();
                return makeMemoryPtr<SetNode>(e/*, typeHint miss*/);
            }
        }
    } 
    // Data type + Booleans
    else if ((token.type == TokenType::Number || token.type == TokenType::String) 
    || (token.type == TokenType::Identifier && (token.value == "true" || token.value == "false"))) {
        next();
        return makeMemoryPtr<LiteralNode>(ASTVariableType::Undefined, token.value);
    } 
    // Null
    else if (token.type == TokenType::Null) {
        next();
        return makeMemoryPtr<LiteralNode>(ASTVariableType::Undefined, "null");
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
                if (arg) args.push_back(makeMemoryPtr<ASTNode>(std::move(arg)));
                
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                //std::println(std::cerr, "[Neoluma/Parser] Expected ')' after arguments{}", "");
                return nullptr;
            }

            auto callee = makeMemoryPtr<VariableNode>(id.value);
            return makeMemoryPtr<CallExpressionNode>(std::move(callee), args);
        }

        // else identifier
        return makeMemoryPtr<VariableNode>(id.value);
    }

    std::println(std::cerr, "[Neoluma/Parser] Unexpected token in primary expression: {}", token.value);
    return nullptr;
}

MemoryPtr<ASTNode> Parser::parseExpression() {
    Token token = curToken();
    auto om = getOperatorMap();

    // Assignments
    if (token.type == TokenType::Identifier && 
        lookupNext().type == TokenType::Operator && 
        om[lookupNext().value] == Operators::Assign) return parseAssignment();
    // Unary operations
    else if (om.find(token.value) != om.end()) {
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

        MemoryPtr<BinaryOperationNode> binaryOpNode = makeMemoryPtr<BinaryOperationNode>(std::move(left), op, std::move(right));
        left = std::move(binaryOpNode);
    }
    return left;
}

MemoryPtr<UnaryOperationNode> Parser::parseUnary(const std::string& op) {
    MemoryPtr<ASTNode> operand = parsePrimary();
    if (!operand) {
        std::println(std::cerr, "[Neoluma/Parser] Missing operand after unary operator: {}", op);
        return nullptr;
    }
    return makeMemoryPtr<UnaryOperationNode>(op, std::move(operand));
}

MemoryPtr<AssignmentNode> Parser::parseAssignment(){
    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected identifier for assignment{}", "");
        return nullptr;
    }
    std::string variableName = token.value;
    next();

    auto om = getOperatorMap();

    token = curToken();
    if ((!match(TokenType::Operator)) || (om[token.value] == Operators::AddAssign || om[token.value] == Operators::SubAssign || om[token.value] == Operators::MulAssign || om[token.value] == Operators::DivAssign || om[token.value] == Operators::ModAssign || om[token.value] == Operators::PowerAssign )) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected assignment operator (=, +=, -=, and etc.){}", "");
        return nullptr;
    }

    std::string op = token.value;
    next();

    MemoryPtr<ASTNode> value = parseExpression();
    if (!value) {
        //std::println(std::cerr, "[Neoluma/Parser] Failed to parse expression after assignment{}", "");
        return nullptr;
    }

    return makeMemoryPtr<AssignmentNode>(variableName, op, std::move(value));
}

MemoryPtr<ReturnStatementNode> Parser::parseReturn(){
    next();

    if (curToken().type == TokenType::Delimeter && isNextLine()) {
        return makeMemoryPtr<ReturnStatementNode>(nullptr);
    }

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        //std::println(std::cerr, "[Neoluma/Parser] Invalid expression in return statement{}", "");
        return nullptr;
    }

    if (curToken().type != TokenType::Delimeter || !isNextLine()) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ';' after return expression{}", "");
        return nullptr;
    }
    next();

    return makeMemoryPtr<ReturnStatementNode>(std::move(expr));
}

// ==== Control flow ====
MemoryPtr<IfNode> Parser::parseIf() {
    next();

    auto dm = getDelimeterNames();
    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'if'{}", "");
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected condition expression after 'if'{}", "");
        return nullptr;
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ')' after condition in 'if'{}", "");
        return nullptr;
    }

    MemoryPtr<ASTNode> ifBlock = parseBlockorStatement();
    MemoryPtr<ASTNode> elseBlock = nullptr;

    auto km = getKeywordNames();
    if (match(TokenType::Keyword, km[Keywords::Else])) {
        next();
        elseBlock = parseBlockorStatement();
    }

    return makeMemoryPtr<IfNode>(condition, ifBlock, elseBlock);
}

MemoryPtr<SwitchNode> Parser::parseSwitch() {
    next();
    auto dm = getDelimeterNames();
    auto km = getKeywordNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'switch'{}", "");
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        //std::println(std::cerr, "[Neoluma/Parser] Invalid expression in switch{}", "");
        return nullptr;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ')' after switch expression{}", "");
        return nullptr;
    }
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '{' after switch{}", "");
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
                //std::println(std::cerr, "[Neoluma/Parser] Expected ':' after case condition{}", "");
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            cases.push_back(makeMemoryPtr<CaseNode>(condition, body));
        }
        else if (match(TokenType::Keyword, km[Keywords::Default])) {
            next();
            if (!match(TokenType::Delimeter, dm[Delimeters::Colon])) {
                //std::println(std::cerr, "[Neoluma/Parser] Expected ':' after default{}", "");
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            defaultCase = makeMemoryPtr<SCDefaultNode>(body);
        }
        else {
            //std::println(std::cerr, "[Neoluma/Parser] Unexpected token in switch{}", "");
            return nullptr;
        }
    }

    return makeMemoryPtr<SwitchNode>(expr, cases, defaultCase);
}

MemoryPtr<TryCatchNode> Parser::parseTryCatch() {
    next();
    auto km = getKeywordNames();

    auto tryBlock = as<BlockNode>(parseBlockorStatement());
    if (!tryBlock) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected try block{}", "");
        return nullptr;
    }

    if (!match(TokenType::Keyword, km[Keywords::Catch])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected 'catch' after 'try'{}", "");
        return nullptr;
    }
    next();

    auto catchBlock = as<BlockNode>(parseBlockorStatement());
    if (!catchBlock) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected catch block{}", "");
        return nullptr;
    }

    return makeMemoryPtr<TryCatchNode>(tryBlock, catchBlock);
}

MemoryPtr<ForLoopNode> Parser::parseFor() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'for'{}", "");
        return nullptr;
    }
    next();

    if (curToken().type != TokenType::Identifier) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected identifier in for loop{}", "");
        return nullptr;
    }

    std::string varName = curToken().value;
    auto varNode = makeMemoryPtr<VariableNode>(varName, ASTVariableType::Void);
    next();

    if (!match(TokenType::Operator, dm[Delimeters::Colon])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ':' after loop variable in for{}", "");
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        //std::println(std::cerr, "[Neoluma/Parser] Invalid iterable in for loop{}", "");
        return nullptr;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ')' after for condition{}", "");
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected block or statement after for{}", "");
        return nullptr;
    }

    return makeMemoryPtr<ForLoopNode>(std::move(varNode), std::move(iterable), std::move(body));
}

MemoryPtr<WhileLoopNode> Parser::parseWhile() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'while'{}", "");
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid condition in while loop{}", "");
        return nullptr;
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ')' after while condition{}", "");
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected block or statement after while{}", "");
        return nullptr;
    }

    return makeMemoryPtr<WhileLoopNode>(std::move(condition), std::move(body));
}

// ==== Declarations ====
MemoryPtr<FunctionNode> Parser::parseFunction(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dm = getDelimeterNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected function name{}", "");
        return nullptr;
    }
    std::string funcName = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '(' after function name{}", "");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        Token paramName = curToken();
        if (paramName.type != TokenType::Identifier) {
            //std::println(std::cerr, "[Neoluma/Parser] Expected parameter name{}", "");
            return nullptr;
        }
        next();

        ASTVariableType type = ASTVariableType::Undefined; // TODO: detect type later
        params.push_back(makeMemoryPtr<ParameterNode>(paramName.value, type));

        if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
        else break;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected ')' after function parameters{}", "");
        return nullptr;
    }
    next();

    MemoryPtr<BlockNode> body = parseBlock();
    if (!body) return nullptr;

    return makeMemoryPtr<FunctionNode>(funcName, params, body, std::move(decorators), std::move(modifiers));
}

MemoryPtr<ClassNode> Parser::parseClass(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dm = getDelimeterNames();
    auto km = getKeywordNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected class name{}", "");
        return nullptr;
    }
    std::string className = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '{' to start class body{}", "");
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
            fields.push_back(makeMemoryPtr<VariableNode>(fieldName));
            if (curToken().type == TokenType::Delimeter && isNextLine()) next();
        }
        else {
            //std::println(std::cerr, "[Neoluma/Parser] Unexpected token in class body{}", "");
            break;
        }
    }

    next();
    return makeMemoryPtr<ClassNode>(className, methods, fields, methods, modifiers);
}

MemoryPtr<BlockNode> Parser::parseBlock() {
    auto dn = getDelimeterNames();

    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected '{' to start block{}", "");
        return nullptr;
    }
    next();

    auto block = makeMemoryPtr<BlockNode>();

    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            //std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in block{}", "");
            return nullptr;
        }
        block->statements.push_back(std::move(stmt));
        if (curToken().type == TokenType::Delimeter && isNextLine()) next();
    }

    next();
    return block;
}

MemoryPtr<ASTNode> Parser::parseDecorator(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers, bool isCall) {
    next();
    auto dm = getDelimeterNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected decorator name{}", "");
        return nullptr;
    }
    std::string name = nameToken.value;
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    if (match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        next();
        while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
            Token tok = curToken();
            if (!match(TokenType::Identifier)) break;
            params.push_back(makeMemoryPtr<ParameterNode>(tok.value, ASTVariableType::Undefined));
            next();
            if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
            else break;
        }
        if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
            //std::println(std::cerr, "[Neoluma/Parser] Unterminated decorator params{}", "");
            return nullptr;
        }
        next();
    }

    if (isCall) return makeMemoryPtr<CallExpressionNode>(makeMemoryPtr<VariableNode>(name), params);

    auto block = parseBlock();

    return makeMemoryPtr<DecoratorNode>(name, params, block, modifiers);
}

std::vector<MemoryPtr<CallExpressionNode>> Parser::parseDecoratorCalls() {
    std::vector<MemoryPtr<CallExpressionNode>> calls;
    while (!match(TokenType::Keyword)) {
        auto p = as<CallExpressionNode>(parseDecorator({}, {}, true));
        calls.push_back(std::move(p));
        next();
    }
    return calls;
}

std::vector<MemoryPtr<ModifierNode>> Parser::parseModifiers() {
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    auto kn = getKeywordNames();
    
    while (curToken().type == TokenType::Keyword) {
        if (match(TokenType::Keyword, kn[Keywords::Static])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Static));
        else if (match(TokenType::Keyword, kn[Keywords::Const])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Const));
        else if (match(TokenType::Keyword, kn[Keywords::Public])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Public));
        else if (match(TokenType::Keyword, kn[Keywords::Protected])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Protected));
        else if (match(TokenType::Keyword, kn[Keywords::Private])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Private));
        else if (match(TokenType::Keyword, kn[Keywords::Override])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Override));
        else if (match(TokenType::Keyword, kn[Keywords::Async])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Async));
        else if (match(TokenType::Keyword, kn[Keywords::Debug])) 
            modifiers.push_back(makeMemoryPtr<ModifierNode>(ASTModifierType::Debug));
        else break;
        next();
    }

    return modifiers;
}

MemoryPtr<ASTNode> Parser::parsePreprocessor() {
    next();
    auto pn = getPreprocessorNames();

    if (curToken().type != TokenType::Preprocessor) {
        //std::println(std::cerr, "[Neoluma/Parser] Expected preprocessor directive{}", "");
        return nullptr;
    }

    if (match(TokenType::Preprocessor, pn[Preprocessors::Import])) {
        next();
        if (!match(TokenType::String)) {
            //std::println(std::cerr, "[Neoluma/Parser] Couldn't find what to import after #import{}", "");
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
            //std::println(std::cerr, "[Neoluma/Parser] Couldn't find identifier after #macro{}", "");
            return nullptr;
        }
        next();
        std::string value = "";
        while (!isNextLine()) {
            value = value + curToken().value;
            next();
        }
        return makeMemoryPtr<PreprocessorDirectiveNode>(ASTPreprocessorDirectiveType::Macro, value);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Baremetal])) {
        next();
        return makeMemoryPtr<PreprocessorDirectiveNode>(ASTPreprocessorDirectiveType::Baremetal);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Unsafe])) {
        next();
        return makeMemoryPtr<PreprocessorDirectiveNode>(ASTPreprocessorDirectiveType::Unsafe);
    }
    else if (match(TokenType::Preprocessor, pn[Preprocessors::Float])) {
        next();
        return makeMemoryPtr<PreprocessorDirectiveNode>(ASTPreprocessorDirectiveType::Float);
    }

    ASTPreprocessorDirectiveType type = ASTPreprocessorDirectiveType::None; // Fallback to none, if this somehow got out of hand.
    return makeMemoryPtr<PreprocessorDirectiveNode>(type);
}

MemoryPtr<EnumNode> Parser::parseEnum(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    if (!match(TokenType::Identifier)) {
        //std::println(std::cerr, "[Neoluma/Parser] Enum does not have a name{}", "");
        return nullptr;
    }
    next();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        //std::println(std::cerr, "[Neoluma/Parser] Missing '{' after enum name{}", "");
        return nullptr;
    } 
    next();

    std::vector<MemoryPtr<EnumMemberNode>> elements;
    
    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        if (curToken().type != TokenType::Identifier) {
            //std::println(std::cerr, "[Neoluma/Parser] A non-identifier found in enum{}", "");
            return nullptr;
        } 
        auto name = curToken().value;
        MemoryPtr<LiteralNode> value = nullptr;
    
        next();
        if (match(TokenType::Operator, on[Operators::Assign])) {
            auto tmp = parsePrimary();
            if (typeid(tmp) != typeid(MemoryPtr<LiteralNode>)) {
                std::println(std::cerr, "[Neoluma/Parser] Non-literal value detected in enum after identifier '{}'", name);
                return nullptr;
            }
            value = as<LiteralNode>(std::move(tmp));
        }
        elements.push_back(makeMemoryPtr<EnumMemberNode>(name, value));
        next();

        if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
        else if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
            //std::println(std::cerr, "[Neoluma/Parser] Delimeter at enum found that is not either comma or right braces.{}", "");
            return nullptr;
        }
        else break;
    }

    return makeMemoryPtr<EnumNode>(elements, std::move(decorators), std::move(modifiers));
}

MemoryPtr<InterfaceNode> Parser::parseInterface(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    if (!match(TokenType::Identifier)) {
        //std::println(std::cerr, "[Neoluma/Parser] Interface does not have a name{}", "");
        return nullptr;
    }
    next();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        //std::println(std::cerr, "[Neoluma/Parser] Missing '{' after interface name{}", "");
        return nullptr;
    } 
    next();

    std::vector<MemoryPtr<InterfaceFieldNode>> elements;
    
    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        if (curToken().type != TokenType::Identifier) {
            //std::println(std::cerr, "[Neoluma/Parser] A non-identifier found in interface{}", "");
            return nullptr;
        } 
        auto name = curToken().value;
        bool isNullable = false;
        next();
        if (match(TokenType::Operator, on[Operators::Nullable])) isNullable = true;

        if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
           // std::println(std::cerr, "[Neoluma/Parser] No colon after identifier in an interface{}", "");
            return nullptr;
        }

        MemoryPtr<VariableNode> vartype = nullptr;

        // this is a very bad setup imo but it must be fixed later
        if (match(TokenType::Identifier)) {
            auto tmp = parsePrimary();
            if (typeid(tmp) != typeid(MemoryPtr<VariableNode>)) {
                std::println(std::cerr, "[Neoluma/Parser] Non-variable value detected in interface after identifier '{}'", name);
                return nullptr;
            }
            vartype = as<VariableNode>(std::move(tmp));
        }
        elements.push_back(makeMemoryPtr<InterfaceFieldNode>(name, vartype, isNullable));
        next();
        if (isNextLine()) next();
        else if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
           // std::println(std::cerr, "[Neoluma/Parser] Delimeter at enum found that is not either '\\n', ';', or right braces.{}", "");
            return nullptr;
        }
        else break;
    }

    return makeMemoryPtr<InterfaceNode>(elements, std::move(decorators), std::move(modifiers));
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
    }
    else {
        MemoryPtr<ASTNode> block = parseStatement();
        if(!block) {
            //std::println("[Neoluma/Parser] Expected statement after 'if/elif/else' condition{}", "");
            return nullptr;
        }
        return std::move(block);
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightBraces])) {
        //std::cerr << "[Neoluma/Parser] Block wasn't closed with '}'.{}\n";
        return nullptr;
    }
}

// Detects nextline expression
bool Parser::isNextLine(){
    if (curToken().value == ";" || curToken().value == "\n") return true;
    return false;
}
