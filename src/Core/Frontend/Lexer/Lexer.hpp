#pragma once 

#include <string>
#include <vector>
#include "../Token.hpp"
#include "Core/Extras/ErrorManager/ErrorManager.hpp"


struct Lexer {
    std::vector<Token> tokenize(const std::string& filePath, const std::string& source);
    void printTokens(const std::string& filename) const; // Debug command to check tokens correctness
    std::vector<Token> tokens;

    // ErrorManager is used to report errors
    ErrorManager* errorManager = nullptr;

    std::string source;
    size_t pos = 0;
    std::string filePath;
    int line = 1;
    int column = 1;
    std::unordered_map<std::string, Operators> om = getOperatorMap();
    std::unordered_map<std::string, Delimeters> dm = getDelimeterMap();
    std::unordered_map<std::string, Preprocessors> pm = getPreprocessorMap();
    std::unordered_map<std::string, Keywords> km = getKeywordMap();

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
