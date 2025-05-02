#ifndef NODES_H
#define NODES_H

#include <vector>
#include <string>
#include <memory>

enum class ExpressionType {
    Program,
    Literal, Variable, Keyword,
    BinaryOp, UnaryOp,
    Assignment, FunctionCall,
    IfStatement, Loop, Block,
    Function, ReturnStatement
};

struct Expression {
    ExpressionType type;
    std::string value;
    std::vector<std::unique_ptr<Expression>> children;

    Expression(ExpressionType type, const std::string value = "") 
        : type(type), value(value) {}
};

struct Program {
    std::vector<std::unique_ptr<Expression>> expressions;
};

#endif