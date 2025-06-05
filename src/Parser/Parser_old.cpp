
#include "nodes.hpp"
#include "../Lexer/token.hpp"
#include "parser.hpp"

#include <iostream>
#include <vector>

MemoryPtr<ASTNode> Parser::parseLiteral(){
    Token token = curToken();
    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::Literal, token.value);
    return node;
}

MemoryPtr<ASTNode> Parser::parsePrimary() {
    Token token = curToken();
    if (token.type == TokenType::Operator &&
        (token.value == "!" || token.value == "-" || token.value == "not")) {
            next();
            MemoryPtr<ASTNode> operand = parsePrimary();

            MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::UnaryOperation, token.value);
            node->children.push_back(operand);
            return node;
        }
    else if (token.type == TokenType::Delimeter && token.value == "(") {
        next();
        MemoryPtr<ASTNode> expr = parseExpression();
        if (!match(TokenType::Delimeter, ")")){
            std::cerr << "[Parser] Expected ')' after expression\n";
            return nullptr;
        }
        next();
        return expr;
    }
    else if (token.type == TokenType::Number || token.type == TokenType::String || 
        token.type == TokenType::Boolean || token.type == TokenType::NullLiteral) {
            return parseLiteral();
        }
    else if (token.type == TokenType::Identifier) {
        next();

        if (curToken().type == TokenType::Delimeter && curToken().value == "(") {
            next();
            MemoryPtr<ASTNode> call = new ASTNode(ASTNodeType::CallExpression, token.value);

            while (curToken().value != ")" && !isAtEnd()) {
                MemoryPtr<ASTNode> arg = parseExpression();
                if (arg) call->children.push_back(arg);
                
                if (curToken().value == ",") next();
                else break;
            }
            if (curToken().value != ")") {
                std::cerr << "[Parser] Expected ')' after arguments\n";
                return nullptr;
            }
            next();
            return call;
        }

        return new ASTNode(ASTNodeType::Variable, token.value);
    }
    return nullptr;
};
MemoryPtr<ASTNode> Parser::parseExpression(){
    return parseBinaryOp(0);
}

MemoryPtr<ASTNode> Parser::parseStatement(){
    Token token = curToken();
    
    if (token.type == TokenType::Keyword) {
        if (token.value == "if") return parseIf();
        if (token.value == "switch") return parseSwitchCase();
        if (token.value == "try") return parseTryCatch();
        if (token.value == "for") return parseForLoop();
        if (token.value == "while") return parseWhileLoop();
        if (token.value == "foreach") return parseForeachLoop();
        if (token.value == "function") return parseFunction();
        if (token.value == "class") return parseClass();
        if (token.value == "async") return parseAsync();
        if (token.value == "@") return parseDecorator();
        if (token.value == "#") return parsePreprocessor();
        if (token.value == "return") {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::cerr << "[Parser] Expected expression after 'return'\n";
                return nullptr;
            }
            MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::ReturnStatement);
            node->children.push_back(expr);
            return node;
        } 
    }
    if (token.type == TokenType::Identifier) {
        Token nextToken = lookupNext();
        if (nextToken.type == TokenType::Operator && (
            nextToken.value == "=" || nextToken.value == "+=" || 
            nextToken.value == "-=" || nextToken.value == "*=" || 
            nextToken.value == "/=" || nextToken.value == "%=")) {
            return parseAssignment();
        }
        // todo: function calls.
    }

    return parseExpression();
}
MemoryPtr<ASTNode> Parser::parseAssignment(){
    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected identifier for assignment\n";
        return nullptr;
    }
    std::string varName = token.value;
    next();

    if ((!match(TokenType::Operator)) || 
        (token.value != "=" && token.value != "+=" && token.value != "-=" 
            && token.value != "*=" && token.value != "/=" && token.value != "%=")) {
        std::cerr << "[Parser] Expected assignment operator (=, +=, -=, and etc.)\n";
        return nullptr;
    }
    std::string op = token.value;
    MemoryPtr<ASTNode> assignmentOpNode = new ASTNode(ASTNodeType::Literal, op);
    next();

    MemoryPtr<ASTNode> value = parseExpression();
    if (!value) {
        std::cerr << "[Parser] Failed to parse expression for assignment\n";
        return nullptr;
    }

    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::Assignment, varName);
    node->children.push_back(assignmentOpNode);
    node->children.push_back(value);
    return node;
}
MemoryPtr<ASTNode> Parser::parseBinaryOp(int prevPrecedence) {
    MemoryPtr<ASTNode> left = parsePrimary();
    if (!left) return nullptr;

    while (true) {
        Token token = curToken();
        int precedence = getOperatorPrecedence(token.value);
        if (token.type != TokenType::Operator || precedence < prevPrecedence) break;

        next();

        MemoryPtr<ASTNode> right = parseBinaryOp(precedence + 1);
        if (!right) return nullptr;

        MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::BinaryOperation, token.value);
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}

MemoryPtr<ASTNode> Parser::parseIf(){
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
    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::IfStatement);
    node->children.push_back(condition);

    next();
    parseBlockorStatement(node);
    
    while (match(TokenType::Keyword, "elif")) {
        next();
        if (!match(TokenType::Delimeter, "(")) {
            std::cerr << "[Parser] Expected '(' after 'elif'\n";
            return nullptr;
        }
        next();
        MemoryPtr<ASTNode> elifCondition = parseExpression();
        if (!elifCondition) {
            std::cerr << "[Parser] Expected condition expression after 'elif'\n";
            return nullptr;
        }
        if (!match(TokenType::Delimeter, ")")) {
            std::cerr << "[Parser] Expected ')' after condition in 'elif'\n";
            return nullptr;
        }
        MemoryPtr<ASTNode> elifNode = new ASTNode(ASTNodeType::IfStatement);
        elifNode->children.push_back(elifCondition);
        next();
        parseBlockorStatement(elifNode);
        node->children.push_back(elifNode);
    }
    if (match(TokenType::Keyword, "else")) {
        next();
        parseBlockorStatement(node);
    }
    return node;
}

MemoryPtr<ASTNode> Parser::parseSwitchCase(){
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'switch'\n";
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::cerr << "[Parser] Expected condition expression after 'switch'\n";
        return nullptr;
    }
    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after condition in 'switch'\n";
        return nullptr;
    }
    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::Switch);
    node->children.push_back(condition);

    next();
    if (!match(TokenType::Delimeter, "{")) {
        std::cerr << "[Parser] Expected '{' after 'switch' condition\n";
        return nullptr;
    }
    next();

    while (match(TokenType::Keyword, "case")) {
        next();
        MemoryPtr<ASTNode> caseCondition = parseExpression();
        if (!caseCondition) {
            std::cerr << "[Parser] Expected case condition expression\n";
            return nullptr;
        }
        if (!match(TokenType::Delimeter, ":")) {
            std::cerr << "[Parser] Expected ':' after case condition\n";
            return nullptr;
        }
        next();

        MemoryPtr<ASTNode> caseNode = new ASTNode(ASTNodeType::Case);
        caseNode->children.push_back(caseCondition);
        
        while (!isAtEnd() && !match(TokenType::Keyword, "case") && !match(TokenType::Keyword, "default") && !match(TokenType::Delimeter, "}")) {
            MemoryPtr<ASTNode> statement = parseStatement();
            if (statement) caseNode->children.push_back(statement);
            else break; // Exit on error or end of statements
        }
        
        node->children.push_back(caseNode);
    }

    if (match(TokenType::Keyword, "default")) {
        next();
        if (!match(TokenType::Delimeter, ":")) {
            std::cerr << "[Parser] Expected ':' after 'default'\n";
            return nullptr;
        }
        next();

        MemoryPtr<ASTNode> defaultNode = new ASTNode(ASTNodeType::SCDefault);
        
        while (!isAtEnd() && !match(TokenType::Delimeter, "}")) {
            MemoryPtr<ASTNode> statement = parseStatement();
            if (statement) defaultNode->children.push_back(statement);
            else break; // Exit on error or end of statements
        }
        node->children.push_back(defaultNode);
    }
    if (!match(TokenType::Delimeter, "}")) {
        std::cerr << "[Parser] Expected '}' to close 'switch' block\n";
        return nullptr;
    }
    next();
    return node;
}
MemoryPtr<ASTNode> Parser::parseTryCatch(){
    next();
    if (!match(TokenType::Delimeter, "{")) {
        std::cerr << "[Parser] Expected '{' after 'try'\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> tryBlock = parseBlock();
    if (!tryBlock) {
        std::cerr << "[Parser] Expected block after 'try'\n";
        return nullptr;
    }

    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::TryCatch);
    node->children.push_back(tryBlock);

    if (!match(TokenType::Keyword, "catch")) {
        std::cerr << "[Parser] Expected 'catch' after 'try'\n";
        return nullptr;
    } 
    else {
        next();
        if (!match(TokenType::Delimeter, "(")) {
            std::cerr << "[Parser] Expected '(' after 'catch'\n";
            return nullptr;
        }
        next();

        Token catchToken = curToken();
        if (catchToken.type != TokenType::Identifier) {
            std::cerr << "[Parser] Expected identifier for catch variable\n";
            return nullptr;
        }
        std::string catchVar = catchToken.value;
        next();

        if (!match(TokenType::Delimeter, ")")) {
            std::cerr << "[Parser] Expected ')' after catch variable\n";
            return nullptr;
        }
        next();

        if (!match(TokenType::Delimeter, "{")) {
            std::cerr << "[Parser] Expected '{' after 'catch'\n";
            return nullptr;
        }
        next();

        MemoryPtr<ASTNode> catchBlock = parseBlock();
        if (!catchBlock) {
            std::cerr << "[Parser] Expected block after 'catch'\n";
            return nullptr;
        }

        MemoryPtr<ASTNode> catchNode = new ASTNode(ASTNodeType::Variable, catchVar);
        node->children.push_back(catchNode);
        node->children.push_back(catchBlock);
    }

    return node;
}

MemoryPtr<ASTNode> Parser::parseForLoop(){
    next();
    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'for'\n";
        return nullptr;
    }
    next();

    Token varToken = curToken();
    if (varToken.type != TokenType::Identifier) {
        std::cerr << "[Parser] Expected identifier for 'for' loop variable\n";
        return nullptr;
    }
    std::string varName = varToken.value;
    next();

    if (!match(TokenType::Operator, "in") && !match(TokenType::Keyword, ":")) {
        std::cerr << "[Parser] Expected 'in' or ':' after loop variable in 'for' loop\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        std::cerr << "[Parser] Expected iterable expression in 'for' loop\n";
        return nullptr;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after 'for' loop condition\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::ForLoop);
    node->children.push_back(new ASTNode(ASTNodeType::Variable, varName));
    node->children.push_back(iterable);

    parseBlockorStatement(node);

    return node;
}
MemoryPtr<ASTNode> Parser::parseWhileLoop(){
    next();

    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after 'while'\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::cerr << "[Parser] Expected condition expression after 'while'\n";
        return nullptr;
    }

    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' after condition in 'while'\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::WhileLoop);
    node->children.push_back(condition);
    
    parseBlockorStatement(node);

    return node;
}

// Please forgive me for this utter dog..., i hope i never have to refactor this code ever. ever. ever...
// I hate this code so unbelievably much...
MemoryPtr<ASTNode> Parser::parseFunction(){
    MemoryPtr<ASTNode> node = new ASTNode(ASTNodeType::Function);

    while (match(TokenType::Decorator)) {
        MemoryPtr<ASTNode> decoratorNode = parseDecorator();
        if (decoratorNode) {
            node->children.push_back(decoratorNode);
            next();
        } else {
            std::cerr << "[Parser] Error parsing decorator\n";
            return nullptr;
        }
    }

    while (match(TokenType::Keyword) && (curToken().value == "public" || curToken().value == "private" || curToken().value == "protected" || curToken().value == "static")) {
        MemoryPtr<ASTNode> accessModifierNode = new ASTNode(ASTNodeType::Literal, curToken().value);
        node->children.push_back(accessModifierNode);
        next();
    }

    if (!match(TokenType::Type)) {
        std::cerr << "[Parser] Expected return type for function\n";
        return nullptr;
    }
    MemoryPtr<ASTNode> returnTypeNode = new ASTNode(ASTNodeType::Literal, curToken().value);
    node->children.push_back(returnTypeNode);
    next();

    if (!match(TokenType::Identifier)) {
        std::cerr << "[Parser] Expected function name\n";
        return nullptr;
    }
    std::string functionName = curToken().value;
    next();

    if (!match(TokenType::Delimeter, "(")) {
        std::cerr << "[Parser] Expected '(' after function name\n";
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> parametersNode = new ASTNode(ASTNodeType::Parameter);
    while (!isAtEnd() && !match(TokenType::Delimeter, ")")) {
        if (match(TokenType::Type)) {
            MemoryPtr<ASTNode> paramTypeNode = new ASTNode(ASTNodeType::Literal, curToken().value);
            parametersNode->children.push_back(paramTypeNode);
            next();
        } else {
            std::cerr << "[Parser] Expected parameter type\n";
            return nullptr;
        }

        if (!match(TokenType::Identifier)) {
            std::cerr << "[Parser] Expected parameter name\n";
            return nullptr;
        }
        MemoryPtr<ASTNode> paramNameNode = new ASTNode(ASTNodeType::Variable, curToken().value);
        parametersNode->children.push_back(paramNameNode);
        next();

        if (match(TokenType::Delimeter, ",")) {
            next();
        }
    }
    if (!match(TokenType::Delimeter, ")")) {
        std::cerr << "[Parser] Expected ')' to close function parameters\n";
        return nullptr;
    }
    next();
    node->children.push_back(parametersNode);

    parseBlockorStatement(node);

    return node;
}
MemoryPtr<ASTNode> Parser::parseClass(){}
MemoryPtr<ASTNode> Parser::parseBlock(){
    if (!match(TokenType::Delimeter, "{")) {
        std::cerr << "[Parser] Expected '{' to start block\n";
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> blockNode = new ASTNode(ASTNodeType::Block);
    
    while (!isAtEnd() && !match(TokenType::Delimeter, "}")) {
        MemoryPtr<ASTNode> statement = parseStatement();
        if (statement) {
            blockNode->children.push_back(statement);
        } else {
            std::cerr << "[Parser] Error parsing statement in block\n";
            return nullptr;
        }
    }

    if (!match(TokenType::Delimeter, "}")) {
        std::cerr << "[Parser] Expected '}' to close block\n";
        return nullptr;
    }
    next();
    
    return blockNode;
}

MemoryPtr<ASTNode> Parser::parseImport(){}
MemoryPtr<ASTNode> Parser::parseAsync(){}
MemoryPtr<ASTNode> Parser::parseDecorator(){}
MemoryPtr<ASTNode> Parser::parsePreprocessor(){}

// Helper functions

// Parses either a block or a single statement after an 'if' condition.
void Parser::parseBlockorStatement(MemoryPtr<ASTNode> node) {
    if (match(TokenType::Delimeter, "{")) {
        MemoryPtr<ASTNode> ifBlock = parseBlock();
        if (!ifBlock) {
            std::cerr << "[Parser] Expected block after 'if/elif/else' condition\n";
            return;
        }
        node->children.push_back(ifBlock);
    }
    else {
        MemoryPtr<ASTNode> ifBlock = parseStatement();
        if (!ifBlock) {
            std::cerr << "[Parser] Expected statement after 'if/elif/else' condition\n";
            return;
        }
        node->children.push_back(ifBlock);
    }
}