#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "lexer.h"
#include "nodes.h"

class Parser {
private:
    const std::vector<Token>& tokens;
    size_t curPos;

    Token currentToken();
    void nextToken();

    std::unique_ptr<ASTNode> parsePrimary();

    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseFunction();
    std::unique_ptr<ASTNode> parseCondition();
    std::unique_ptr<ASTNode> parseStatements();
    std::unique_ptr<ASTNode> parseClass();
    std::unique_ptr<ASTNode> parseStruct();
    std::unique_ptr<ASTNode> parseEnum();
    std::unique_ptr<ASTNode> parseTryCatch();
    std::unique_ptr<ASTNode> parseAccessModifiers();
    std::unique_ptr<ASTNode> parseHashtags();
    std::unique_ptr<ASTNode> parseImport();
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), curPos(0) {}
    std::unique_ptr<ASTNode> parse();
};

#endif