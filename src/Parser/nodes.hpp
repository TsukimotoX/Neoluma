#ifndef NODES_HPP
#define NODES_HPP

#include <iostream>
#include "HelperFunctions.hpp"

enum ASTNodeType {
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
    * - SCDefault ✅
    * - ImportNode ✅
    * - PreprocessorDirective ✅
*/

enum ASTVariableType {
    Integer, Float, Number, Boolean, String,
    Array, Set, Dictionary, Void, Result,
};

enum ASTModifierType {
    Public, Private, Protected, Static, Const, Override, Async
};

enum ASTPreprocessorDirectiveType {
    Import, Unsafe, Baremetal, Float, Use, Macro
};

class ASTNode {
public:
    ASTNodeType type;
    string value; // for basic values like literals, etc.

    virtual ~ASTNode() = default;
    virtual void toString() const;
    virtual void generateCode(); // for the compiler to generate code from this AST node
};

 // All node types inherited from ASTNode, for the parser and compiler to determine.

class LiteralNode : public ASTNode {
public:
    LiteralNode(const string& val) {
        type = Literal;
        value = val;
    }
};

class VariableNode : public ASTNode {
public:
    string varName; // name of the variable
    ASTVariableType varType; // type of the variable, e.g. Integer, Float, etc.

    VariableNode(string& varName, ASTVariableType& varType): varName(varName), varType(varType) {
        type = Variable;
    }
};

class AssignmentNode : public ASTNode {
public:
    VariableNode* variable; // the variable being assigned to
    MemoryPtr<ASTNode> value; // the value being assigned

    AssignmentNode(VariableNode* variable, MemoryPtr<ASTNode>& value): variable(variable), value(std::move(value)) {
        type = Assignment;
    }
};

class BinaryOperationNode : public ASTNode {
public:
    MemoryPtr<ASTNode> leftOperand;
    MemoryPtr<ASTNode> rightOperand;

    BinaryOperationNode(MemoryPtr<ASTNode>& leftOp, const string& op, MemoryPtr<ASTNode>& rightOp)
        : leftOperand(std::move(leftOp)), rightOperand(std::move(rightOp)) {
        type = BinaryOperation;
        value = op;
    }
};

class UnaryOperationNode : public ASTNode {
public:
    MemoryPtr<ASTNode> operand; // because i'm terrible at english: it's the value after the operator, e.g. -x, !x, etc.

    UnaryOperationNode(const string& op, MemoryPtr<ASTNode>& operand)
        : operand(std::move(operand)) {
        type = UnaryOperation;
        value = op;
    }
};

class BlockNode : public ASTNode {
public:
    array<MemoryPtr<ASTNode>> statements;

    BlockNode() {
        type = Block;
    }
};

class IfNode : public ASTNode {
public:
    MemoryPtr<ASTNode> condition; // the condition of the if statement
    MemoryPtr<BlockNode> thenBlock; // the block of code to execute if the condition is true
    MemoryPtr<BlockNode> elseBlock; // the block of code to execute if the condition is false

    IfNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> thenBlock, MemoryPtr<BlockNode> elseBlock = nullptr)
        : condition(std::move(condition)), thenBlock(std::move(thenBlock)), elseBlock(std::move(elseBlock)) {
        type = IfStatement;
    }
};

class SwitchNode : public ASTNode {
public:
    MemoryPtr<ASTNode> expression; // the expression to switch on
    array<MemoryPtr<CaseNode>> cases; // the cases of the switch statement
    MemoryPtr<SCDefaultNode> defaultCase; // the default case of the switch statement

    SwitchNode(MemoryPtr<ASTNode> expression, array<MemoryPtr<CaseNode>> cases, MemoryPtr<SCDefaultNode> defaultCase = nullptr)
        : expression(std::move(expression)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
        type = Switch;
    }
};

class CaseNode : public ASTNode {
public:
    MemoryPtr<ASTNode> condition; // the condition of the case
    MemoryPtr<BlockNode> body; // the body of the case

    CaseNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        type = Case;
    }
};

class SCDefaultNode : public ASTNode {
public:
    MemoryPtr<BlockNode> body; // the body of the default case

    SCDefaultNode(MemoryPtr<BlockNode> body)
        : body(std::move(body)) {
        type = SCDefault;
    }
};

class ForLoopNode : public ASTNode {
public:
    MemoryPtr<VariableNode> variable; // the loop variable
    MemoryPtr<ASTNode> iterable; // the iterable to loop over
    MemoryPtr<BlockNode> body; // the body of the loop

    ForLoopNode(MemoryPtr<VariableNode> variable, MemoryPtr<ASTNode> iterable, MemoryPtr<BlockNode> body)
        : variable(std::move(variable)), iterable(std::move(iterable)), body(std::move(body)) {
        type = ForLoop;
    }
};

class WhileLoopNode : public ASTNode {
public:
    MemoryPtr<ASTNode> condition; // the condition of the while loop
    MemoryPtr<BlockNode> body; // the body of the loop

    WhileLoopNode(MemoryPtr<ASTNode> condition, MemoryPtr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {
        type = WhileLoop;
    }
};

class FunctionNode : public ASTNode {
public:
    string name; // the name of the function
    array<MemoryPtr<DecoratorNode>> decorators; // the decorators applied to the function
    array<MemoryPtr<ModifierNode>> modifiers; // the modifiers applied to the function (e.g. async, static, etc.)
    array<MemoryPtr<ParameterNode>> parameters; // the parameters of the function
    MemoryPtr<BlockNode> body; // the body of the function

    FunctionNode(const string& name, array<MemoryPtr<ParameterNode>> parameters, MemoryPtr<BlockNode> body,
                 array<MemoryPtr<DecoratorNode>> decorators = {}, array<MemoryPtr<ModifierNode>> modifiers = {})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = Function;
    }
};

class ClassNode : public ASTNode {
public:
    string name; // the name of the class
    array<MemoryPtr<DecoratorNode>> decorators; // the decorators applied to the class
    array<MemoryPtr<ModifierNode>> modifiers; // the modifiers applied to the class (e.g. public, private, etc.)
    array<MemoryPtr<VariableNode>> fields; // the fields of the class
    array<MemoryPtr<FunctionNode>> methods; // the methods of the class

    ClassNode(const string& name, array<MemoryPtr<VariableNode>> fields, array<MemoryPtr<FunctionNode>> methods,
              array<MemoryPtr<DecoratorNode>> decorators = {}, array<MemoryPtr<ModifierNode>> modifiers = {})
        : name(name), fields(std::move(fields)), methods(std::move(methods)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = Class;
    }
};

class DecoratorNode : public ASTNode {
public:
    string name; // the name of the decorator
    array<MemoryPtr<DecoratorNode>> decorators; // the decorators applied to the decorator
    array<MemoryPtr<ModifierNode>> modifiers; // the modifiers applied to the decorator (e.g. async, static, etc.)
    array<MemoryPtr<ParameterNode>> parameters; // the parameters of the decorator
    MemoryPtr<BlockNode> body; // the body of the decorator

    DecoratorNode(const string& name, array<MemoryPtr<ParameterNode>> parameters, MemoryPtr<BlockNode> body,
                 array<MemoryPtr<DecoratorNode>> decorators = {}, array<MemoryPtr<ModifierNode>> modifiers = {})
        : name(name), parameters(std::move(parameters)), body(std::move(body)), decorators(std::move(decorators)), modifiers(std::move(modifiers)) {
        type = ASTNodeType::Decorator;
    }
};

class ReturnStatementNode : public ASTNode {
public:
    MemoryPtr<ASTNode> expression; // the expression to return

    ReturnStatementNode(MemoryPtr<ASTNode>& expression)
        : expression(std::move(expression)) {
        type = ReturnStatement;
    }
};

class CallExpressionNode : public ASTNode {
public:
    MemoryPtr<ASTNode> callee; // the function or method being called
    array<MemoryPtr<ParameterNode>> arguments; // the arguments passed to the function or method

    CallExpressionNode(MemoryPtr<ASTNode>& callee, array<MemoryPtr<ParameterNode>>& arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {
        type = CallExpression;
    }
};

class ThrowStatementNode : public ASTNode {
public:
    MemoryPtr<ASTNode> expression; // the expression to throw

    ThrowStatementNode(MemoryPtr<ASTNode>& expression)
        : expression(std::move(expression)) {
        type = ThrowStatement;
    }
};

class BreakStatementNode : public ASTNode {
public:
    BreakStatementNode() {
        type = BreakStatement;
    }
};

class ContinueStatementNode : public ASTNode {
public:
    ContinueStatementNode() {
        type = ContinueStatement;
    }
};

class TryCatchNode : public ASTNode {
public:
    MemoryPtr<BlockNode> tryBlock; // the block of code to try
    MemoryPtr<BlockNode> catchBlock; // the block of code to execute if an exception is thrown
    MemoryPtr<LiteralNode> exception; // the variable to store the exception in (optional)

    TryCatchNode(MemoryPtr<BlockNode>& tryBlock, MemoryPtr<BlockNode>& catchBlock)
        : tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)) {
        type = TryCatch;
    }
};

class ParameterNode : public ASTNode {
public:
    string parameterName; // the name of the parameter
    ASTVariableType parameterType; // the type of the parameter
    std::optional<string> defaultValue; // the default value of the parameter (optional)

    ParameterNode(const string& parameterName, ASTVariableType& parameterType, const string& defaultValue = nullptr)
        : parameterName(parameterName), parameterType(parameterType) {
        this->type = Parameter;
        if (!defaultValue.empty()) this->defaultValue = defaultValue;
    }
};

class ModifierNode : public ASTNode {
public:
    ASTModifierType modifier;

    ModifierNode(ASTModifierType& modifier) : modifier(modifier) {
        this->type = Modifier;
    }
};

// These nodes are unfinished for now, don't take them as final implementations.
class ImportNode : public ASTNode {
public:
    string moduleName; // the name of the module being imported
    array<string> importNames; // the names of the items being imported from the module

    ImportNode(const string& moduleName, const array<string>& importNames = {})
        : moduleName(moduleName), importNames(importNames) {
        type = ASTNodeType::Import;
    }
};

class PreprocessorDirectiveNode : public ASTNode {
public:
    ASTPreprocessorDirectiveType directive; // the preprocessor directive (e.g. #include, #define, etc.)

    PreprocessorDirectiveNode(ASTPreprocessorDirectiveType& directive, const string& value = "")
        : directive(directive) {
        type = ASTNodeType::Preprocessor;
    }
};

#endif