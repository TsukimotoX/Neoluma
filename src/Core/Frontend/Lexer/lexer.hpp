#pragma once 

#include <string>
#include <vector>
#include "../token.hpp"

struct Compiler; // forward declaration cuz f##k c++ ig

struct Lexer {
    std::vector<Token> tokenize(const std::string& filePath, const std::string& source);
    void printTokens(const std::string& filename) const; // Debug command to check tokens correctness
    std::vector<Token> tokens;

    // Compiler access through a pointer
    Compiler* compiler = nullptr;
    void setCompiler(Compiler* comp) { this->compiler = comp; }

    std::string source;
    size_t pos = 0;
    std::string filePath;
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
};
