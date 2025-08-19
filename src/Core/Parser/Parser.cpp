#include "nodes.hpp"
#include "parser.hpp"
#include "../Lexer/token.hpp"
#include "../HelperFunctions.hpp"
#include <iostream>
#include <unordered_set>

/*

    ⚠️TRIGGER WARNING!⚠️
    This code may annoy you so much you'll combust,
    because it was made by a high school graduate that doesn't know C++ well yet.

    Please proceed with caution.

*/

// ==== Main parsing ====
MemoryPtr<ASTNode> Parser::parseModule() {
    auto moduleNode = makeMemoryPtr<ModuleNode>();

    while (!isAtEnd()) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::cerr << "[Parser] Failed to parse statement in module\n";
            break;
        }

        moduleNode->body.push_back(std::move(stmt));
    }

    return moduleNode;
}


// ==== Expression parsing ====
MemoryPtr<ASTNode> Parser::parsePrimary(){
    Token token = curToken();

    // Unary operations
    if (token.type == TokenType::Operator && 
        (token.value == "!" || token.value == "-" || token.value == "not")) {
        string op = token.value;
        next();
        return parseUnary(op);
    } 

    // Parenthesis
    if (token.type == TokenType::Delimeter && token.value == "(") {
        next();
        MemoryPtr<ASTNode> expr = parseExpression();
        if (!match(TokenType::Delimeter, ")")) {
            std::cerr << "[Parser] Expected ')' after expression\n";
            return nullptr;
        }
        next();
        return expr;
    } 
    if (token.type == TokenType::Number || token.type == TokenType::String || 
               token.type == TokenType::Boolean || token.type == TokenType::NullLiteral) {
        next();
        return makeMemoryPtr<LiteralNode>(token.value);
    } 
    if (token.type == TokenType::Identifier) {
        Token id = next();
        
        if (match(TokenType::Delimeter, "(")) {
            next();
            array<MemoryPtr<ParameterNode>> args;

            while (!match(TokenType::Delimeter, ")") && !isAtEnd()) {
                auto arg = parseExpression();
                if (arg) args.push_back(makeMemoryPtr<ParameterNode>(std::move(arg)));
                
                if (match(TokenType::Delimeter, ",")) next();
                else break;
            }
            if (!match(TokenType::Delimeter, ")")) {
                std::cerr << "[Parser] Expected ')' after arguments\n";
                return nullptr;
            }

            auto callee = makeMemoryPtr<VariableNode>(id.value); // Variable or identifier
            return makeMemoryPtr<CallExpressionNode>(std::move(callee), args);
        }

        return makeMemoryPtr<VariableNode>(id.value); // Variable or identifier
    }

    std::cerr << "[Parser] Unexpected token in primary expression: " << token.value << "\n";
    return nullptr;
}

MemoryPtr<ASTNode> Parser::parseExpression() {
    return parseBinary(0);
}

MemoryPtr<ASTNode> Parser::parseBinary(int prevPredecence = 0) {
    MemoryPtr<ASTNode> left = parsePrimary();
    if (!left) return nullptr;

    while (true) {
        Token token = curToken();
        int predecence = getOperatorPrecedence(token.value);
        if (token.type != TokenType::Operator || predecence < prevPredecence) break;
        string op = token.value;
        next();

        MemoryPtr<ASTNode> right = parseBinary(predecence+1);
        if (!right) return nullptr;

        MemoryPtr<BinaryOperationNode> binaryOpNode = makeMemoryPtr<BinaryOperationNode>(std::move(left), op, std::move(right));
        left = std::move(binaryOpNode);
    }
    return left;
}

MemoryPtr<UnaryOperationNode> Parser::parseUnary(const string& op) {
    MemoryPtr<ASTNode> operand = parsePrimary();
    if (!operand) {
        std::cerr << "[Parser] Missing operand after unary operator: " << op << "\n";
        return nullptr;
    }
    return makeMemoryPtr<UnaryOperationNode>(op, std::move(operand));
}

// ==== Statement parsing ====

MemoryPtr<ASTNode> Parser::parseStatement() {
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
                std::cerr << "[Parser] Expected expression after 'return'\n";
                return nullptr;
            }
            return makeMemoryPtr<ReturnStatementNode>(expr);
        }

        if (token.value == "throw") {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Parser] Expected expression after 'throw'\n";
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

        if (token.value == "function") return parseFunction();
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

MemoryPtr<AssignmentNode> Parser::parseAssignment(){
    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected identifier for assignment\n";
        return nullptr;
    }
    string variableName = token.value;
    next();

    static const std::unordered_set<std::string> assignmentOps = {
        "=", "+=", "-=", "*=", "/=", "%=", "^="
    };

    token = curToken();
    if ((!match(TokenType::Operator)) || assignmentOps.find(token.value) == assignmentOps.end()) {
        std::cerr << "[Parser] Expected assignment operator (=, +=, -=, and etc.)\n";
        return nullptr;
    }

    string op = token.value;
    next();

    MemoryPtr<ASTNode> value = parseExpression();
    if (!value) {
        std::cerr << "[Parser] Failed to parse expression after assignment\n";
        return nullptr;
    }

    return makeMemoryPtr<AssignmentNode>(variableName, op, std::move(value));
}

MemoryPtr<ReturnStatementNode> Parser::parseReturn(){
    next();

    if (curToken().type == TokenType::Delimeter && curToken().value == ";") {
        return makeMemoryPtr<ReturnStatementNode>(nullptr);
    }

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::cerr << "[Parser] Invalid expression in return statement\n";
        return nullptr;
    }

    if (curToken().type != TokenType::Delimeter || curToken().value != ";") {
        std::cerr << "[Parser] Expected ';' after return expression\n";
        return nullptr;
    }
    next();

    return makeMemoryPtr<ReturnStatementNode>(std::move(expr));
}

// ==== Control flow ====
MemoryPtr<IfNode> Parser::parseIf() {
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'if'\n";
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::cerr << "[Parser] Expected condition expression after 'if'\n";
        return nullptr;
    }
    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after condition in 'if'\n";
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
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'switch'\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::cerr << "[Parser] Invalid expression in switch\n";
        return nullptr;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after switch expression\n";
        return nullptr;
    }
    next();

    if (!match(TokenType::Delimeter, "{")) {
        std::cerr << "[Parser] Expected '{' after switch\n";
        return nullptr;
    }
    next();

    array<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase = nullptr;

    while (!match(TokenType::Delimeter, "}")) {
        Token tok = curToken();

        if (tok.type == TokenType::Keyword && tok.value == "case") {
            next();
            MemoryPtr<ASTNode> condition = parseExpression();
            if (!match(TokenType::Delimeter, ":")) {
                std::cerr << "[Parser] Expected ':' after case condition\n";
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            cases.push_back(makeMemoryPtr<CaseNode>(condition, body));
        }
        else if (tok.type == TokenType::Keyword && tok.value == "default") {
            next();
            if (!match(TokenType::Delimeter, ":")) {
                std::cerr << "[Parser] Expected ':' after default\n";
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            defaultCase = makeMemoryPtr<SCDefaultNode>(body);
        }
        else {
            std::cerr << "[Parser] Unexpected token in switch\n";
            return nullptr;
        }
    }

    return makeMemoryPtr<SwitchNode>(expr, cases, defaultCase);
}

MemoryPtr<TryCatchNode> Parser::parseTryCatch() {
    next();

    auto tryBlock = as<BlockNode>(parseBlockorStatement());
    if (!tryBlock) {
        std::cerr << "[Parser] Expected try block\n";
        return nullptr;
    }

    if (!match(TokenType::Keyword, "catch")) {
        std::cerr << "[Parser] Expected 'catch' after 'try'\n";
        return nullptr;
    }
    next();

    auto catchBlock = as<BlockNode>(parseBlockorStatement());
    if (!catchBlock) {
        std::cerr << "[Parser] Expected catch block\n";
        return nullptr;
    }

    return makeMemoryPtr<TryCatchNode>(tryBlock, catchBlock);
}

MemoryPtr<ForLoopNode> Parser::parseFor() {
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'for'\n";
        return nullptr;
    }
    next();

    if (curToken().type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected identifier in for loop\n";
        return nullptr;
    }

    string varName = curToken().value;
    auto varNode = makeMemoryPtr<VariableNode>(varName, ASTVariableType::Void);
    next();

    if (!match(TokenType::Operator, ":")) {
        std::cerr << "[Parser] Expected ':' after loop variable in for\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        std::cerr << "[Parser] Invalid iterable in for loop\n";
        return nullptr;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after for condition\n";
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        std::cerr << "[Parser] Expected block or statement after for\n";
        return nullptr;
    }

    return makeMemoryPtr<ForLoopNode>(std::move(varNode), std::move(iterable), std::move(body));
}

MemoryPtr<WhileLoopNode> Parser::parseWhile() {
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'while'\n";
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::cerr << "[Parser] Invalid condition in while loop\n";
        return nullptr;
    }
    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after while condition\n";
        return nullptr;
    }

    auto body = as<BlockNode>(parseBlockorStatement());
    if (!body) {
        std::cerr << "[Parser] Expected block or statement after while\n";
        return nullptr;
    }

    return makeMemoryPtr<WhileLoopNode>(std::move(condition), std::move(body));
}

// ==== Declarations ====
MemoryPtr<FunctionNode> Parser::parseFunction() {
    next();

    Token nameToken = curToken();
    if (nameToken.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected function name\n";
        return nullptr;
    }
    string funcName = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after function name\n";
        return nullptr;
    }
    next();

    array<MemoryPtr<ParameterNode>> params;
    while (!match(TokenType::Delimeter, ")")) {
        Token paramName = curToken();
        if (paramName.type != TokenType::Identifier) {
            std::cerr << "[Parser] Expected parameter name\n";
            return nullptr;
        }
        next();

        ASTVariableType type = ASTVariableType::Void; // TODO: detect type later
        params.push_back(makeMemoryPtr<ParameterNode>(paramName.value, type));

        if (match(TokenType::Delimeter, ",")) next();
        else break;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after function parameters\n";
        return nullptr;
    }
    next();

    MemoryPtr<BlockNode> body = parseBlock();
    if (!body) return nullptr;

    return makeMemoryPtr<FunctionNode>(funcName, params, body);
}

MemoryPtr<ClassNode> Parser::parseClass() {
    next();

    Token nameToken = curToken();
    if (nameToken.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected class name\n";
        return nullptr;
    }
    string className = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, "{")) {
        std::cerr << "[Parser] Expected '{' to start class body\n";
        return nullptr;
    }
    next();

    array<MemoryPtr<VariableNode>> fields;
    array<MemoryPtr<FunctionNode>> methods;

    while (!match(TokenType::Delimeter, "}")) {
        Token t = curToken();

        if (t.type == TokenType::Keyword && t.value == "function") {
            auto method = parseFunction();
            if (method) methods.push_back(std::move(method));
        }
        else if (t.type == TokenType::Identifier) {
            string fieldName = t.value;
            next();
            ASTVariableType varType = ASTVariableType::Void;
            fields.push_back(makeMemoryPtr<VariableNode>(fieldName, varType));
            if (match(TokenType::Delimeter, ";")) next();
        }
        else {
            std::cerr << "[Parser] Unexpected token in class body\n";
            break;
        }
    }

    next();
    return makeMemoryPtr<ClassNode>(className, fields, methods);
}

MemoryPtr<BlockNode> Parser::parseBlock() {
    if (!match(TokenType::Delimeter, "{")) {
        std::cerr << "[Parser] Expected '{' to start block\n";
        return nullptr;
    }
    next();

    auto block = makeMemoryPtr<BlockNode>();

    while (!match(TokenType::Delimeter, "}")) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::cerr << "[Parser] Failed to parse statement in block\n";
            return nullptr;
        }
        block->statements.push_back(std::move(stmt));
        if (match(TokenType::Delimeter, ";")) next();
    }

    next();
    return block;
}

MemoryPtr<DecoratorNode> Parser::parseDecorator() {
    next(); // consume '@'

    Token nameToken = curToken();
    if (nameToken.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected decorator name\n";
        return nullptr;
    }
    string name = nameToken.value;
    next();

    array<MemoryPtr<ParameterNode>> params;
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
            std::cerr << "[Parser] Unterminated decorator params\n";
            return nullptr;
        }
        next(); // consume ')'
    }

    return makeMemoryPtr<DecoratorNode>(name, params, nullptr);
}

MemoryPtr<PreprocessorDirectiveNode> Parser::parsePreprocessor() {
    next(); // consume '#'
    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected preprocessor directive\n";
        return nullptr;
    }

    ASTPreprocessorDirectiveType type = ASTPreprocessorDirectiveType::Use; // TODO: Map properly
    return makeMemoryPtr<PreprocessorDirectiveNode>(type, token.value);
}


// ==== Helper functions ====

// Parses either a block or a single statement after an 'if' condition.
MemoryPtr<ASTNode> Parser::parseBlockorStatement() {
    if (match(TokenType::Delimeter, "{")) {
        MemoryPtr<BlockNode> block = parseBlock();
        if (!block) {
            std::cerr << "[Parser] Expected block after 'if/elif/else' condition\n";
            return nullptr;
        }
        return std::move(block);
    }
    else {
        MemoryPtr<ASTNode> block = parseStatement();
        if (!block) {
            std::cerr << "[Parser] Expected statement after 'if/elif/else' condition\n";
            return nullptr;
        }
        return std::move(block);
    }
}