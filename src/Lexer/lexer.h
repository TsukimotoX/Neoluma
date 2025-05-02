#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source);

    std::vector<Token> tokenize();
private:
    std::string source;
    size_t pos = 0;
    std::vector<Token> tokens;
    
    char curChar() const;
    char move();
    bool match(char expected);
    bool isEnd() const;

    void parseIdentifierOrKeyword();
    void parseNumber();
    void parseString();
    void parseOperator();
    void parseDelimeter();

    bool isKeyword(const std::string& word) const;
    bool isBuiltinType(const std::string& word, BuiltinType& outType) const;
};

#endif