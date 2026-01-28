#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <print>
#include "Core/Frontend/nodes.hpp"

struct SemanticAnalysis {

    // Main function
    void visitModule(ModuleNode* module);

    // Visitors
    void visitBlock(BlockNode* block);
    void visitDeclaration(DeclarationNode* declaration);
    void visitVariable(VariableNode* variable);
    void visitAssignment(AssignmentNode* assignment);
    void visitFunction(FunctionNode* function);

    // Helper visitors
    void visitCall(CallExpressionNode* call);
    void visitIf(IfNode* node);
    void visitWhile(WhileLoopNode* node);
    void visitFor(ForLoopNode* node);
    void visitReturn(ReturnStatementNode* node);
    void visitBreak(BreakStatementNode* node);
    void visitContinue(ContinueStatementNode* node);

private:
    enum class SymbolKind { Variable, Function, Parameter, Class, Enum, Interface, Decorator };

    struct Symbol {
        SymbolKind kind;
        std::string rawType = "None";
        bool isConst = false;

        std::string file;
        int line = 0;
        int col = 0;
    };

    // Table of variables, basically
    std::vector<std::unordered_map<std::string, Symbol>> scopes;

    int loopDepth = 0;
    int functionDepth = 0;
    int errorCount = 0;

    // helpers
    void pushScope();
    void popScope();

    bool declareName(const std::string& name, Symbol sym, ASTNode* where);
    Symbol* findName(const std::string& name);

    // one dispatcher to walk any node
    void visit(ASTNode* node);

    // report
    void report(ASTNode* at, const std::string& msg);
};

// ## **Implementation Suggestions**
//
// For your semantic analyzer, you'll need:
//
// 1. **Symbol Table** - Track variables, functions, classes in scopes
// 2. **Type Checker** - Validate type compatibility across operations
// 3. **Scope Manager** - Handle nested scopes for blocks, functions, classes
// 4. **Control Flow Analyzer** - Track reachability and proper break/continue usage
// 5. **AST Walker** - Traverse your AST nodes systematically

