#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "lexer.h"

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct LiteralNode : public ASTNode {
    int value;
    LiteralNode(int val) : value(val) {}
};

struct VariableNode : public ASTNode {
    std::string n; //name
    VariableNode(const std::string& name) : n(name) {}
};

struct BinaryOperationNode : public ASTNode {
    std::unique_ptr<ASTNode> l; // left
    std::string o; //operation like math or logic
    std::unique_ptr<ASTNode> r; // right

    BinaryOperationNode(std::unique_ptr<ASTNode> left, const std::string& op, std::unique_ptr<ASTNode> right) : l(std::move(left)), o(op), r(std::move(right)) {}
};

// Unary operation is an operation to do with one value (-a, !a)
struct UnaryOperationNode : public ASTNode {
    std::string op; //operator
    std::unique_ptr<ASTNode> oprnd; //operand

    UnaryOperationNode(const std::string& Operator, std::unique_ptr<ASTNode> operand) : op(Operator), oprnd(std::move(operand)) {}
};

struct FunctionCallNode : public ASTNode {
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    FunctionCallNode(const std::string& name) : funcName(name) {}
};

#endif 