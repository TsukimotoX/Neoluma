#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

class Lexer {
public: 
    std::vector<Token> tokenize(const std::string& source);
    void printTokens(std::string file) const;
private:
    std::string source;
    size_t pos = 0;

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