#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include "lexer.h"
#include "parser.h"

// Helpful functions to make code look better
std::unique_ptr<Expression> Parser::makeExpression(ExpressionType type, std::string value) {
    std::unique_ptr<Expression> result = std::make_unique<Expression>(type, value);
    Parser::nextToken();
    return result;
}
std::unique_ptr<Expression> Parser::makeUnaryOp(const std::string& op, std::unique_ptr<Expression> operand) {
    auto expr = std::make_unique<Expression>(ExpressionType::UnaryOp, op);
    expr->children.push_back(std::move(operand));
    return expr;
}


bool isUnaryOp(const std::string& op) {
    static const std::unordered_set<std::string> unaryOps = {"-", "!", "~"};
    return unaryOps.find(op) != unaryOps.end();
}

bool isBinaryOp(const std::string& op) {
    static const std::unordered_set<std::string> binaryOps = {"+", "-", "*", "/", "^", "&&", "||", "==", "!=", "<", ">", "<=", ">="};
    return binaryOps.find(op) != binaryOps.end();
}

std::string getExpressionTypeName(ExpressionType type) {
    switch (type) {
        case ExpressionType::Literal: return "Literal";
        case ExpressionType::BinaryOp: return "BinaryOp";
        case ExpressionType::UnaryOp: return "UnaryOp";
        case ExpressionType::Variable: return "Variable";
        case ExpressionType::Keyword: return "Keyword";
        case ExpressionType::Assignment: return "Assignment";
        case ExpressionType::IfStatement: return "IfStatement";
        case ExpressionType::Loop: return "Loop";
        default: return "Unknown";
    }
}


// Parser itself
std::unique_ptr<Expression> Parser::parseExpression(int predecence) {
    auto left = parsePrimaryExpression(); // Get the left side of expression

    // Check on if it's assignment
    if (!isEnd() && currentToken().type == TokenType::OPERATOR && currentToken().value == "=") {
        nextToken(); // Skip '=' sign
        auto right = parseExpression(); // Anything that's on right
        auto expr = std::make_unique<Expression>(ExpressionType::Assignment, "=");
        expr->children.push_back(std::move(left));
        expr->children.push_back(std::move(right));
        return expr;
    }

    while (!isEnd()){ // While its not till the end of the code
        Token token = currentToken(); // Get current token
        if (token.type != TokenType::OPERATOR) break; // if its just a value then don't process it, go to next

        int opPredecence = getOperatorPrecedence(token.value); // If it IS an operator, we get its priority degree
        if (opPredecence < predecence) break; // If our priority is lower, stop it

        nextToken(); // Skip the operator
        auto right = parseExpression(opPredecence + 1); // Recursion on right side of expression, with nesting going deeper on higher level operator.
        
        // We make a new expression that is binary operation between the two.
        auto binaryOp = std::make_unique<Expression>(ExpressionType::BinaryOp, token.value);
        binaryOp->children.push_back(std::move(left)); // Add left operand
        binaryOp->children.push_back(std::move(right)); // Add right operand

        left = std::move(binaryOp); // Now its a new binary operation written
    }

    return left;
}

/*  This function parses the primary variable for the expression. Like: is it a number, is it an operator and etc.
    This helps us to build up an order to perform expressions, like mathematic orders. 
    Like: 1 + 2 * 3 will be 1 + (2 * 3) for program. It will perform the one in brackets first, from top to bottom.
*/
std::unique_ptr<Expression> Parser::parsePrimaryExpression() {
    Token token = currentToken(); // Gets current token

    switch (token.type) { // Looks what type the token is
        case TokenType::OPERATOR:
            if (isUnaryOp(token.value)) { //If its an unary operator
                std::string op = token.value; // Get the operator
                nextToken(); // Move on to second expression
                return makeUnaryOp(op, std::move(parsePrimaryExpression())); // Parse the entire unary operation.
                // You can always look up what the function is, i just make abstracted code because it looks cleaner,
                // because idk, maybe little Timmy will look up to it and create something better than i do.
            }
            if (token.value == "(") { // if it's something in brackets
                nextToken();
                auto innerExpression = parseExpression(); // parse expression inside it
                if (currentToken().value != ")") throw std::runtime_error("Expected ')'. Like skipping brackets? :D");
                nextToken();
                return innerExpression;
            }
        case TokenType::NUMBER: return makeExpression(ExpressionType::Literal, token.value);
        case TokenType::VARIABLE: return makeExpression(ExpressionType::Variable, token.value);
        case TokenType::KEYWORD: 
        default:
            throw std::runtime_error("Unexpected token in primary expression: " + token.value);
    }
}

// Prints the AbstractSyntaxTree into the console.
void Parser::printAST(const std::unique_ptr<Expression>& expr, const std::string& indent, bool isLast) {
    if (!expr) return; // If there's nothing to print just stop

    std::cout << indent; // Prints indent string
    if (isLast) { // Checks if the thing in the tree is last.
        std::cout << "└──";
    } else {
        std::cout << "├──";
    }

    std::cout << "[" << expr->value << "]" << " (" << getExpressionTypeName(expr->type) << ")" << std::endl; // Prints the expression
    std::string newIndent = indent + (isLast ? "    " : "│   "); // Adds nested indentation
    for (size_t i = 0; i < expr->children.size(); ++i) {
        printAST(expr->children[i], newIndent, i == expr->children.size() - 1); // Prints nested expressions recursively
    }
}

std::unique_ptr<Expression> Parser::parseProgram() {
    auto program = std::make_unique<Expression>(ExpressionType::Program, "Program");

    while (!isEnd()) {
        auto expr = parseExpression();
        if (expr) {
            program->children.push_back(std::move(expr));
        }

        // Пропускаем `;`, если есть
        if (currentToken().type == TokenType::DELIMITER && currentToken().value == ";") {
            nextToken();
        }
    }

    return program;
}
