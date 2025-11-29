// note from tsuki: this is tf2 coconut of this entire project. if you delete it
// the entire project will blow tf up. don't delete it.
// xD /jk
#include "nodes.hpp"

ASTNode::~ASTNode() = default;

// if you get this output it means you screwed up -tsuki
std::string ASTNode::toString(int indent) const { return std::string(indent, ' ') + "<ASTNode>"; }

// All toString implementations moved here from the header
std::string LiteralNode::toString(int indent) const {
    return std::format("{}Literal(value={})", std::string(indent, ' '), value);
}

std::string VariableNode::toString(int indent) const {
    return std::format("{}Variable(varName={})", std::string(indent, ' '), varName);
}

std::string DeclarationNode::toString(int indent) const {
    return std::format("{}Declaration {{\n{}variable: {}\n{}rawType: {}\n{}value: {}\n{}isNullable: {}\n{}}}",
        std::string(indent, ' '),
        std::string(indent+2, ' '), variable ? variable->toString(indent+2) : "null",
        std::string(indent+2, ' '), rawType,
        std::string(indent+2, ' '), value ? value->toString(indent+2) : "null",
        std::string(indent+2, ' '), isNullable ? "true" : "false",
        std::string(indent, ' '));
}

std::string AssignmentNode::toString(int indent) const {
    return std::format("{}Assignment {{\n{}variable: {}\n{}op: {}\n{}value: {}\n{}}}",
        std::string(indent, ' '),
        std::string(indent+2, ' '), variable ? variable->toString(indent+2) : "null",
        std::string(indent+2, ' '), op,
        std::string(indent+2, ' '), value ? value->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string MemberAccessNode::toString(int indent) const {
    return std::format("{}MemberAccess {{\n{}parent: {}\n{}val: {}\n{}}}",
        std::string(indent, ' '),
        std::string(indent+2, ' '), parent ? parent->toString(indent+2) : "null",
        std::string(indent+2, ' '), val ? val->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string BinaryOperationNode::toString(int indent) const {
    return std::format("{}BinaryOperation({}){{\n{}{}, \n{}{}\n{}}}",
        std::string(indent, ' '), value,
        std::string(indent+2, ' '), leftOperand ? leftOperand->toString(indent+2) : "null",
        std::string(indent+2, ' '), rightOperand ? rightOperand->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string UnaryOperationNode::toString(int indent) const {
    return std::format("{}UnaryOperation({}){{\n{}{}\n{}}}",
        std::string(indent, ' '), value,
        std::string(indent+2, ' '), operand ? operand->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string BlockNode::toString(int indent) const {
    std::string res = std::string(indent, ' ') + "Block {\n";
    for (const auto& stmt : statements) {
        if (stmt) res += stmt->toString(indent+2) + "\n";
    }
    res += std::string(indent, ' ') + "}";
    return res;
}

std::string IfNode::toString(int indent) const {
    return std::format("{}If{{\ncondition: {}\nthenBlock: {}\nelseBlock: {}}}",
        std::string(indent, ' '),
        condition ? condition->toString(indent + 2) : "null",
        thenBlock ? thenBlock->toString(indent + 2) : "null",
        elseBlock ? elseBlock->toString(indent + 2) : "None");
}

std::string SCDefaultNode::toString(int indent) const {
    return std::format("{}SCDefault{{\n{}{}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string CaseNode::toString(int indent) const {
    return std::format("{}Case{{\n{}condition: {}\n{}body: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), condition ? condition->toString(indent+2) : "null",
        std::string(indent+2, ' '), body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string SwitchNode::toString(int indent) const {
    std::string res = std::format("{}Switch{{\n{}expression: {}\n", std::string(indent, ' '),
        std::string(indent+2, ' '), expression ? expression->toString(indent+2) : "null");
    res += std::string(indent+2, ' ') + "cases: [\n";
    for (const auto& caseNode : cases) {
        res += caseNode ? caseNode->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}defaultCase: {}\n{} }}", std::string(indent+2, ' '),
        defaultCase ? defaultCase->toString(indent+2) : "null",
        std::string(indent, ' '));
    return res;
}

std::string ForLoopNode::toString(int indent) const {
    return std::format("{}ForLoop{{\n{}variable: {}\n{}iterable: {}\n{}body: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), variable ? variable->toString(indent+2) : "null",
        std::string(indent+2, ' '), iterable ? iterable->toString(indent+2) : "null",
        std::string(indent+2, ' '), body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string WhileLoopNode::toString(int indent) const {
    return std::format("{}WhileLoop{{\n{}condition: {}\n{}body: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), condition ? condition->toString(indent+2) : "null",
        std::string(indent+2, ' '), body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string BreakStatementNode::toString(int indent) const { return std::format("{}BreakStatement", std::string(indent, ' ')); }
std::string ContinueStatementNode::toString(int indent) const { return std::format("{}ContinueStatement", std::string(indent, ' ')); }

std::string ReturnStatementNode::toString(int indent) const {
    return std::format("{}ReturnStatement{{\n{}expression: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), expression ? expression->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string ThrowStatementNode::toString(int indent) const {
    return std::format("{}ThrowStatement{{\n{}expression: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), expression ? expression->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string TryCatchNode::toString(int indent) const {
    return std::format("{}TryCatch{{\n{}tryBlock: {}\n{}catchBlock: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), tryBlock ? tryBlock->toString(indent+2) : "null",
        std::string(indent+2, ' '), catchBlock ? catchBlock->toString(indent+2) : "null",
        std::string(indent, ' '));
}

std::string ArrayNode::toString(int indent) const {
    std::string res = std::format("{}Array{{\n{}elements: [\n", std::string(indent, ' '), std::string(indent+2, ' '));
    for (const auto& elem : elements) {
        res += elem ? elem->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}typeHint: {}\n{}}}", std::string(indent+2, ' '),
        typeHint ? typeHint->toString(indent+2) : "null",
        std::string(indent, ' '));
    return res;
}

std::string SetNode::toString(int indent) const {
    std::string res = std::format("{}Set{{\n{}elements: [\n", std::string(indent, ' '), std::string(indent+2, ' '));
    for (const auto& elem : elements) {
        res += elem ? elem->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}typeHint: {}\n{}}}", std::string(indent+2, ' '),
        typeHint ? typeHint->toString(indent+2) : "null",
        std::string(indent, ' '));
    return res;
}

std::string DictNode::toString(int indent) const {
    std::string res = std::format("{}Dict{{\n{}elements: [\n", std::string(indent, ' '), std::string(indent+2, ' '));
    for (const auto& elem : elements) {
        res += std::format("{}Key: {}\n{}Value: {}\n", std::string(indent+4, ' '),
            elem.first ? elem.first->toString(indent+6) : "null",
            std::string(indent+4, ' '),
            elem.second ? elem.second->toString(indent+6) : "null");
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}types: [keyType={}, valueType={}]\n{}}}", std::string(indent+2, ' '),
        types[0], types[1],
        std::string(indent, ' '));
    return res;
}

std::string VoidNode::toString(int indent) const { return std::format("{}Void", std::string(indent, ' ')); }

std::string ResultNode::toString(int indent) const {
    return std::format("{}Result{{\n{}t: {}\n{}e: {}\n{}isError: {}\n{}}}", std::string(indent, ' '),
        std::string(indent+2, ' '), t ? t->toString(indent+2) : "null",
        std::string(indent+2, ' '), e ? e->toString(indent+2) : "null",
        std::string(indent+2, ' '), isError ? "true" : "false",
        std::string(indent, ' '));
}

std::string ParameterNode::toString(int indent) const {
    return std::format("{}Parameter(parameterName={}, parameterRawType={}, defaultValue={})", std::string(indent, ' '), parameterName, parameterRawType, defaultValue ? defaultValue->toString(indent+2) : "null");
}

std::string ModifierNode::toString(int indent) const {
    auto modifierString = [](ASTModifierType modifier) -> std::string {
        switch (modifier) {
            case ASTModifierType::Public: return "Public";
            case ASTModifierType::Private: return "Private";
            case ASTModifierType::Protected: return "Protected";
            case ASTModifierType::Static: return "Static";
            case ASTModifierType::Const: return "Const";
            case ASTModifierType::Override: return "Override";
            case ASTModifierType::Async: return "Async";
            case ASTModifierType::Debug: return "Debug";
            default: return "Unknown";
        }
    };
    std::string modStr = modifierString(modifier);
    return std::format("{}Modifier(modifier={})", std::string(indent, ' '), modStr);
}

std::string CallExpressionNode::toString(int indent) const {
    std::string res = std::format("{}CallExpression{{\n{}callee: {}\n", std::string(indent, ' '),
        std::string(indent+2, ' '), callee ? callee->toString(indent+2) : "null");
    res += std::string(indent+2, ' ') + "arguments: [\n";
    for (const auto& arg : arguments) {
        res += arg ? arg->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::string(indent, ' ') + "}";
    return res;
}

std::string EnumMemberNode::toString(int indent) const {
    return std::format("{}EnumMember(name={}, value={})", std::string(indent, ' '), name, value ? value->toString(indent) : "null");
}

std::string EnumNode::toString(int indent) const {
    std::string res = std::format("{}Enum{{\n", std::string(indent, ' '));
    res += std::string(indent+2, ' ') + "elements: [\n";
    for (const auto& elem : elements) {
        res += elem ? elem->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::string(indent, ' ') + "}";
    return res;
}

std::string InterfaceFieldNode::toString(int indent) const {
    std::string res = std::format("{}InterfaceField(name={}, rawType={}, isNullable={}){{\n", std::string(indent, ' '), name, rawType, isNullable ? "true" : "false");
    if (isFunction) {
        res += std::string(indent+2, ' ') + "isFunction: true\n";
        res += std::string(indent+2, ' ') + "parameters: [\n";
        for (const auto& param : parameters) {
            res += param ? param->toString(indent+4) + "\n" : "null\n";
        }
        res += std::string(indent+2, ' ') + "]\n";
        res += std::format("{}returnType: {}\n{}}}", std::string(indent+2, ' '),
            returnType ? returnType->toString(indent+2) : "null",
            std::string(indent, ' '));
    } else {
        res += std::string(indent+2, ' ') + "isFunction: false\n";
        res += std::string(indent, ' ') + "}";
    }
    return res;
}

std::string InterfaceNode::toString(int indent) const {
    std::string res = std::format("{}Interface{{\n", std::string(indent, ' '));
    res += std::string(indent+2, ' ') + "elements: [\n";
    for (const auto& elem : elements) {
        res += elem ? elem->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::string(indent, ' ') + "}";
    return res;
}

std::string LambdaNode::toString(int indent) const {
    std::string res = std::format("{}Lambda{{\n", std::string(indent, ' '));
    res += std::string(indent+2, ' ') + "params: [\n";
    for (const auto& param : params) {
        res += param ? param->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}body: {}\n{}}}", std::string(indent+2, ' '),
        body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
    return res;
}

std::string FunctionNode::toString(int indent) const {
    std::string res = std::format("{}Function(name={}){{\n", std::string(indent, ' '), name);
    res += std::string(indent+2, ' ') + "parameters: [\n";
    for (const auto& param : parameters) {
        res += param ? param->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}body: {}\n{}}}", std::string(indent+2, ' '),
        body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
    return res;
}

std::string ClassNode::toString(int indent) const {
    std::string res = std::format("{}Class(name={}){{\n", std::string(indent, ' '), name);
    res += std::string(indent+2, ' ') + "super (inherited from): " + (super ? super->toString(indent+2) : "null") + "\n";

    res += std::string(indent+2, ' ') + "constructor: {}\n" +
        (constructor ? constructor->toString(indent+2) : "null") + "\n";

    res += std::string(indent+2, ' ') + "fields: [\n";
    for (const auto& field : fields) {
        res += field ? field->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";

    res += std::string(indent+2, ' ') + "methods: [\n";
    for (const auto& method : methods) {
        res += method ? method->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::string(indent, ' ') + "}";
    return res;
}

std::string DecoratorNode::toString(int indent) const {
    std::string res = std::format("{}Decorator(name={}){{\n", std::string(indent, ' '), name);
    res += std::string(indent+2, ' ') + "parameters: [\n";
    for (const auto& param : parameters) {
        res += param ? param->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::format("{}body: {}\n{}}}", std::string(indent+2, ' '),
        body ? body->toString(indent+2) : "null",
        std::string(indent, ' '));
    return res;
}

std::string ImportNode::toString(int indent) const {
    auto importTypeString = [](ASTImportType importType) -> std::string {
        switch (importType) {
            case ASTImportType::Native: return "Native";
            case ASTImportType::Relative: return "Relative";
            case ASTImportType::Foreign: return "Foreign";
            case ASTImportType::ForeignRelative: return "ForeignRelative";
            default: return "Unknown";
        }
    };
    std::string impTypeStr = importTypeString(importType);
    return std::format("{}Import(moduleName={}, alias={}, importType={})", std::string(indent, ' '), moduleName, alias, impTypeStr);
}

std::string PreprocessorDirectiveNode::toString(int indent) const {
    auto directiveString = [](ASTPreprocessorDirectiveType directive) -> std::string {
        switch (directive) {
            case ASTPreprocessorDirectiveType::Import: return "Import";
            case ASTPreprocessorDirectiveType::Unsafe: return "Unsafe";
            case ASTPreprocessorDirectiveType::Baremetal: return "Baremetal";
            case ASTPreprocessorDirectiveType::Float: return "Float";
            case ASTPreprocessorDirectiveType::Macro: return "Macro";
            case ASTPreprocessorDirectiveType::None: return "None";
            default: return "Unknown";
        }
    };
    std::string dirStr = directiveString(directive);
    return std::format("{}Preprocessor(directive={}, value={})", std::string(indent, ' '), dirStr, value);
}

std::string ModuleNode::toString(int indent) const {
    std::string res = std::format("{}Module(name={}){{\n", std::string(indent, ' '), moduleName);
    res += std::string(indent+2, ' ') + "body: [\n";
    for (const auto& node : body) {
        res += node ? node->toString(indent+4) + "\n" : "null\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::string(indent, ' ') + "}";
    return res;
}

std::string ProgramNode::toString(int indent) const {
    std::string res = std::format("{}Program{{\n", std::string(indent, ' '));
    res += std::string(indent+2, ' ') + "body: [\n";
    for (const auto& module : body) {
        res += module.toString(indent+4) + "\n";
    }
    res += std::string(indent+2, ' ') + "]\n";
    res += std::string(indent, ' ') + "}";
    return res;
}
