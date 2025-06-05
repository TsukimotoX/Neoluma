#include "nodes.hpp"
#include "parser.hpp"
#include "../Lexer/token.hpp"
#include "HelperFunctions.hpp"
#include <iostream>

MemoryPtr<ASTNode> Parser::parsePrimary(){
    Token token = curToken();

    // Unary operations
    if (token.type == TokenType::Operator && 
        (token.value == "!" || token.value == "-" || token.value == "not")) {
        next();
        MemoryPtr<ASTNode> operand = parsePrimary();
        return makeMemoryPtr<UnaryOperationNode>(token.value, std::move(operand));
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