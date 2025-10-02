#pragma once

#include "../Lexer/lexer.hpp"
#include "nodes.hpp"
#include "../Lexer/token.hpp"
#include <iostream>

// to make math order
int getOperatorPrecedence(const std::string& op){
    if (op == "^") return 5;
    else if (op == "*" || op == "/" || op == "%") return 4;
    else if (op == "+" || op == "-") return 3;
    else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") return 2;
    else if (op == "&&") return 1;
    else if (op == "||") return 0;
    return -1;
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
    std::string moduleName;

    Token& curToken() { 
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
    MemoryPtr<FunctionNode> parseFunction(std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<ClassNode> parseClass(std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<BlockNode> parseBlock();

    MemoryPtr<EnumNode> parseEnum(std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<InterfaceNode> parseInterface(std::vector<MemoryPtr<ModifierNode>> modifiers);

    // Imports, decorators, modifiers, preprocessor
    MemoryPtr<ImportNode> parseImport();
    MemoryPtr<DecoratorNode> parseDecorator(std::vector<MemoryPtr<ModifierNode>> modifiers);
    std::vector<MemoryPtr<ModifierNode>> parseModifier();
    MemoryPtr<PreprocessorDirectiveNode> parsePreprocessor();

    // Helper functions
    MemoryPtr<ASTNode> parseBlockorStatement();
    bool isNextLine();
};