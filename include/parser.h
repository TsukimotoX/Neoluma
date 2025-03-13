#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

struct ASTNode {
    TokenType type;
    std::string value;
    ASTNode* left;
    ASTNode* right;
};

#endif 