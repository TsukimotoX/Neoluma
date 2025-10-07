#pragma once 

#include <string>
#include <vector>
#include "../token.hpp"

class Lexer {
public: 
    std::vector<Token> tokenize(const std::string& source);
    void printTokens(const std::string& filename) const; // Debug command to check tokens correctness
private:
    std::string source;
    size_t pos = 0;
    int line = 1;
    int column = 1;

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