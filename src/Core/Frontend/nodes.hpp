#pragma once

#include <iostream>
#include <vector>
#include <optional>
#include <array>
#include <variant>
#include "../../HelperFunctions.hpp"
//#include "llvm/IR/Value.h"

enum struct ASTNodeType {
    Literal, Variable, Assignment, BinaryOperation, UnaryOperation, CallExpression,
    Block, IfStatement, ForLoop, WhileLoop, TryCatch, ReturnStatement, 
    Function, Class, 
    Parameter, Modifier,
    Switch, Case, SCDefault,
    Module, Program, 
    Import, Decorator, Preprocessor, 
    BreakStatement, ContinueStatement, ThrowStatement,
    Array, Set, Dict, Void, Result, Enum, Interface, Lambda,
    EnumMember, InterfaceField
};

// enum struct ASTVariableType {
//     Integer, Float, Number, Boolean, String,
//     Array, Set, Dictionary, Void, Result,
//     Undefined
// };

enum struct ASTModifierType {
    Public, Private, Protected, Static, Const, Override, Async, Debug
};

enum struct ASTPreprocessorDirectiveType {
    Import, Unsafe, Baremetal, Float, Macro, None
};

enum struct ASTImportType {
    /*
    Native - from dependencies
    Relative - relative to path
    Foreign - imported from other language via langpacks.
    ForeignRelative - imported from other language via langpacks, but with relative path.
    */
    Native, Relative, Foreign, ForeignRelative,
};

struct ASTNode {
    ASTNodeType type;
    std::string value; // for basic values like literals, etc.

    virtual ~ASTNode() = default;
    virtual void toString() const;
    //virtual LLVM::Value generateCode(); // for the compiler to generate code from this AST node
};

// All nodes available in Neoluma

struct LiteralNode : ASTNode {
    LiteralNode(const std::string& val = "") {
        this->type = ASTNodeType::Literal;
        value = val;
    }
};

struct VariableNode : ASTNode {
    std::string varName;
    bool isNullable = false;
    std::string rawType;

    VariableNode(const std::string& varName, const std::string& type, bool isNullable = false) : varName(varName), isNullable(isNullable) {
        this->type = ASTNodeType::Variable;
        this->rawType = type;
    }
};

struct AssignmentNode : ASTNode {
    VariableNode* variable;
    MemoryPtr<ASTNode> variableValue;
    bool isInitialized = false;

    AssignmentNode(VariableNode* variable, const std::string& op, MemoryPtr<ASTNode> varValue)
        : variable(variable), variableValue(std::move(varValue)) {
        this->value = op;
        this->type = ASTNodeType::Assignment;
    }
};

struct BinaryOperationNode : ASTNode {
    MemoryPtr<ASTNode> leftOperand;
    MemoryPtr<ASTNode> rightOperand;

    BinaryOperationNode(MemoryPtr<ASTNode> leftOp, const std::string& op, MemoryPtr<ASTNode> rightOp)
        : leftOperand(std::move(leftOp)), rightOperand(std::move(rightOp)) {
        this->type = ASTNodeType::BinaryOperation;
        value = op;
    }
};

struct UnaryOperationNode : ASTNode {
    MemoryPtr<ASTNode> operand;

    UnaryOperationNode(const std::string& op, MemoryPtr<ASTNode> operand)
        : operand(std::move(operand)) {
        this->type = ASTNodeType::UnaryOperation;
        value = op;
    }
};

// statements
struct BlockNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> statements;
    BlockNode() { this->type = ASTNodeType::Block; }
};

struct IfNode : ASTNode {
    MemoryPtr<ASTNode> condition;
    MemoryPtr<ASTNode> thenBlock;
    MemoryPtr<ASTNode> elseBlock;

    IfNode(MemoryPtr<ASTNode> condition, MemoryPtr<ASTNode> thenBlock, MemoryPtr<ASTNode> elseBlock = nullptr)
        : condition(std::move(condition)), thenBlock(std::move(thenBlock)), elseBlock(std::move(elseBlock)) {
        this->type = ASTNodeType::IfStatement;
    }
};

struct SCDefaultNode : ASTNode {
    MemoryPtr<BlockNode> body;
    SCDefaultNode(MemoryPtr<BlockNode> body) : body(std::move(body)) {
        this->type = ASTNodeType::SCDefault;
    }
};

struct CaseNode : ASTNode {
    MemoryPtr<ASTNode> condition;
    MemoryPtr<BlockNode> body;
    CaseNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        this->type = ASTNodeType::Case;
    }
};

struct SwitchNode : ASTNode {
    MemoryPtr<ASTNode> expression;
    std::vector<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase;

    SwitchNode(MemoryPtr<ASTNode> expression, std::vector<MemoryPtr<CaseNode>> cases, MemoryPtr<SCDefaultNode> defaultCase = nullptr)
        : expression(std::move(expression)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
        this->type = ASTNodeType::Switch;
    }
};

struct ForLoopNode : ASTNode {
    MemoryPtr<VariableNode> variable;
    MemoryPtr<ASTNode> iterable;
    MemoryPtr<BlockNode> body;

    ForLoopNode(MemoryPtr<VariableNode> variable, MemoryPtr<ASTNode> iterable, MemoryPtr<BlockNode> body)
        : variable(std::move(variable)), iterable(std::move(iterable)), body(std::move(body)) {
        this->type = ASTNodeType::ForLoop;
    }
};

struct WhileLoopNode : ASTNode {
    MemoryPtr<ASTNode> condition;
    MemoryPtr<BlockNode> body;

    WhileLoopNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        this->type = ASTNodeType::WhileLoop;
    }
};

struct BreakStatementNode : ASTNode {
    BreakStatementNode() { this->type = ASTNodeType::BreakStatement; }
};

struct ContinueStatementNode : ASTNode {
    ContinueStatementNode() { this->type = ASTNodeType::ContinueStatement; }
};

struct ReturnStatementNode : ASTNode {
    MemoryPtr<ASTNode> expression;
    ReturnStatementNode(MemoryPtr<ASTNode> expression)
        : expression(std::move(expression)) {
        this->type = ASTNodeType::ReturnStatement;
    }
};

struct ThrowStatementNode : ASTNode {
    MemoryPtr<ASTNode> expression;
    ThrowStatementNode(MemoryPtr<ASTNode> expression)
        : expression(std::move(expression)) {
        this->type = ASTNodeType::ThrowStatement;
    }
};

struct TryCatchNode : ASTNode {
    MemoryPtr<BlockNode> tryBlock;
    MemoryPtr<BlockNode> catchBlock;
    MemoryPtr<LiteralNode> exception;

    TryCatchNode(MemoryPtr<BlockNode> tryBlock, MemoryPtr<BlockNode> catchBlock)
        : tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)) {
        this->type = ASTNodeType::TryCatch;
    }
};

// composite data
struct ArrayNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> elements;
    MemoryPtr<ASTNode> typeHint;

    ArrayNode(std::vector<MemoryPtr<ASTNode>> elements, MemoryPtr<ASTNode> typeHint=nullptr)
        : elements(std::move(elements)), typeHint(std::move(typeHint)) {
        this->type = ASTNodeType::Array;
    }
};

struct SetNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> elements;
    MemoryPtr<ASTNode> typeHint;

    SetNode(std::vector<MemoryPtr<ASTNode>> elements, MemoryPtr<ASTNode> typeHint=nullptr)
        : elements(std::move(elements)), typeHint(std::move(typeHint)) {
        this->type = ASTNodeType::Set;
    }
};

struct DictNode : ASTNode {
    std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> elements;
    std::array<std::string, 2> types;

    DictNode(std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> elements, std::array<std::string, 2> types)
        : elements(std::move(elements)), types(std::move(types)) {
        this->type = ASTNodeType::Dict;
    }
};

struct VoidNode : ASTNode {
    VoidNode() { this->type = ASTNodeType::Void; }
};

struct ResultNode : ASTNode {
    MemoryPtr<ASTNode> t;
    MemoryPtr<ASTNode> e;
    bool isError;

    ResultNode(MemoryPtr<ASTNode> t, MemoryPtr<ASTNode> e = nullptr, bool isError = false)
        : t(std::move(t)), e(std::move(e)), isError(isError) {
        this->type = ASTNodeType::Result;
    }
};

// higher structures
struct ParameterNode : ASTNode {
    std::string parameterName;
    std::string parameterRawType;
    std::optional<std::string> defaultValue;

    ParameterNode(const std::string& parameterName, const std::string& parameterRawType, const std::string& defaultValue = "")
        : parameterName(parameterName), parameterRawType(parameterRawType) {
        this->type = ASTNodeType::Parameter;
        if (!defaultValue.empty()) this->defaultValue = defaultValue;
    }
};

struct ModifierNode : ASTNode {
    ASTModifierType modifier;
    ModifierNode(ASTModifierType& modifier) : modifier(modifier) {
        this->type = ASTNodeType::Modifier;
    }
};

struct CallExpressionNode : ASTNode {
    MemoryPtr<ASTNode> callee;
    std::vector<MemoryPtr<ASTNode>> arguments;

    CallExpressionNode(MemoryPtr<ASTNode> callee, std::vector<MemoryPtr<ASTNode>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {
        this->type = ASTNodeType::CallExpression;
    }
};

struct EnumMemberNode : ASTNode {
    std::string name;
    MemoryPtr<LiteralNode> value;
    EnumMemberNode(const std::string& name, MemoryPtr<LiteralNode> value = nullptr) : name(name), value(std::move(value)) {
        this->type = ASTNodeType::EnumMember;
    }
};

struct EnumNode : ASTNode {
    std::vector<MemoryPtr<CallExpressionNode>> decorators;
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    std::vector<MemoryPtr<EnumMemberNode>> elements;

    EnumNode(std::vector<MemoryPtr<EnumMemberNode>> elements, std::vector<MemoryPtr<CallExpressionNode>> decorators = std::vector<MemoryPtr<CallExpressionNode>>{}, std::vector<MemoryPtr<ModifierNode>> modifiers = std::vector<MemoryPtr<ModifierNode>>{}) {
        this->type = ASTNodeType::Enum;
        this->elements = std::move(elements);
        this->decorators = std::move(decorators);
        this->modifiers = std::move(modifiers);
    }
};

struct InterfaceFieldNode : ASTNode {
    std::string name;
    std::string vartype;
    bool isNullable;
    InterfaceFieldNode(const std::string& name, const std::string& type, bool isNullable = false)
        : name(name), vartype(type), isNullable(isNullable) {
        this->type = ASTNodeType::InterfaceField;
    }
};

struct InterfaceNode : ASTNode {
    std::vector<MemoryPtr<CallExpressionNode>> decorators;
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    std::vector<MemoryPtr<InterfaceFieldNode>> elements;

    InterfaceNode(std::vector<MemoryPtr<InterfaceFieldNode>> elements, std::vector<MemoryPtr<CallExpressionNode>> decorators = std::vector<MemoryPtr<CallExpressionNode>>{}, std::vector<MemoryPtr<ModifierNode>> modifiers = std::vector<MemoryPtr<ModifierNode>>{}) {
        this->type = ASTNodeType::Interface;
        this->elements = std::move(elements);
        this->decorators = std::move(decorators);
        this->modifiers = std::move(modifiers);
    }
};

struct LambdaNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> params;
    MemoryPtr<ASTNode> body;

    LambdaNode(std::vector<MemoryPtr<ASTNode>> params, MemoryPtr<ASTNode> body)
        : params(std::move(params)), body(std::move(body)) {
        this->type = ASTNodeType::Lambda;
    }
};

struct FunctionNode : ASTNode {
    std::string name;
    std::vector<MemoryPtr<CallExpressionNode>> decorators;
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    std::vector<MemoryPtr<ParameterNode>> parameters;
    MemoryPtr<BlockNode> body;

    FunctionNode(const std::string& name, std::vector<MemoryPtr<ParameterNode>> parameters, MemoryPtr<BlockNode> body,
                 std::vector<MemoryPtr<CallExpressionNode>> decorators = std::vector<MemoryPtr<CallExpressionNode>>{}, std::vector<MemoryPtr<ModifierNode>> modifiers = std::vector<MemoryPtr<ModifierNode>>{})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        this->type = ASTNodeType::Function;
    }
};

struct ClassNode : ASTNode {
    std::string name;
    std::vector<MemoryPtr<CallExpressionNode>> decorators;
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    std::vector<MemoryPtr<VariableNode>> fields;
    std::vector<MemoryPtr<FunctionNode>> methods;

    ClassNode(const std::string& name, std::vector<MemoryPtr<VariableNode>> fields, std::vector<MemoryPtr<FunctionNode>> methods,
              std::vector<MemoryPtr<CallExpressionNode>> decorators = std::vector<MemoryPtr<CallExpressionNode>>{}, std::vector<MemoryPtr<ModifierNode>> modifiers = std::vector<MemoryPtr<ModifierNode>>{})
        : name(name), fields(std::move(fields)), methods(std::move(methods)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        this->type = ASTNodeType::Class;
    }
};

struct DecoratorNode : ASTNode {
    std::string name;
    std::vector<MemoryPtr<CallExpressionNode>> decorators;
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    std::vector<MemoryPtr<ParameterNode>> parameters;
    MemoryPtr<BlockNode> body;

    DecoratorNode(const std::string& name, std::vector<MemoryPtr<ParameterNode>> parameters, MemoryPtr<BlockNode> body,
                 std::vector<MemoryPtr<CallExpressionNode>> decorators = std::vector<MemoryPtr<CallExpressionNode>>{}, std::vector<MemoryPtr<ModifierNode>> modifiers = std::vector<MemoryPtr<ModifierNode>>{})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        this->type = ASTNodeType::Decorator;
    }
};

// imports and program structure
struct ImportNode : ASTNode {
    std::string moduleName;
    std::string alias;
    ASTImportType importType;

    ImportNode(const std::string& moduleName, const std::string& alias, ASTImportType importType)
        : moduleName(moduleName), alias(alias), importType(importType) {
        this->type = ASTNodeType::Import;
    }
};

struct PreprocessorDirectiveNode : ASTNode {
    ASTPreprocessorDirectiveType directive;
    PreprocessorDirectiveNode(ASTPreprocessorDirectiveType& directive, const std::string& value = "")
        : directive(directive) {
        this->type = ASTNodeType::Preprocessor;
        this->value = value;
    }
};

struct ModuleNode : ASTNode {
    std::string moduleName;
    std::vector<MemoryPtr<ASTNode>> body;
    ModuleNode(const std::string& name) : moduleName(name) {
        this->type = ASTNodeType::Module;
    }
};

struct ProgramNode : ASTNode {
    std::vector<ModuleNode> body;
    ProgramNode() { this->type = ASTNodeType::Program; }
};