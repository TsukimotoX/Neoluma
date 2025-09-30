#pragma once

#include <iostream>
#include <vector>
#include <optional>
#include <array>
#include <variant>
#include "../../../HelperFunctions.hpp"
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
    Array, Set, Dict, Void, Result, Enum, Interface, Lambda
};
/* ASTNodes checklist!!!
    * - LiteralNode ✅
    * - VariableNode ✅
    * - AssignmentNode ✅
    * - BinaryOperationNode ✅
    * - UnaryOperationNode ✅
    * - CallExpressionNode ✅
    * - BlockNode ✅
    * - IfNode ✅
    * - ForLoopNode ✅
    * - WhileLoopNode ✅
    * - TryCatchNode ✅
    * - ReturnStatementNode ✅
    * - FunctionNode ✅
    * - structNode ✅
    * - DecoratorNode ✅
    * - ParameterNode ✅
    * - ModifierNode ✅
    * - SwitchNode ✅
    * - CaseNode ✅
    * - SCDefaultNode ✅
    * - ImportNode ✅
    * - PreprocessorDirectiveNode ✅
    * - ModuleNode ✅
    * - ProgramNode ✅
    * - ArrayNode ✅
    * - SetNode ✅
    * - DictNode ✅
    * - VoidNode ✅
    * - ResultNode ✅
    * - EnumNode ✅
    * - InterfaceNode ✅
    * - LambdaNode ✅
*/

enum struct ASTVariableType {
    Integer, Float, Number, Boolean, String,
    Array, Set, Dictionary, Void, Result,
    Undefined
};

enum struct ASTModifierType {
    Public, Private, Protected, Static, Const, Override, Async
};

enum struct ASTPreprocessorDirectiveType {
    Import, Unsafe, Baremetal, Float, Macro
};

enum struct ASTImportType {
    /*
    Native - from dependencies
    Relative - relative to path
    Foreign - imported from other language via langpacks.
    */
    Native, Relative, Foreign
};

struct ASTNode {
    ASTNodeType type;
    std::string value; // for basic values like literals, etc.

    virtual ~ASTNode() = default;
    virtual void toString() const;
    //virtual LLVM::Value generateCode(); // for the compiler to generate code from this AST node
};

// All node types inherited from ASTNode, for the parser and compiler to determine.

struct ArrayNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> elements;
    MemoryPtr<ASTNode> typeHint;

    ArrayNode(std::vector<MemoryPtr<ASTNode>> elements, MemoryPtr<ASTNode> typeHint) {
        this->type = ASTNodeType::Array;
        this->elements = std::move(elements);
        this->typeHint = std::move(typeHint);
    }
};

struct SetNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> elements;
    MemoryPtr<ASTNode> typeHint;

    SetNode(std::vector<MemoryPtr<ASTNode>> elements, MemoryPtr<ASTNode> typeHint) {
        this->type = ASTNodeType::Set;
        this->elements = std::move(elements);
        this->typeHint = std::move(typeHint);
    }
};

struct DictNode : ASTNode {
    std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> elements;
    std::array<ASTVariableType, 2> types;

    DictNode(const std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>>& elements, const std::array<ASTVariableType, 2>& types) {
        this->type = ASTNodeType::Dict;
        this->elements = elements;
        this->types = types;
    }
};

struct VoidNode : ASTNode {
    VoidNode() { this->type = ASTNodeType::Void; }
};

struct ResultNode : ASTNode {
    MemoryPtr<ASTNode> t;
    MemoryPtr<ASTNode> e = nullptr;
    bool isError;

    ResultNode(MemoryPtr<ASTNode> t, MemoryPtr<ASTNode> e = nullptr, bool isError = false) 
    : t(std::move(t)), e(std::move(e)) {
        this->type = ASTNodeType::Result;
    }
};

struct EnumNode : ASTNode {
    // im not sure if this is how you do all of this shit
    std::vector<MemoryPtr<VariableNode>> elements;

    EnumNode(std::vector<MemoryPtr<VariableNode>> elements) {
        this->type = ASTNodeType::Enum;
        this->elements = std::move(elements);
    }
};

// just an enum but for classes lol
struct InterfaceNode : ASTNode {
    std::vector<VariableNode> elements;

    InterfaceNode(const std::vector<VariableNode>& elements) {
        this->type = ASTNodeType::Enum;
        this->elements = elements;
    }
};

struct LambdaNode : ASTNode {
    std::vector<MemoryPtr<ASTNode>> params;
    MemoryPtr<ASTNode> body;

    LambdaNode(const std::vector<MemoryPtr<ASTNode>>& params, MemoryPtr<ASTNode>& body) {
        this->type = ASTNodeType::Lambda;
        this->params = params;
        this->body = std::move(body);
    }
};

struct LiteralNode : ASTNode {
    ASTVariableType varType; // type of the variable, e.g. Integer, Float, etc.
    LiteralNode(const ASTVariableType& varType = ASTVariableType::Undefined, const std::string& val) {
        type = ASTNodeType::Literal;
        this->varType = varType;
        value = val;
    }
};

struct VariableNode : ASTNode {
public:
    std::string varName; // name of the variable
    bool isNullable = false; // can the variable be nothing or not?

    VariableNode(const std::string& varName, bool isNullable = false): varName(varName), isNullable(isNullable) {
        type = ASTNodeType::Variable;
    }
};

struct AssignmentNode : ASTNode {
    VariableNode* variable; // the variable being assigned to
    MemoryPtr<ASTNode> variableValue; // the value being assigned
    bool isInitialized = false; // does this assignment create a new variable or not?

    AssignmentNode(VariableNode* variable, const std::string& op, MemoryPtr<ASTNode> varValue): variable(variable), variableValue(std::move(varValue)) {
        this->value = op;
        type = ASTNodeType::Assignment;
    }
};

struct BinaryOperationNode : ASTNode {
    MemoryPtr<ASTNode> leftOperand;
    MemoryPtr<ASTNode> rightOperand;

    BinaryOperationNode(MemoryPtr<ASTNode>& leftOp, const std::string& op, MemoryPtr<ASTNode>& rightOp)
        : leftOperand(std::move(leftOp)), rightOperand(std::move(rightOp)) {
        type = ASTNodeType::BinaryOperation;
        value = op;
    }
};

struct UnaryOperationNode : ASTNode {
    MemoryPtr<ASTNode> operand; // because i'm terrible at english: it's the value after the operator, e.g. -x, !x, etc.

    UnaryOperationNode(const std::string& op, MemoryPtr<ASTNode>& operand)
        : operand(std::move(operand)) {
        type = ASTNodeType::UnaryOperation;
        value = op;
    }
};

struct BlockNode : ASTNode {
public:
    std::vector<MemoryPtr<ASTNode>> statements;

    BlockNode() {
        type = ASTNodeType::Block;
    }
};

struct IfNode : ASTNode {
public:
    MemoryPtr<ASTNode> condition; // the condition of the if statement
    MemoryPtr<BlockNode> thenBlock; // the block of code to execute if the condition is true
    MemoryPtr<BlockNode> elseBlock; // the block of code to execute if the condition is false

    IfNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> thenBlock, MemoryPtr<BlockNode> elseBlock = nullptr)
        : condition(std::move(condition)), thenBlock(std::move(thenBlock)), elseBlock(std::move(elseBlock)) {
        type = ASTNodeType::IfStatement;
    }
};

struct SCDefaultNode : ASTNode {
public:
    MemoryPtr<BlockNode> body; // the body of the default case

    SCDefaultNode(MemoryPtr<BlockNode> body)
        : body(std::move(body)) {
        type = ASTNodeType::SCDefault;
    }
};

struct CaseNode : ASTNode {
public:
    MemoryPtr<ASTNode> condition; // the condition of the case
    MemoryPtr<BlockNode> body; // the body of the case

    CaseNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        type = ASTNodeType::Case;
    }
};

struct SwitchNode : ASTNode {
public:
    MemoryPtr<ASTNode> expression; // the expression to switch on
    std::vector<MemoryPtr<CaseNode>> cases; // the cases of the switch statement
    MemoryPtr<SCDefaultNode> defaultCase; // the default case of the switch statement

    SwitchNode(MemoryPtr<ASTNode> expression, std::vector<MemoryPtr<CaseNode>> cases, MemoryPtr<SCDefaultNode> defaultCase = nullptr)
        : expression(std::move(expression)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
        type = ASTNodeType::Switch;
    }
};

struct ForLoopNode : ASTNode {
public:
    MemoryPtr<VariableNode> variable; // the loop variable
    MemoryPtr<ASTNode> iterable; // the iterable to loop over
    MemoryPtr<BlockNode> body; // the body of the loop

    ForLoopNode(MemoryPtr<VariableNode> variable, MemoryPtr<ASTNode> iterable, MemoryPtr<BlockNode> body)
        : variable(std::move(variable)), iterable(std::move(iterable)), body(std::move(body)) {
        type = ASTNodeType::ForLoop;
    }
};

struct WhileLoopNode : ASTNode {
public:
    MemoryPtr<ASTNode> condition; // the condition of the while loop
    MemoryPtr<BlockNode> body; // the body of the loop

    WhileLoopNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        type = ASTNodeType::WhileLoop;
    }
};

struct ParameterNode : ASTNode {
public:
    std::string parameterName; // the name of the parameter
    ASTVariableType parameterType; // the type of the parameter
    std::optional<std::string> defaultValue; // the default value of the parameter (optional)

    ParameterNode(const std::string& parameterName, ASTVariableType& parameterType, const std::string& defaultValue = "")
        : parameterName(parameterName), parameterType(parameterType) {
        this->type = ASTNodeType::Parameter;
        if (!defaultValue.empty()) this->defaultValue = defaultValue;
    }
};

struct ModifierNode : ASTNode {
public:
    ASTModifierType modifier;

    ModifierNode(ASTModifierType& modifier) : modifier(modifier) {
        this->type = ASTNodeType::Modifier;
    }
};

struct DecoratorNode : ASTNode {
public:
    std::string name; // the name of the decorator
    std::vector<MemoryPtr<DecoratorNode>> decorators; // the decorators applied to the decorator
    std::vector<MemoryPtr<ModifierNode>> modifiers; // the modifiers applied to the decorator (e.g. async, static, etc.)
    std::vector<MemoryPtr<ParameterNode>> parameters; // the parameters of the decorator
    MemoryPtr<BlockNode> body; // the body of the decorator

    DecoratorNode(const std::string& name, std::vector<MemoryPtr<ParameterNode>> parameters, MemoryPtr<BlockNode> body,
                 std::vector<MemoryPtr<DecoratorNode>> decorators = {}, std::vector<MemoryPtr<ModifierNode>> modifiers = {})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Decorator;
    }
};

struct FunctionNode : ASTNode {
public:
    std::string name; // the name of the function
    std::vector<MemoryPtr<DecoratorNode>> decorators; // the decorators applied to the function
    std::vector<MemoryPtr<ModifierNode>> modifiers; // the modifiers applied to the function (e.g. async, static, etc.)
    std::vector<MemoryPtr<ParameterNode>> parameters; // the parameters of the function
    MemoryPtr<BlockNode> body; // the body of the function

    FunctionNode(const std::string& name, std::vector<MemoryPtr<ParameterNode>> parameters, MemoryPtr<BlockNode> body,
                 std::vector<MemoryPtr<DecoratorNode>> decorators = {}, std::vector<MemoryPtr<ModifierNode>> modifiers = {})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Function;
    }
};

struct ClassNode : ASTNode {
public:
    std::string name; // the name of the class
    std::vector<MemoryPtr<DecoratorNode>> decorators; // the decorators applied to the class
    std::vector<MemoryPtr<ModifierNode>> modifiers; // the modifiers applied to the class (e.g. public, private, etc.)
    std::vector<MemoryPtr<VariableNode>> fields; // the fields of the class
    std::vector<MemoryPtr<FunctionNode>> methods; // the methods of the class

    ClassNode(const std::string& name, std::vector<MemoryPtr<VariableNode>> fields, std::vector<MemoryPtr<FunctionNode>> methods,
              std::vector<MemoryPtr<DecoratorNode>> decorators = {}, std::vector<MemoryPtr<ModifierNode>> modifiers = {})
        : name(name), fields(std::move(fields)), methods(std::move(methods)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Class;
    }
};

struct ReturnStatementNode : ASTNode {
public:
    MemoryPtr<ASTNode> expression; // the expression to return

    ReturnStatementNode(MemoryPtr<ASTNode>& expression)
        : expression(std::move(expression)) {
        type = ASTNodeType::ReturnStatement;
    }
};

struct CallExpressionNode : ASTNode {
public:
    MemoryPtr<ASTNode> callee; // the function or method being called
    std::vector<MemoryPtr<ParameterNode>> arguments; // the arguments passed to the function or method

    CallExpressionNode(MemoryPtr<ASTNode>& callee, std::vector<MemoryPtr<ParameterNode>>& arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {
        type = ASTNodeType::CallExpression;
    }
};

struct ThrowStatementNode : ASTNode {
public:
    MemoryPtr<ASTNode> expression; // the expression to throw

    ThrowStatementNode(MemoryPtr<ASTNode>& expression)
        : expression(std::move(expression)) {
        type = ASTNodeType::ThrowStatement;
    }
};

struct BreakStatementNode : ASTNode {
public:
    BreakStatementNode() {
        type = ASTNodeType::BreakStatement;
    }
};

struct ContinueStatementNode : ASTNode {
public:
    ContinueStatementNode() {
        type = ASTNodeType::ContinueStatement;
    }
};

struct TryCatchNode : ASTNode {
public:
    MemoryPtr<BlockNode> tryBlock; // the block of code to try
    MemoryPtr<BlockNode> catchBlock; // the block of code to execute if an exception is thrown
    MemoryPtr<LiteralNode> exception; // the variable to store the exception in (optional)

    TryCatchNode(MemoryPtr<BlockNode>& tryBlock, MemoryPtr<BlockNode>& catchBlock)
        : tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)) {
        type = ASTNodeType::TryCatch;
    }
};

// These nodes are unfinished for now, don't take them as final implementations.
struct ImportNode : ASTNode {
public:
    std::string moduleName; // the name of the module being imported
    std::vector<std::string> importNames; // the names of the items being imported from the module
    ASTImportType importType;

    ImportNode(const std::string& moduleName, const std::vector<std::string>& importNames = {}, ASTImportType importType)
        : moduleName(moduleName), importNames(importNames), importType(importType) {
        type = ASTNodeType::Import;
    }
};

struct PreprocessorDirectiveNode : ASTNode {
public:
    ASTPreprocessorDirectiveType directive; // the preprocessor directive (e.g. #include, #define, etc.)

    PreprocessorDirectiveNode(ASTPreprocessorDirectiveType& directive, const std::string& value = "")
        : directive(directive) {
        type = ASTNodeType::Preprocessor;
    }
};

// core nodes
struct ModuleNode : ASTNode {
public:
    std::string moduleName;
    std::vector<MemoryPtr<ASTNode>> body;

    ModuleNode(const std::string& name) : moduleName(name) {
        type = ASTNodeType::Module;
    }
};

struct ProgramNode : ASTNode {
public:
    std::vector<ModuleNode> body;

    ProgramNode() {
        type = ASTNodeType::Program;
    }
};