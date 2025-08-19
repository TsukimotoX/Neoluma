#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

class Lexer {
public: 
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();
    void printTokens() const;
private:
    std::string source;
    size_t pos;

    // Helpers
    char curChar() const;
    char move();
    bool match(char expected);
    bool isAtEnd() const;

    // Parsers
    void parseIK(); // Parse identifier or keyword
    void parseNumber();
    void parseString();
    void parseOperator();
    void parseDelimeter();
    void parsePreprocessor();
    void parseDecorator();
    void skipComment();

    std::vector<Token> tokens;
};

#endif