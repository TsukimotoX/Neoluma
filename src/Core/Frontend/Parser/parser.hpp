#pragma once

#include "../Lexer/lexer.hpp"
#include "../nodes.hpp"
#include "../token.hpp"
#include <iostream>

// to make math order
int getOperatorPrecedence(const std::string& op){
    auto on = getOperatorNames();
    if (op == on[Operators::Power]) return 7;
    else if (op == on[Operators::Multiply] || op == on[Operators::Divide] || op == on[Operators::Modulo]) return 6;
    else if (op == on[Operators::Add] || op == on[Operators::Subtract]) return 5;
    else if (op == on[Operators::BitwiseLeftShift] || op == on[Operators::BitwiseRightShift]) return 4;
    else if (op == on[Operators::BitwiseAnd]) return 3;
    else if (op == on[Operators::BitwiseXOr]) return 2;
    else if (op == on[Operators::BitwiseOr]) return 1;
    else if (op == on[Operators::Equal] || op == on[Operators::NotEqual] || op == on[Operators::LessThan] || op == on[Operators::GreaterThan] || op == on[Operators::LessThanOrEqual] || op == on[Operators::GreaterThanOrEqual]) return 0;
    else if (op == on[Operators::LogicalAnd]) return -1;
    else if (op == on[Operators::LogicalOr]) return -2;
    return -3;
}

//Parser
class Parser {
public:
    Parser(const std::vector<Token>& tokens, const std::string& moduleName) : tokens(tokens), moduleName(moduleName) {}
    MemoryPtr<ModuleNode> parseModule(); // main parsing
    void printModule();
private:
    std::vector<Token> tokens;
    size_t pos = 0;
    std::string moduleName = "";

    Token curToken() { 
        if (pos >= tokens.size()) { Token token = Token{TokenType::EndOfFile, ""}; return token; }
        return tokens[pos]; 
    };
    Token lookBack() {
        return tokens[pos--];
    }
    Token next() { 
        if (pos >= tokens.size()) return Token{TokenType::EndOfFile, ""};
        return tokens[pos++]; 
    };
    Token lookupNext() {
        if (pos + 1 >= tokens.size()) return Token{TokenType::EndOfFile, ""};
        return tokens[pos + 1];
    }
    bool match(TokenType type, const std::string& value = "") {
        if (curToken().type != type) return false;
        if (!value.empty() && curToken().value != value) return false;
        return true;
    };
    bool isAtEnd() { return pos >= tokens.size() || curToken().type == TokenType::EndOfFile; }

    // Expression parsing
    MemoryPtr<ASTNode> parsePrimary();
    MemoryPtr<ASTNode> parseExpression();
    MemoryPtr<ASTNode> parseBinary(int prevPrecedence = 0);
    MemoryPtr<UnaryOperationNode> parseUnary(const std::string& op);

    // Statement parsing
    MemoryPtr<ASTNode> parseStatement();
    MemoryPtr<AssignmentNode> parseAssignment();
    MemoryPtr<ReturnStatementNode> parseReturn();

    // Control flow
    MemoryPtr<IfNode> parseIf();
    MemoryPtr<SwitchNode> parseSwitch();
    MemoryPtr<TryCatchNode> parseTryCatch();
    MemoryPtr<ForLoopNode> parseFor();
    MemoryPtr<WhileLoopNode> parseWhile();

    // Declarations
    MemoryPtr<FunctionNode> parseFunction(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<ClassNode> parseClass(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<BlockNode> parseBlock();

    MemoryPtr<EnumNode> parseEnum(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<InterfaceNode> parseInterface(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers);

    // Imports, decorators, modifiers, preprocessor
    MemoryPtr<ImportNode> parseImport();
    MemoryPtr<ASTNode> parseDecorator(std::vector<MemoryPtr<CallExpressionNode>> decorators = {}, std::vector<MemoryPtr<ModifierNode>> modifiers = {}, bool isCall=false);
    std::vector<MemoryPtr<CallExpressionNode>> parseDecoratorCalls();
    std::vector<MemoryPtr<ModifierNode>> parseModifiers();
    MemoryPtr<ASTNode> parsePreprocessor();

    // Helper functions
    MemoryPtr<ASTNode> parseBlockorStatement();
    bool isNextLine();
};