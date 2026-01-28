/*
 *#include "SemanticAnalysis.hpp"

// this is ai-generated for now, i will fix it later i swear

void SemanticAnalysis::pushScope() {
    scopes.emplace_back();
}

void SemanticAnalysis::popScope() {
    if (!scopes.empty()) scopes.pop_back();
}

void SemanticAnalysis::report(ASTNode* at, const std::string& msg) {
    errorCount++;
    if (at) {
        std::println("[Semantic] {} at {}:{} in {}", msg, at->line, at->column, at->filePath);
    } else {
        std::println("[Semantic] {}", msg);
    }
}

bool SemanticAnalysis::declareName(const std::string& name, Symbol sym, ASTNode* where) {
    if (scopes.empty()) pushScope();
    auto& top = scopes.back();

    if (top.contains(name)) {
        report(where, std::format("Redefined name '{}'", name));
        return false;
    }

    if (where) {
        sym.file = where->filePath;
        sym.line = where->line;
        sym.col  = where->column;
    }

    top.emplace(name, std::move(sym));
    return true;
}

SemanticAnalysis::Symbol* SemanticAnalysis::findName(const std::string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return &it->second;
    }
    return nullptr;
}

void SemanticAnalysis::visit(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case ASTNodeType::Module:
            visitModule(static_cast<ModuleNode*>(node));
            break;

        case ASTNodeType::Block:
            visitBlock(static_cast<BlockNode*>(node));
            break;

        case ASTNodeType::Declaration:
            visitDeclaration(static_cast<DeclarationNode*>(node));
            break;

        case ASTNodeType::Variable:
            visitVariable(static_cast<VariableNode*>(node));
            break;

        case ASTNodeType::Assignment:
            visitAssignment(static_cast<AssignmentNode*>(node));
            break;

        case ASTNodeType::Function:
            visitFunction(static_cast<FunctionNode*>(node));
            break;

        case ASTNodeType::CallExpression:
            visitCall(static_cast<CallExpressionNode*>(node));
            break;

        case ASTNodeType::IfStatement:
            visitIf(static_cast<IfNode*>(node));
            break;

        case ASTNodeType::WhileLoop:
            visitWhile(static_cast<WhileLoopNode*>(node));
            break;

        case ASTNodeType::ForLoop:
            visitFor(static_cast<ForLoopNode*>(node));
            break;

        case ASTNodeType::ReturnStatement:
            visitReturn(static_cast<ReturnStatementNode*>(node));
            break;

        case ASTNodeType::BreakStatement:
            visitBreak(static_cast<BreakStatementNode*>(node));
            break;

        case ASTNodeType::ContinueStatement:
            visitContinue(static_cast<ContinueStatementNode*>(node));
            break;

        // expressions that contain children:
        case ASTNodeType::BinaryOperation: {
            auto* b = static_cast<BinaryOperationNode*>(node);
            visit(b->leftOperand.get());
            visit(b->rightOperand.get());
            break;
        }
        case ASTNodeType::UnaryOperation: {
            auto* u = static_cast<UnaryOperationNode*>(node);
            visit(u->operand.get());
            break;
        }
        case ASTNodeType::MemberAccess: {
            auto* m = static_cast<MemberAccessNode*>(node);
            visit(m->parent.get());
            visit(m->val.get());
            break;
        }

        default:
            // Literal / Array / Set / Dict / Class / etc — пока можно не трогать
            break;
    }
}

// ====== top-level ======
void SemanticAnalysis::visitModule(ModuleNode* module) {
    if (!module) return;

    // module scope
    pushScope();

    // Pass 1: predeclare top-level functions (чтобы можно было вызывать до определения)
    for (auto& st : module->body) {
        if (!st) continue;
        if (st->type == ASTNodeType::Function) {
            auto* fn = static_cast<FunctionNode*>(st.get());
            declareName(fn->name, Symbol{SymbolKind::Function}, fn);
        }
    }

    // Pass 2: walk everything
    for (auto& st : module->body) {
        visit(st.get());
    }

    popScope();
}

// ====== blocks / vars ======
void SemanticAnalysis::visitBlock(BlockNode* block) {
    if (!block) return;

    pushScope();
    for (auto& st : block->statements) {
        visit(st.get());
    }
    popScope();
}

void SemanticAnalysis::visitDeclaration(DeclarationNode* declaration) {
    if (!declaration) return;

    // initializer first: var x = x  -> UndefinedVariable
    visit(declaration->value.get());

    if (!declaration->variable) return;
    const std::string name = declaration->variable->varName;

    Symbol s{SymbolKind::Variable};
    s.rawType = declaration->rawType;
    declareName(name, s, declaration);
}

void SemanticAnalysis::visitVariable(VariableNode* variable) {
    if (!variable) return;

    if (!findName(variable->varName)) {
        report(variable, std::format("Undefined variable '{}'", variable->varName));
    }
}

void SemanticAnalysis::visitAssignment(AssignmentNode* assignment) {
    if (!assignment) return;

    // RHS
    visit(assignment->value.get());

    // LHS must be lvalue: Variable or MemberAccess
    if (!assignment->variable) return;

    auto t = assignment->variable->type;
    if (!(t == ASTNodeType::Variable || t == ASTNodeType::MemberAccess)) {
        report(assignment, "Assignment to non-lvalue");
        return;
    }

    // ensure the lvalue is valid (for Variable it checks declared)
    visit(assignment->variable.get());
}

// ====== functions / control flow ======
void SemanticAnalysis::visitFunction(FunctionNode* function) {
    if (!function) return;

    functionDepth++;
    pushScope();

    // params as locals + duplicate check
    std::unordered_map<std::string, bool> seen;
    for (auto& p : function->parameters) {
        if (!p) continue;

        if (seen.contains(p->parameterName)) {
            report(p.get(), std::format("Duplicate parameter '{}'", p->parameterName));
        } else {
            seen[p->parameterName] = true;
            Symbol s{SymbolKind::Parameter};
            s.rawType = p->parameterRawType;
            declareName(p->parameterName, s, p.get());
        }

        visit(p->defaultValue.get());
    }

    visit(function->body.get());

    popScope();
    functionDepth--;
}

void SemanticAnalysis::visitCall(CallExpressionNode* call) {
    if (!call) return;

    visit(call->callee.get());
    for (auto& a : call->arguments) visit(a.get());

    // If callee is VariableNode => must exist and be function
    if (call->callee && call->callee->type == ASTNodeType::Variable) {
        auto* v = static_cast<VariableNode*>(call->callee.get());
        auto* sym = findName(v->varName);
        if (!sym) {
            report(call, std::format("Undefined function '{}'", v->varName));
        } else if (sym->kind != SymbolKind::Function) {
            report(call, std::format("'{}' is not a function", v->varName));
        }
    }
}

void SemanticAnalysis::visitIf(IfNode* node) {
    if (!node) return;
    visit(node->condition.get());
    visit(node->thenBlock.get());
    visit(node->elseBlock.get());
}

void SemanticAnalysis::visitWhile(WhileLoopNode* node) {
    if (!node) return;
    visit(node->condition.get());
    loopDepth++;
    visit(node->body.get());
    loopDepth--;
}

void SemanticAnalysis::visitFor(ForLoopNode* node) {
    if (!node) return;

    visit(node->iterable.get());

    loopDepth++;
    pushScope();

    // for-loop variable exists inside loop scope
    if (node->variable) {
        declareName(node->variable->varName, Symbol{SymbolKind::Variable}, node->variable.get());
    }
    visit(node->body.get());

    popScope();
    loopDepth--;
}

void SemanticAnalysis::visitReturn(ReturnStatementNode* node) {
    if (!node) return;

    if (functionDepth <= 0) {
        report(node, "Return outside function");
    }
    visit(node->expression.get());
}

void SemanticAnalysis::visitBreak(BreakStatementNode* node) {
    if (!node) return;
    if (loopDepth <= 0) report(node, "Break outside loop");
}

void SemanticAnalysis::visitContinue(ContinueStatementNode* node) {
    if (!node) return;
    if (loopDepth <= 0) report(node, "Continue outside loop");
}
*/