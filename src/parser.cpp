#include <memory>
#include <iostream>
#include "parser.h"
#include "lexer.h"


// Private stuff
Token Parser::currentToken() {
    if (curPos < tokens.size()) {
        return tokens[curPos];
    }
    return Token{TokenType::END_OF_FILE, ""};
}

void Parser::nextToken() {
    curPos++;
}

std::unique_ptr<ASTNode> Parser::parsePrimary(){
    Token token = currentToken();

    if (token.type == TokenType::NUMBER){
        nextToken();
        return std::make_unique<LiteralNode>(token.value);
    }

    if (token.type == TokenType::IDENTIFIER){
        nextToken();
        return std::make_unique<VariableNode>(token.value);
    }

    throw std::runtime_error("Unexpected token: " + token.value);
}

std::unique_ptr<ASTNode> Parser::parseExpression(){
    auto left = parsePrimary();

    while (currentToken().type == TokenType::OPERATOR){
        Token op = currentToken();
        nextToken();
        auto right = parsePrimary();
        left = std::make_unique<BinaryOperationNode>(std::move(left), op, std::move(right));
    }

    return left;
}

// Public stuff
std::unique_ptr<ASTNode> Parser::parse(){
    return parseExpression();
}