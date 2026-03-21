#include "SemanticAnalysis.hpp"

// ==== Main function ====
void SemanticAnalysis::analyzeProgram(const ProgramUnit& program, const std::vector<ModuleInfo>& infos){
    for (ModuleId id : program.order)
        analyzeModule(infos[id].module);
}

/* Module analysis usually breaks down into two passes
 * 1. Declaration - we get all the symbols used in code, before checking their work logic
 * 2. Analysis - we pass through the whole module body, calling each analyze* for each part
 */
void SemanticAnalysis::analyzeModule(ModuleNode* module) {
    pushScope();

    // Declaration pass
    for (const auto& statement : module->body){
        if (match(statement.get(), ASTNodeType::Function)) {
            auto* node = static_cast<FunctionNode*>(statement.get());
            bool isConst = false;
            for (auto& modifier : node->modifiers) if (modifier.get()->modifier == ASTModifierType::Const) isConst = true;
            declareName(node->name, Symbol{Symbol::Kind::Function, isConst, node->filePath, node->line, node->column}, node);
        }
        else if (match(statement.get(), ASTNodeType::Class)) {
            auto* node = static_cast<ClassNode*>(statement.get());
            bool isConst = false;
            for (auto& modifier : node->modifiers) if (modifier.get()->modifier == ASTModifierType::Const) isConst = true;
            declareName(node->name, Symbol{Symbol::Kind::Class, isConst, node->filePath, node->line, node->column}, node);
        }
        else if (match(statement.get(), ASTNodeType::Enum)) {
            auto* node = static_cast<EnumNode*>(statement.get());
            bool isConst = false;
            for (auto& modifier : node->modifiers) if (modifier.get()->modifier == ASTModifierType::Const) isConst = true;
            declareName(node->name, Symbol{Symbol::Kind::Enum, isConst, node->filePath, node->line, node->column}, node);
        }
        else if (match(statement.get(), ASTNodeType::Interface)) {
            auto* node = static_cast<InterfaceNode*>(statement.get());
            bool isConst = false;
            for (auto& modifier : node->modifiers) if (modifier.get()->modifier == ASTModifierType::Const) isConst = true;
            declareName(node->name, Symbol{Symbol::Kind::Interface, isConst, node->filePath, node->line, node->column}, node);
        }
        else if (match(statement.get(), ASTNodeType::Decorator)) {
            auto* node = static_cast<DecoratorNode*>(statement.get());
            bool isConst = false;
            for (auto& modifier : node->modifiers) if (modifier.get()->modifier == ASTModifierType::Const) isConst = true;
            declareName(node->name, Symbol{Symbol::Kind::Decorator, isConst, node->filePath, node->line, node->column}, node);
        }
    }

    // Analysis pass
    for (const auto& statement : module->body)
        analyzeStatement(statement.get());

    popScope();
}

void SemanticAnalysis::analyzeFunction(FunctionNode* node) {
    pushScope();
    functionDepth++;

    for (const auto& parameter : node->parameters) {
        if (findName(parameter->parameterName)) {
            compiler->errorManager.addError(ErrorType::Analysis, AnalysisErrors::DuplicateParameterName,
                ErrorSpan{parameter->filePath, parameter->parameterName, parameter->line, parameter->column},
                "ErrorManager.Analysis.DuplicateParameterName.message", {parameter->parameterName, node->name},
                "ErrorManager.Analysis.DuplicateParameterName.hint");
            return;
        }

        //FIXME: Right now parameters do not support const. this must be addressed.
        declareName(parameter->parameterName, Symbol{Symbol::Kind::Parameter, false, node->filePath, node->line, node->column}, parameter.get());

        if (parameter->defaultValue) analyzeExpression(parameter->defaultValue.get());
    }

    analyzeBlock(node->body.get());
    functionDepth--;
    popScope();
}

void SemanticAnalysis::analyzeBlock(BlockNode* node) {
    pushScope();

    for (const auto& statement : node->statements)
        analyzeStatement(statement.get());

    popScope();
}

void SemanticAnalysis::analyzeStatement(ASTNode* statement) {
    switch (statement->type) {
        case ASTNodeType::Block: analyzeBlock(static_cast<BlockNode*>(statement)); break;
        case ASTNodeType::Declaration: analyzeDeclaration(static_cast<DeclarationNode*>(statement)); break;
        case ASTNodeType::Assignment: analyzeAssignment(static_cast<AssignmentNode*>(statement)); break;
        case ASTNodeType::Function: analyzeFunction(static_cast<FunctionNode*>(statement)); break;
        case ASTNodeType::Class: analyzeClass(static_cast<ClassNode*>(statement)); break;
        case ASTNodeType::Enum: analyzeEnum(static_cast<EnumNode*>(statement)); break;
        case ASTNodeType::Interface: analyzeInterface(static_cast<InterfaceNode*>(statement)); break;
        case ASTNodeType::CallExpression: analyzeCallExpression(static_cast<CallExpressionNode*>(statement)); break;
        case ASTNodeType::Decorator: analyzeDecorator(static_cast<DecoratorNode*>(statement)); break;
        case ASTNodeType::IfStatement: analyzeIf(static_cast<IfNode*>(statement)); break;
        case ASTNodeType::Switch: analyzeSwitch(static_cast<SwitchNode*>(statement)); break;
        case ASTNodeType::WhileLoop: analyzeWhile(static_cast<WhileLoopNode*>(statement)); break;
        case ASTNodeType::ForLoop: analyzeFor(static_cast<ForLoopNode*>(statement)); break;
        case ASTNodeType::TryCatch: analyzeTryCatch(static_cast<TryCatchNode*>(statement)); break;
        case ASTNodeType::ReturnStatement: analyzeReturn(static_cast<ReturnStatementNode*>(statement)); break;
        case ASTNodeType::ThrowStatement: analyzeThrow(static_cast<ThrowStatementNode*>(statement)); break;
        case ASTNodeType::BreakStatement: analyzeBreak(static_cast<BreakStatementNode*>(statement)); break;
        case ASTNodeType::ContinueStatement: analyzeContinue(static_cast<ContinueStatementNode*>(statement)); break;
        case ASTNodeType::Lambda: analyzeLambda(static_cast<LambdaNode*>(statement)); break;
        case ASTNodeType::Import: break; // Imports are already resolved by the Orchestrator
        default: analyzeExpression(statement); break;
    }
}

void SemanticAnalysis::analyzeDeclaration(DeclarationNode* node) {
    // at first you make sure what the value is to not screw up with x := x being undefined
    if (node->value) analyzeExpression(node->value.get());

    if (node->isTypeInference == true && node->rawType) {
        auto type = resolveType(node->rawType.get());
        if (type == ResolvedType::Unknown)
            compiler->errorManager.addError(ErrorType::Analysis, AnalysisErrors::UnknownType,
        ErrorSpan{node->rawType->filePath, node->rawType->varType->varName, node->rawType->line, node->rawType->column},
        "ErrorManager.Analysis.UnknownType.message", {node->rawType->varType->varName, node->variable->varName},
        "ErrorManager.Analysis.UnknownType.hint");
    }

    bool isConst = false;
    for (auto& modifier : node->modifiers) if (modifier.get()->modifier == ASTModifierType::Const) isConst = true;
    declareName(node->variable->varName, Symbol{Symbol::Kind::Variable, isConst, node->filePath, node->line, node->column}, node);
}

// analyzeAssignment next and etc.

ResolvedType SemanticAnalysis::resolveType(RawTypeNode* type) {
    auto varType = type->varType.get()->varName;
    auto tm = getTypeMap();

    // first check built-ins
    if (tm.contains(varType)) return tm.find(varType)->second;

    // well, perhaps it's user-defined?
    auto* userDefined = findName(varType);
    if (userDefined && (userDefined->kind == Symbol::Kind::Class || userDefined->kind == Symbol::Kind::Enum || userDefined->kind == Symbol::Kind::Interface)) return ResolvedType::UserDefined;

    // The type is unknown. Error message context is added at parent call.
    return ResolvedType::Unknown;
}

// ==== Helpers ====

void SemanticAnalysis::pushScope() { scopes.emplace_back(); }

void SemanticAnalysis::popScope() { if (!scopes.empty()) scopes.pop_back(); }

bool SemanticAnalysis::declareName(const std::string& name, Symbol symbol, ASTNode* node)
{
    if (scopes.empty()) pushScope();
    auto& parent = scopes.back();

    if (parent.contains(name)){
        compiler->errorManager.addError(ErrorType::Analysis, AnalysisErrors::RedefinedVariable,
            ErrorSpan{node->filePath, node->value, node->line, node->column},
            "ErrorManager.Analysis.RedefinedVariable.message", {node->value},
            "ErrorManager.Analysis.RedefinedVariable.hint");
        return false;
    }

    symbol.filePath = node->filePath;
    symbol.line = node->line;
    symbol.column = node->column;

    parent.emplace(name, symbol);
    return true;
}

SemanticAnalysis::Symbol* SemanticAnalysis::findName(const std::string& name) {
    for (int i = (int)scopes.size()-1; i >= 0; i--){
        if (scopes[i].contains(name))
            return &scopes[i].find(name)->second;
    }
    return nullptr;
}

const EResolvedType typesMap[] = {
    { ResolvedType::Int8, "int8" }, { ResolvedType::Int16, "int16" }, { ResolvedType::Int, "int" }, { ResolvedType::Int64, "int64" },
    { ResolvedType::Int128, "int128" }, { ResolvedType::UInt8, "uint8" }, { ResolvedType::UInt16, "uint16" }, { ResolvedType::UInt, "uint" },
    { ResolvedType::UInt64, "uint64" }, { ResolvedType::UInt128, "uint128" }, { ResolvedType::Float32, "float32" }, { ResolvedType::Float64, "float64" },
    { ResolvedType::Number, "number" }, { ResolvedType::Bool, "bool" }, { ResolvedType::String, "string" }, { ResolvedType::Array, "array" },
    { ResolvedType::Dict, "dict" }, { ResolvedType::Set, "set" }, { ResolvedType::Result, "result" }, { ResolvedType::Void, "void" },
};

const std::unordered_map<std::string, ResolvedType>& getTypeMap() {
    static std::unordered_map<std::string, ResolvedType> map;
    if (map.empty()) for (auto& k : typesMap) map[k.name] = k.type;
    return map;
}

const std::unordered_map<ResolvedType, std::string>& getTypeNames() {
    static std::unordered_map<ResolvedType, std::string> map;
    if (map.empty()) for (auto& k : typesMap) map[k.type] = k.name;
    return map;
}
