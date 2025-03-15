#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "lexer.h"

enum class ExpressionType {
    Literal, BinaryOp, UnaryOp, Variable, Keyword, Assignment, IfStatement
};

struct Expression {
    ExpressionType type;
    std::string value;
    Expression* left = nullptr;
    Expression* right = nullptr;
};

struct IfStatementExpression : public Expression {
    Expression* condition;
    std::vector<Expression*> ifBlock;
    std::vector<Expression*> elseBlock;

    IfStatementExpression(Expression* conditionExpr, std::vector<Expression*> ifB, std::vector<Expression*> elseB)
        : condition(conditionExpr), ifBlock(ifB), elseBlock(elseB) {
        type = ExpressionType::IfStatement;
    }
};


struct Program {
    std::vector<Expression*> expressions;
};

int getOperatorPrecedence(const std::string& op);

bool isOperator(const Token& token);
bool isKeyword(const Token& token, const std::string& keyword);
bool isDelimiter(const Token& token, const std::string& delimiter);
bool isNumber(const Token& token);
bool isVariable(const Token& token);

Expression* createExpression(ExpressionType type, const std::string& value);

Expression* parseExpression(const std::vector<Token>& tokens, size_t& pos, int precedence);
Expression* parsePrimaryExpression(const std::vector<Token>& tokens, size_t& pos);

Expression* parseIfStatement(const std::vector<Token>& tokens, size_t& pos);

std::vector<Expression*> parse(const std::vector<Token>& tokens);
Program parseProgram(const std::vector<Token>& tokens);
void printExpression(Expression* expr, const std::string& prefix);
void printProgram(const Program& program);

#endif