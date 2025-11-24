#pragma once
#include <iostream>
#include <vector>
#include <optional>
#include <array>
#include <string>
#include <format>
#include "../../HelperFunctions.hpp"
//#include "llvm/IR/Value.h"

enum struct ASTNodeType {
    Literal, Variable, MemberAccess, Assignment, BinaryOperation, UnaryOperation, CallExpression,
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

    virtual ~ASTNode();
    virtual std::string toString(int indent = 0) const;
    //virtual llvm::LLVMContext generateCode(); // for the compiler to generate code from this AST node
};

// All nodes available in Neoluma
struct LiteralNode : ASTNode {
    LiteralNode(const std::string& val = "") {
        this->type = ASTNodeType::Literal;
        value = val;
    }

    std::string toString(int indent = 0) const override;
};

struct VariableNode : ASTNode {
    std::string varName;
    bool isNullable = false;
    std::string rawType;

    VariableNode(const std::string& varName, const std::string& type, bool isNullable = false) : varName(varName), isNullable(isNullable) {
        this->type = ASTNodeType::Variable;
        this->rawType = type;
    }

    std::string toString(int indent = 0) const override;
};

struct MemberAccessNode : ASTNode {
    MemoryPtr<ASTNode> parent;
    MemoryPtr<ASTNode> val;

    MemberAccessNode(MemoryPtr<ASTNode> parent, MemoryPtr<ASTNode> val)
        : parent(std::move(parent)), val(std::move(val)) {
        this->type = ASTNodeType::MemberAccess;
    }

    std::string toString(int indent = 0) const override;
};

struct AssignmentNode : ASTNode {
    MemoryPtr<VariableNode> variable;
    MemoryPtr<ASTNode> variableValue;
    bool isInitialized = false;
    AssignmentNode(MemoryPtr<VariableNode> variable, const std::string& op, MemoryPtr<ASTNode> varValue)
        : variable(std::move(variable)), variableValue(std::move(varValue)) {
        this->value = op;
        this->type = ASTNodeType::Assignment;
    }

    std::string toString(int indent = 0) const override;
};

struct BinaryOperationNode : ASTNode {
    MemoryPtr<ASTNode> leftOperand;
    MemoryPtr<ASTNode> rightOperand;

    BinaryOperationNode(MemoryPtr<ASTNode> leftOp, const std::string& op, MemoryPtr<ASTNode> rightOp)
        : leftOperand(std::move(leftOp)), rightOperand(std::move(rightOp)) {
        this->type = ASTNodeType::BinaryOperation;
        value = op;
    }

    std::string toString(int indent = 0) const override;
};

struct UnaryOperationNode : ASTNode {
    MemoryPtr<ASTNode> operand;

    UnaryOperationNode(const std::string& op, MemoryPtr<ASTNode> operand)
        : operand(std::move(operand)) {
        this->type = ASTNodeType::UnaryOperation;
        value = op;
    }

    std::string toString(int indent = 0) const override;
};

// statements
struct BlockNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> statements;
    BlockNode() { this->type = ASTNodeType::Block; }

    std::string toString(int indent = 0) const override;
};

struct IfNode : ASTNode {
    MemoryPtr<ASTNode> condition;
    MemoryPtr<ASTNode> thenBlock;
    MemoryPtr<ASTNode> elseBlock;

    IfNode(MemoryPtr<ASTNode> condition, MemoryPtr<ASTNode> thenBlock, MemoryPtr<ASTNode> elseBlock = nullptr)
        : condition(std::move(condition)), thenBlock(std::move(thenBlock)), elseBlock(std::move(elseBlock)) {
        this->type = ASTNodeType::IfStatement;
    }

    std::string toString(int indent = 0) const override;
};

struct SCDefaultNode : ASTNode {
    MemoryPtr<ASTNode> body;
    SCDefaultNode(MemoryPtr<ASTNode> body) : body(std::move(body)) {
        this->type = ASTNodeType::SCDefault;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct CaseNode : ASTNode {
    MemoryPtr<ASTNode> condition;
    MemoryPtr<ASTNode> body;
    CaseNode(MemoryPtr<ASTNode> condition, MemoryPtr<ASTNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        this->type = ASTNodeType::Case;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct SwitchNode : ASTNode {
    MemoryPtr<ASTNode> expression;
    std::vector<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase;

    SwitchNode(MemoryPtr<ASTNode> expression, std::vector<MemoryPtr<CaseNode>> cases, MemoryPtr<SCDefaultNode> defaultCase = nullptr)
        : expression(std::move(expression)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
        this->type = ASTNodeType::Switch;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct ForLoopNode : ASTNode {
    MemoryPtr<VariableNode> variable;
    MemoryPtr<ASTNode> iterable;
    MemoryPtr<BlockNode> body;

    ForLoopNode(MemoryPtr<VariableNode> variable, MemoryPtr<ASTNode> iterable, MemoryPtr<BlockNode> body)
        : variable(std::move(variable)), iterable(std::move(iterable)), body(std::move(body)) {
        this->type = ASTNodeType::ForLoop;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct WhileLoopNode : ASTNode {
    MemoryPtr<ASTNode> condition;
    MemoryPtr<BlockNode> body;

    WhileLoopNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        this->type = ASTNodeType::WhileLoop;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct BreakStatementNode : ASTNode {
    BreakStatementNode() { this->type = ASTNodeType::BreakStatement; }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct ContinueStatementNode : ASTNode {
    ContinueStatementNode() { this->type = ASTNodeType::ContinueStatement; }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct ReturnStatementNode : ASTNode {
    MemoryPtr<ASTNode> expression;
    ReturnStatementNode(MemoryPtr<ASTNode> expression)
        : expression(std::move(expression)) {
        this->type = ASTNodeType::ReturnStatement;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct ThrowStatementNode : ASTNode {
    MemoryPtr<ASTNode> expression;
    ThrowStatementNode(MemoryPtr<ASTNode> expression)
        : expression(std::move(expression)) {
        this->type = ASTNodeType::ThrowStatement;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct TryCatchNode : ASTNode {
    MemoryPtr<BlockNode> tryBlock;
    MemoryPtr<BlockNode> catchBlock;
    MemoryPtr<LiteralNode> exception;

    TryCatchNode(MemoryPtr<BlockNode> tryBlock, MemoryPtr<BlockNode> catchBlock)
        : tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)) {
        this->type = ASTNodeType::TryCatch;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

// composite data
struct ArrayNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> elements;
    MemoryPtr<ASTNode> typeHint;

    ArrayNode(std::vector<MemoryPtr<ASTNode>> elements, MemoryPtr<ASTNode> typeHint=nullptr)
        : elements(std::move(elements)), typeHint(std::move(typeHint)) {
        this->type = ASTNodeType::Array;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct SetNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> elements;
    MemoryPtr<ASTNode> typeHint;

    SetNode(std::vector<MemoryPtr<ASTNode>> elements, MemoryPtr<ASTNode> typeHint=nullptr)
        : elements(std::move(elements)), typeHint(std::move(typeHint)) {
        this->type = ASTNodeType::Set;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

struct DictNode : ASTNode {
    std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> elements;
    std::array<std::string, 2> types;

    DictNode(std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> elements, std::array<std::string, 2> types)
        : elements(std::move(elements)), types(std::move(types)) {
        this->type = ASTNodeType::Dict;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent) const override;
};

// Hold on, has this ever been used anywhere?
struct VoidNode : ASTNode {
    VoidNode() { this->type = ASTNodeType::Void; }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct ResultNode : ASTNode {
    MemoryPtr<ASTNode> t;
    MemoryPtr<ASTNode> e;
    bool isError;

    ResultNode(MemoryPtr<ASTNode> t, MemoryPtr<ASTNode> e = nullptr, bool isError = false)
        : t(std::move(t)), e(std::move(e)), isError(isError) {
        this->type = ASTNodeType::Result;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct ModifierNode : ASTNode {
    ASTModifierType modifier;
    ModifierNode(ASTModifierType& modifier) : modifier(modifier) {
        this->type = ASTNodeType::Modifier;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct CallExpressionNode : ASTNode {
    MemoryPtr<ASTNode> callee;
    std::vector<MemoryPtr<ASTNode>> arguments;

    CallExpressionNode(MemoryPtr<ASTNode> callee, std::vector<MemoryPtr<ASTNode>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {
        this->type = ASTNodeType::CallExpression;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct EnumMemberNode : ASTNode {
    std::string name;
    MemoryPtr<LiteralNode> value;
    EnumMemberNode(const std::string& name, MemoryPtr<LiteralNode> value = nullptr) : name(name), value(std::move(value)) {
        this->type = ASTNodeType::EnumMember;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct InterfaceFieldNode : ASTNode {
    std::string name;
    std::string vartype;
    bool isNullable;
    InterfaceFieldNode(const std::string& name, const std::string& type, bool isNullable = false)
        : name(name), vartype(type), isNullable(isNullable) {
        this->type = ASTNodeType::InterfaceField;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct LambdaNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> params;
    MemoryPtr<ASTNode> body;

    LambdaNode(std::vector<MemoryPtr<ASTNode>> params, MemoryPtr<ASTNode> body)
        : params(std::move(params)), body(std::move(body)) {
        this->type = ASTNodeType::Lambda;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
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

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct PreprocessorDirectiveNode : ASTNode {
    ASTPreprocessorDirectiveType directive;
    PreprocessorDirectiveNode(ASTPreprocessorDirectiveType& directive, const std::string& value = "")
        : directive(directive) {
        this->type = ASTNodeType::Preprocessor;
        this->value = value;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct ModuleNode : ASTNode {
    std::string moduleName;
    std::vector<MemoryPtr<ASTNode>> body;
    ModuleNode(const std::string& name) : moduleName(name) {
        this->type = ASTNodeType::Module;
    }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

struct ProgramNode : ASTNode {
    std::vector<ModuleNode> body;
    ProgramNode() { this->type = ASTNodeType::Program; }

    // Suggested by AI. If it fails, it's his fault
    std::string toString(int indent = 0) const override;
};

