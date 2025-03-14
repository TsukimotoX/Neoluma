#ifndef NODES_H
#define NODES_H

#include <vector>
#include <memory>
#include "lexer.h"

enum NodeType {
    UnaryOp,
    BinaryOp,
    Function,
    Variable,
    IfCondition,
    Loop,
    TryCatch,
    ReturnStatement,
    Literal
};

struct ASTNode {
    NodeType type;
    std::string value; // For values like for example number, string, boolean, array, etc.
    std::vector<std::unique_ptr<ASTNode>> children; // Children nodes for doing operations with two nodes.

    ASTNode(NodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

struct LiteralNode : public ASTNode {
    int value;
    LiteralNode(int value) : ASTNode(NodeType::Literal), value(value) {}
};

struct VariableNode : public ASTNode {
    std::string name;
    VariableNode(const std::string& name) : ASTNode(NodeType::Variable), name(name) {}
};

struct BinaryOperationNode : public ASTNode {
    std::unique_ptr<ASTNode> left; // left node
    Token op; //operation like math or logic
    std::unique_ptr<ASTNode> right; // right node

    BinaryOperationNode(std::unique_ptr<ASTNode> left, Token op, std::unique_ptr<ASTNode> right) : ASTNode(NodeType::BinaryOp), left(std::move(left)), op(op), right(std::move(right)) {}
};

// Unary operation is an operation to do with one value (-a, !a)
struct UnaryOperationNode : public ASTNode {
    Token op; //operator
    std::unique_ptr<ASTNode> operand; //operand to use operator on

    UnaryOperationNode(Token op, std::unique_ptr<ASTNode> operand) : ASTNode(NodeType::UnaryOp), op(op), operand(std::move(operand)) {}
};

struct FunctionNode : public ASTNode {
    std::string name;
    std::vector<std::string> args;
    std::unique_ptr<ASTNode> body;
    FunctionNode(const std::string& name, std::vector<std::string> args, std::unique_ptr<ASTNode> body) : ASTNode(NodeType::Function), name(name), args(std::move(args)), body(std::move(body)) {}
};

struct IfConditionNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> ifBody;
    std::unique_ptr<ASTNode> elseBody;

    IfConditionNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> ifBody, std::unique_ptr<ASTNode> elseBody) : ASTNode(NodeType::IfCondition), condition(std::move(condition)), ifBody(std::move(ifBody)), elseBody(std::move(elseBody)) {}
};

struct LoopNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    LoopNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> body) : ASTNode(NodeType::Loop), condition(std::move(condition)), body(std::move(body)) {}
};

struct TryCatchNode : public ASTNode {
    std::unique_ptr<ASTNode> tryBody;
    std::unique_ptr<ASTNode> catchBody;

    TryCatchNode(std::unique_ptr<ASTNode> tryBody, std::unique_ptr<ASTNode> catchBody) : ASTNode(NodeType::TryCatch), tryBody(std::move(tryBody)), catchBody(std::move(catchBody)) {}
};

struct ReturnStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> value;

    ReturnStatementNode(std::unique_ptr<ASTNode> value) : ASTNode(NodeType::ReturnStatement), value(std::move(value)) {}
};

#endif 