#pragma once

#include "../Lexer/lexer.hpp"
#include "../nodes.hpp"
#include "../token.hpp"
#include <iostream>

struct Compiler; // forward declaration cuz f##k c++ ig

// to make math order
int getOperatorPrecedence(const std::string& op);
bool isAssignmentOperator(const std::string& op);

//Parser
struct Parser {
    void parseModule(const std::vector<Token>& tok, const std::string& moduleName); // main parsing
    void printModule(int indentation = 0);

    // Compiler access through a pointer
    Compiler* compiler = nullptr;
    void setCompiler(Compiler* comp) { this->compiler = comp; }

    MemoryPtr<ModuleNode> moduleSource = nullptr;
    std::vector<Token> tokens;
    size_t pos = 0;
    std::string moduleName = "";

    Token curToken() { 
        if (pos >= tokens.size()) { Token token = Token{TokenType::EndOfFile, ""}; return token; }
        return tokens[pos]; 
    };
    Token lookBack() {
        if (pos + 1 >= tokens.size()) return Token{TokenType::EndOfFile, ""};
        return tokens[pos - 1];
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
    MemoryPtr<RawTypeNode> parseType();

    // Statement parsing
    MemoryPtr<ASTNode> parseStatement();
    MemoryPtr<DeclarationNode> parseDeclaration(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers);
    MemoryPtr<AssignmentNode> parseAssignment();

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
    MemoryPtr<ASTNode> parseDecorator(std::vector<MemoryPtr<CallExpressionNode>> decorators = {}, std::vector<MemoryPtr<ModifierNode>> modifiers = {}, bool isCall=false);
    std::vector<MemoryPtr<CallExpressionNode>> parseDecoratorCalls();
    std::vector<MemoryPtr<ModifierNode>> parseModifiers();
    MemoryPtr<ASTNode> parsePreprocessor();

    // Helper functions
    MemoryPtr<ASTNode> parseBlockorStatement();
    bool isNextLine();
    // Lookahead helper to check if upcoming tokens form an assignable lvalue followed by an assignment operator
    bool isAssignableAhead(size_t offset = 0);

    //Token statementAnchor; // statementAnchor allows errorManager to refer to the statement itself instead of decorators and modifiers for it. Example: function, class, etc.
};