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
    
}

// ==== Main parsing ====
MemoryPtr<ModuleNode> Parser::parseModule() {
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
    Token token = curToken();

    // === Control Flow Keywords ===
    if (token.type == NTokenType::Keyword) {
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
    if (token.type == NTokenType::Identifier) {
        Token nextTok = lookupNext();

        if (nextTok.type == NTokenType::Operator && (
            nextTok.value == "=" || nextTok.value == "+=" || nextTok.value == "-=" ||
            nextTok.value == "*=" || nextTok.value == "/=" || nextTok.value == "%=" ||
            nextTok.value == "^=")) {
            return parseAssignment();
        }

        // TODO: Maybe function call? detect "(" after identifier
        // TODO: Variable declaration later?
    }

    // === Block ===
    if (token.type == NTokenType::Delimeter && token.value == "{") {
        return parseBlock();
    }

    // === Fallback ===
    return parseExpression(); // Default to expression statement
}

// ==== Expression parsing ====
MemoryPtr<ASTNode> Parser::parsePrimary(){
    Token token = curToken();

    // Unary operations
    if (token.type == NTokenType::Operator && 
        (token.value == "!" || token.value == "-" || token.value == "not")) {
        std::string op = token.value;
        next();
        return parseUnary(op);
    } 

    // Parenthesis
    else if (token.type == NTokenType::Delimeter && token.value == "(") {
        next();
        MemoryPtr<ASTNode> expr = parseExpression();
        if (!match(NTokenType::Delimeter, ")")) {
            std::println(std::cerr, "[Neoluma/Parser] Expected ')' after expression");
            return nullptr;
        }
        next();
        return expr;
    } 
    // Data type
    else if (token.type == NTokenType::Number || token.type == NTokenType::String || 
               token.type == NTokenType::Boolean || token.type == NTokenType::NullLiteral) {
        next();
        return makeMemoryPtr<LiteralNode>(token.value);
    } 
    // Identifier or variable
    if (token.type == NTokenType::Identifier) {
        Token id = next();
        
        if (match(NTokenType::Delimeter, "(")) {
            next();
            std::vector<MemoryPtr<ParameterNode>> args;

            while (!match(NTokenType::Delimeter, ")") && !isAtEnd()) {
                auto arg = parseExpression();
                if (arg) args.push_back(makeMemoryPtr<ParameterNode>(std::move(arg)));
                
                if (match(NTokenType::Delimeter, ",")) next();
                else break;
            }
            if (!match(NTokenType::Delimeter, ")")) {
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
    return parseBinary(0);
}

MemoryPtr<ASTNode> Parser::parseBinary(int prevPredecence = 0) {
    MemoryPtr<ASTNode> left = parsePrimary();
    if (!left) return nullptr;

    while (true) {
        Token token = curToken();
        int predecence = getOperatorPrecedence(token.value);
        if (token.type != NTokenType::Operator || predecence < prevPredecence) break;
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
    if (token.type != NTokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected identifier for assignment");
        return nullptr;
    }
    std::string variableName = token.value;
    next();

    static const std::unordered_set<std::string> assignmentOps = {
        "=", "+=", "-=", "*=", "/=", "%=", "^="
    };

    token = curToken();
    if ((!match(NTokenType::Operator)) || assignmentOps.find(token.value) == assignmentOps.end()) {
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
    next();

    if (curToken().type == NTokenType::Delimeter && curToken().value == ";") {
        return makeMemoryPtr<ReturnStatementNode>(nullptr);
    }

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid expression in return statement");
        return nullptr;
    }

    if (curToken().type != NTokenType::Delimeter || curToken().value != ";") {
        std::println(std::cerr, "[Neoluma/Parser] Expected ';' after return expression");
        return nullptr;
    }
    next();

    return makeMemoryPtr<ReturnStatementNode>(std::move(expr));
}

// ==== Control flow ====
MemoryPtr<IfNode> Parser::parseIf() {
    next();
    if (!match(NTokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'if'");
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser] Expected condition expression after 'if'");
        return nullptr;
    }
    if (!match(NTokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after condition in 'if'");
        return nullptr;
    }

    MemoryPtr<ASTNode> ifBlock = parseBlockorStatement();
    MemoryPtr<ASTNode> elseBlock = nullptr;

    if (match(NTokenType::Keyword, "else")) {
        next();
        elseBlock = parseBlockorStatement();
    }

    return makeMemoryPtr<IfNode>(condition, ifBlock, elseBlock);
}

MemoryPtr<SwitchNode> Parser::parseSwitch() {
    next();
    if (!match(NTokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'switch'");
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid expression in switch");
        return nullptr;
    }

    if (!match(NTokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after switch expression");
        return nullptr;
    }
    next();

    if (!match(NTokenType::Delimeter, "{")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{' after switch");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase = nullptr;

    while (!match(NTokenType::Delimeter, "}")) {
        Token tok = curToken();

        if (tok.type == NTokenType::Keyword && tok.value == "case") {
            next();
            MemoryPtr<ASTNode> condition = parseExpression();
            if (!match(NTokenType::Delimeter, ":")) {
                std::println(std::cerr, "[Neoluma/Parser] Expected ':' after case condition");
                return nullptr;
            }
            next();
            auto body = as<BlockNode>(parseBlockorStatement());
            cases.push_back(makeMemoryPtr<CaseNode>(condition, body));
        }
        else if (tok.type == NTokenType::Keyword && tok.value == "default") {
            next();
            if (!match(NTokenType::Delimeter, ":")) {
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
    next();

    auto tryBlock = as<BlockNode>(parseBlockorStatement());
    if (!tryBlock) {
        std::println(std::cerr, "[Neoluma/Parser] Expected try block");
        return nullptr;
    }

    if (!match(NTokenType::Keyword, "catch")) {
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
    next();
    if (!match(NTokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'for'");
        return nullptr;
    }
    next();

    if (curToken().type != NTokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected identifier in for loop");
        return nullptr;
    }

    std::string varName = curToken().value;
    auto varNode = makeMemoryPtr<VariableNode>(varName, ASTVariableType::Void);
    next();

    if (!match(NTokenType::Operator, ":")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ':' after loop variable in for");
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid iterable in for loop");
        return nullptr;
    }

    if (!match(NTokenType::Delimeter, ")")) {
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
    next();
    if (!match(NTokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after 'while'");
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser] Invalid condition in while loop");
        return nullptr;
    }
    if (!match(NTokenType::Delimeter, ")")) {
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
    next();

    Token nameToken = curToken();
    if (nameToken.type != NTokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected function name");
        return nullptr;
    }
    std::string funcName = nameToken.value;
    next();

    if (!match(NTokenType::Delimeter, "(")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '(' after function name");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    while (!match(NTokenType::Delimeter, ")")) {
        Token paramName = curToken();
        if (paramName.type != NTokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser] Expected parameter name");
            return nullptr;
        }
        next();

        ASTVariableType type = ASTVariableType::Void; // TODO: detect type later
        params.push_back(makeMemoryPtr<ParameterNode>(paramName.value, type));

        if (match(NTokenType::Delimeter, ",")) next();
        else break;
    }

    if (!match(NTokenType::Delimeter, ")")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected ')' after function parameters");
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
    if (nameToken.type != NTokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected class name");
        return nullptr;
    }
    std::string className = nameToken.value;
    next();

    if (!match(NTokenType::Delimeter, "{")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{' to start class body");
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<VariableNode>> fields;
    std::vector<MemoryPtr<FunctionNode>> methods;

    while (!match(NTokenType::Delimeter, "}")) {
        Token t = curToken();

        if (t.type == NTokenType::Keyword && t.value == "function") {
            auto method = parseFunction();
            if (method) methods.push_back(std::move(method));
        }
        else if (t.type == NTokenType::Identifier) {
            std::string fieldName = t.value;
            next();
            ASTVariableType varType = ASTVariableType::Void;
            fields.push_back(makeMemoryPtr<VariableNode>(fieldName, varType));
            if (match(NTokenType::Delimeter, ";")) next();
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
    if (!match(NTokenType::Delimeter, "{")) {
        std::println(std::cerr, "[Neoluma/Parser] Expected '{' to start block");
        return nullptr;
    }
    next();

    auto block = makeMemoryPtr<BlockNode>();

    while (!match(NTokenType::Delimeter, "}")) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser] Failed to parse statement in block");
            return nullptr;
        }
        block->statements.push_back(std::move(stmt));
        if (match(NTokenType::Delimeter, ";")) next();
    }

    next();
    return block;
}

MemoryPtr<DecoratorNode> Parser::parseDecorator() {
    next(); // consume '@'

    Token nameToken = curToken();
    if (nameToken.type != NTokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected decorator name");
        return nullptr;
    }
    std::string name = nameToken.value;
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    if (match(NTokenType::Delimeter, "(")) {
        next(); // consume '('
        while (!match(NTokenType::Delimeter, ")")) {
            Token p = curToken();
            if (p.type != NTokenType::Identifier) break;
            ASTVariableType dummy = ASTVariableType::Void;
            params.push_back(makeMemoryPtr<ParameterNode>(p.value, dummy));
            next();
            if (match(NTokenType::Delimeter, ",")) next();
            else break;
        }
        if (!match(NTokenType::Delimeter, ")")) {
            std::println(std::cerr, "[Neoluma/Parser] Unterminated decorator params");
            return nullptr;
        }
        next(); // consume ')'
    }

    return makeMemoryPtr<DecoratorNode>(name, params, nullptr);
}

MemoryPtr<PreprocessorDirectiveNode> Parser::parsePreprocessor() {
    next(); // consume '#'
    Token token = curToken();
    if (token.type != NTokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser] Expected preprocessor directive");
        return nullptr;
    }

    ASTPreprocessorDirectiveType type = ASTPreprocessorDirectiveType::Import; // TODO: Map properly
    return makeMemoryPtr<PreprocessorDirectiveNode>(type, token.value);
}

// ==== Helper functions ====

// Parses either a block or a single statement after an 'if' condition.
MemoryPtr<ASTNode> Parser::parseBlockorStatement() {
    if (match(NTokenType::Delimeter, "{")) {
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
        throw std::runtime_error(errorMsg);
    }
}