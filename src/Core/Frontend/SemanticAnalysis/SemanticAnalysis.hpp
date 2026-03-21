#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Core/Compiler.hpp"

// ResolvedType is an enum of types Neoluma compiler internally supports by default.
enum class ResolvedType {
    Int8, Int16, Int, Int64, Int128,
    UInt8, UInt16, UInt, UInt64, UInt128,
    Float32, Float64,
    Number, Bool, String,
    Array, Dict, Set, Result,
    Void, UserDefined, Unknown
};
struct EResolvedType {
    ResolvedType type;
    std::string name;
};
const std::unordered_map<std::string, ResolvedType>& getTypeMap();
const std::unordered_map<ResolvedType, std::string>& getTypeNames();

// Semantic Analysis is a part of Frontend in Compiler responsible for logical part of the code.
struct SemanticAnalysis {
    Compiler* compiler = nullptr;
    void setCompiler(Compiler* comp) { compiler = comp; }

    // Main entry
    void analyzeProgram(const ProgramUnit& program, const std::vector<ModuleInfo>& infos);

    // Per-node analyzers
    void analyzeModule(ModuleNode* module);
    void analyzeBlock(BlockNode* block);
    void analyzeDeclaration(DeclarationNode* node);
    void analyzeAssignment(AssignmentNode* node);
    void analyzeFunction(FunctionNode* node);
    void analyzeClass(ClassNode* node);
    void analyzeEnum(EnumNode* node);
    void analyzeInterface(InterfaceNode* node);
    void analyzeCallExpression(CallExpressionNode* node);
    void analyzeDecorator(DecoratorNode* node);
    void analyzeIf(IfNode* node);
    void analyzeSwitch(SwitchNode* node);
    void analyzeWhile(WhileLoopNode* node);
    void analyzeFor(ForLoopNode* node);
    void analyzeTryCatch(TryCatchNode* node);
    void analyzeReturn(ReturnStatementNode* node);
    void analyzeThrow(ThrowStatementNode* node);
    void analyzeBreak(BreakStatementNode* node);
    void analyzeContinue(ContinueStatementNode* node);
    void analyzeLambda(LambdaNode* node);
    void analyzeExpression(ASTNode* node); // dispatcher for expressions only

    void analyzeStatement(ASTNode* node);
    ResolvedType resolveType(RawTypeNode* type);

private:
    struct Symbol {
        enum class Kind { Variable, Function, Parameter, Class, Enum, Interface, Decorator };
        Kind kind;
        bool isConst = false;
        std::string filePath;
        int line = 0, column = 0;
    };

    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    int loopDepth = 0;
    int functionDepth = 0;

    // Scope helpers
    void pushScope();
    void popScope();
    bool declareName(const std::string& name, Symbol symbol, ASTNode* node);
    Symbol* findName(const std::string& name);

    // just helpers
    bool match(ASTNode* node, ASTNodeType type) {
        if (node->type == type) return true;
        return false;
    }
};