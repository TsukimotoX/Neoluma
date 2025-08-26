#pragma once

#include <iostream>
#include <vector>
#include <optional>

enum class ASTNodeType {
    Literal, Variable, Assignment, BinaryOperation, UnaryOperation, CallExpression,
    Block, IfStatement, ForLoop, WhileLoop, TryCatch, ReturnStatement, 
    Function, Class, 
    Parameter, Modifier,
    Switch, Case, SCDefault,
    Module, Program, 
    Import, Decorator, Preprocessor, 
    BreakStatement, ContinueStatement, ThrowStatement,
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
    * - ClassNode ✅
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
    * / TODO: Add condition node, with expression that must be true. like a binary operation kind of thing but for ==
*/

enum class ASTVariableType {
    Integer, Float, Number, Boolean, String,
    Array, Set, Dictionary, Void, Result,
    Undefined
};

enum class ASTModifierType {
    Public, Private, Protected, Static, Const, Override, Async
};

enum class ASTPreprocessorDirectiveType {
    Import, Unsafe, Baremetal, Float, Use, Macro
};

enum class ASTImportType {
    /*
    Native - from dependencies
    Relative - relative to path
    Foreign - imported from other language via langpacks.
    */
    Native, Relative, Foreign
};

class ASTNode {
public:
    ASTNodeType type;
    std::string value; // for basic values like literals, etc.

    virtual ~ASTNode() = default;
    virtual void toString() const;
    virtual void generateCode(); // for the compiler to generate code from this AST node
};

 // All node types inherited from ASTNode, for the parser and compiler to determine.

class LiteralNode : public ASTNode {
public:
    LiteralNode(const std::string& val) {
        type = ASTNodeType::Literal;
        value = val;
    }
};

class VariableNode : public ASTNode {
public:
    std::string varName; // name of the variable
    ASTVariableType varType; // type of the variable, e.g. Integer, Float, etc.

    VariableNode(const std::string& varName, ASTVariableType& varType): varName(varName), varType(varType) {
        type = ASTNodeType::Variable;
    }
};

class AssignmentNode : public ASTNode {
public:
    VariableNode* variable; // the variable being assigned to
    std::unique_ptr<ASTNode> variableValue; // the value being assigned

    AssignmentNode(VariableNode* variable, const std::string& op, std::unique_ptr<ASTNode>& varValue): variable(variable), variableValue(std::move(varValue)) {
        this->value = op;
        type = ASTNodeType::Assignment;
    }
};

class BinaryOperationNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> leftOperand;
    std::unique_ptr<ASTNode> rightOperand;

    BinaryOperationNode(std::unique_ptr<ASTNode>& leftOp, const std::string& op, std::unique_ptr<ASTNode>& rightOp)
        : leftOperand(std::move(leftOp)), rightOperand(std::move(rightOp)) {
        type = ASTNodeType::BinaryOperation;
        value = op;
    }
};

class UnaryOperationNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> operand; // because i'm terrible at english: it's the value after the operator, e.g. -x, !x, etc.

    UnaryOperationNode(const std::string& op, std::unique_ptr<ASTNode>& operand)
        : operand(std::move(operand)) {
        type = ASTNodeType::UnaryOperation;
        value = op;
    }
};

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;

    BlockNode() {
        type = ASTNodeType::Block;
    }
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition; // the condition of the if statement
    std::unique_ptr<BlockNode> thenBlock; // the block of code to execute if the condition is true
    std::unique_ptr<BlockNode> elseBlock; // the block of code to execute if the condition is false

    IfNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<BlockNode> thenBlock, std::unique_ptr<BlockNode> elseBlock = nullptr)
        : condition(std::move(condition)), thenBlock(std::move(thenBlock)), elseBlock(std::move(elseBlock)) {
        type = ASTNodeType::IfStatement;
    }
};

class SCDefaultNode : public ASTNode {
public:
    std::unique_ptr<BlockNode> body; // the body of the default case

    SCDefaultNode(std::unique_ptr<BlockNode> body)
        : body(std::move(body)) {
        type = ASTNodeType::SCDefault;
    }
};

class CaseNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition; // the condition of the case
    std::unique_ptr<BlockNode> body; // the body of the case

    CaseNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        type = ASTNodeType::Case;
    }
};

class SwitchNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression; // the expression to switch on
    std::vector<std::unique_ptr<CaseNode>> cases; // the cases of the switch statement
    std::unique_ptr<SCDefaultNode> defaultCase; // the default case of the switch statement

    SwitchNode(std::unique_ptr<ASTNode> expression, std::vector<std::unique_ptr<CaseNode>> cases, std::unique_ptr<SCDefaultNode> defaultCase = nullptr)
        : expression(std::move(expression)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
        type = ASTNodeType::Switch;
    }
};

class ForLoopNode : public ASTNode {
public:
    std::unique_ptr<VariableNode> variable; // the loop variable
    std::unique_ptr<ASTNode> iterable; // the iterable to loop over
    std::unique_ptr<BlockNode> body; // the body of the loop

    ForLoopNode(std::unique_ptr<VariableNode> variable, std::unique_ptr<ASTNode> iterable, std::unique_ptr<BlockNode> body)
        : variable(std::move(variable)), iterable(std::move(iterable)), body(std::move(body)) {
        type = ASTNodeType::ForLoop;
    }
};

class WhileLoopNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition; // the condition of the while loop
    std::unique_ptr<BlockNode> body; // the body of the loop

    WhileLoopNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        type = ASTNodeType::WhileLoop;
    }
};

class ParameterNode : public ASTNode {
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

class ModifierNode : public ASTNode {
public:
    ASTModifierType modifier;

    ModifierNode(ASTModifierType& modifier) : modifier(modifier) {
        this->type = ASTNodeType::Modifier;
    }
};

class DecoratorNode : public ASTNode {
public:
    std::string name; // the name of the decorator
    std::vector<std::unique_ptr<DecoratorNode>> decorators; // the decorators applied to the decorator
    std::vector<std::unique_ptr<ModifierNode>> modifiers; // the modifiers applied to the decorator (e.g. async, static, etc.)
    std::vector<std::unique_ptr<ParameterNode>> parameters; // the parameters of the decorator
    std::unique_ptr<BlockNode> body; // the body of the decorator

    DecoratorNode(const std::string& name, std::vector<std::unique_ptr<ParameterNode>> parameters, std::unique_ptr<BlockNode> body,
                 std::vector<std::unique_ptr<DecoratorNode>> decorators = {}, std::vector<std::unique_ptr<ModifierNode>> modifiers = {})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Decorator;
    }
};

class FunctionNode : public ASTNode {
public:
    std::string name; // the name of the function
    std::vector<std::unique_ptr<DecoratorNode>> decorators; // the decorators applied to the function
    std::vector<std::unique_ptr<ModifierNode>> modifiers; // the modifiers applied to the function (e.g. async, static, etc.)
    std::vector<std::unique_ptr<ParameterNode>> parameters; // the parameters of the function
    std::unique_ptr<BlockNode> body; // the body of the function

    FunctionNode(const std::string& name, std::vector<std::unique_ptr<ParameterNode>> parameters, std::unique_ptr<BlockNode> body,
                 std::vector<std::unique_ptr<DecoratorNode>> decorators = {}, std::vector<std::unique_ptr<ModifierNode>> modifiers = {})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Function;
    }
};

class ClassNode : public ASTNode {
public:
    std::string name; // the name of the class
    std::vector<std::unique_ptr<DecoratorNode>> decorators; // the decorators applied to the class
    std::vector<std::unique_ptr<ModifierNode>> modifiers; // the modifiers applied to the class (e.g. public, private, etc.)
    std::vector<std::unique_ptr<VariableNode>> fields; // the fields of the class
    std::vector<std::unique_ptr<FunctionNode>> methods; // the methods of the class

    ClassNode(const std::string& name, std::vector<std::unique_ptr<VariableNode>> fields, std::vector<std::unique_ptr<FunctionNode>> methods,
              std::vector<std::unique_ptr<DecoratorNode>> decorators = {}, std::vector<std::unique_ptr<ModifierNode>> modifiers = {})
        : name(name), fields(std::move(fields)), methods(std::move(methods)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Class;
    }
};

class ReturnStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression; // the expression to return

    ReturnStatementNode(std::unique_ptr<ASTNode>& expression)
        : expression(std::move(expression)) {
        type = ASTNodeType::ReturnStatement;
    }
};

class CallExpressionNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> callee; // the function or method being called
    std::vector<std::unique_ptr<ParameterNode>> arguments; // the arguments passed to the function or method

    CallExpressionNode(std::unique_ptr<ASTNode>& callee, std::vector<std::unique_ptr<ParameterNode>>& arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {
        type = ASTNodeType::CallExpression;
    }
};

class ThrowStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression; // the expression to throw

    ThrowStatementNode(std::unique_ptr<ASTNode>& expression)
        : expression(std::move(expression)) {
        type = ASTNodeType::ThrowStatement;
    }
};

class BreakStatementNode : public ASTNode {
public:
    BreakStatementNode() {
        type = ASTNodeType::BreakStatement;
    }
};

class ContinueStatementNode : public ASTNode {
public:
    ContinueStatementNode() {
        type = ASTNodeType::ContinueStatement;
    }
};

class TryCatchNode : public ASTNode {
public:
    std::unique_ptr<BlockNode> tryBlock; // the block of code to try
    std::unique_ptr<BlockNode> catchBlock; // the block of code to execute if an exception is thrown
    std::unique_ptr<LiteralNode> exception; // the variable to store the exception in (optional)

    TryCatchNode(std::unique_ptr<BlockNode>& tryBlock, std::unique_ptr<BlockNode>& catchBlock)
        : tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)) {
        type = ASTNodeType::TryCatch;
    }
};

// These nodes are unfinished for now, don't take them as final implementations.
class ImportNode : public ASTNode {
public:
    std::string moduleName; // the name of the module being imported
    std::vector<std::string> importNames; // the names of the items being imported from the module
    ASTImportType importType;

    ImportNode(const std::string& moduleName, const std::vector<std::string>& importNames = {}, ASTImportType importType)
        : moduleName(moduleName), importNames(importNames), importType(importType) {
        type = ASTNodeType::Import;
    }
};

class PreprocessorDirectiveNode : public ASTNode {
public:
    ASTPreprocessorDirectiveType directive; // the preprocessor directive (e.g. #include, #define, etc.)

    PreprocessorDirectiveNode(ASTPreprocessorDirectiveType& directive, const std::string& value = "")
        : directive(directive) {
        type = ASTNodeType::Preprocessor;
    }
};

// core nodes
class ModuleNode : public ASTNode {
public:
    std::string moduleName;
    std::vector<std::unique_ptr<ASTNode>> body;

    ModuleNode(const std::string& name) : moduleName(name) {
        type = ASTNodeType::Module;
    }
};

class ProgramNode : public ASTNode {
public:
    std::vector<ModuleNode> body;

    ProgramNode() {
        type = ASTNodeType::Program;
    }
};