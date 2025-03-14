#include "lexer.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

std::unordered_map<std::string, Operators> operatorMap = {
    {"+", Operators::Add}, {"-", Operators::Subtract}, {"*", Operators::Multiply}, {"/", Operators::Divide}, {"%", Operators::Modulo}, {"^", Operators::Power},
    {"==", Operators::Equal}, {"!=", Operators::NotEqual}, {"<", Operators::LessThan}, {">", Operators::GreaterThan}, {"<=", Operators::LessThanOrEqual}, {">=", Operators::GreaterThanOrEqual},
    {"&&", Operators::LogicalAnd}, {"||", Operators::LogicalOr}, {"!", Operators::LogicalNot},
    {"and", Operators::LogicalAnd}, {"or", Operators::LogicalOr}, {"not", Operators::LogicalNot},
    {"!", Operators::Not}, {"-", Operators::Negate},
};

std::unordered_map<std::string, Keyword> keywordMap = {
    {"function", Keyword::Function}, {"class", Keyword::Class}, {"struct", Keyword::Struct}, {"enum", Keyword::Enum}, {"interface", Keyword::Interface}, {"#namespace", Keyword::Namespace},
    {"number", Keyword::Number}, {"int", Keyword::Int}, {"float", Keyword::Float}, {"bool", Keyword::Bool}, {"string", Keyword::String}, {"array", Keyword::Array}, {"dictionary", Keyword::Dictionary}, {"set", Keyword::Set},
    {"true", Keyword::True}, {"false", Keyword::False}, {"null", Keyword::Null},
    {"if", Keyword::If}, {"else", Keyword::Else},
    {"for", Keyword::For}, {"foreach", Keyword::Foreach}, {"while", Keyword::While}, {"break", Keyword::Break}, {"continue", Keyword::Continue},
    {"switch", Keyword::Switch}, {"case", Keyword::Case}, {"default", Keyword::Default},
    {"try", Keyword::Try}, {"catch", Keyword::Catch}, {"throw", Keyword::Throw},
    {"#import", Keyword::Import}, {"#use", Keyword::Use},
    {"async", Keyword::Async}, {"await", Keyword::Await},
    {"yield", Keyword::Yield}, {"return", Keyword::Return},
    {"@public", Keyword::Public}, {"@private", Keyword::Private}, {"@protected", Keyword::Protected},
    {"static", Keyword::Static}, {"override", Keyword::Override},
    {"#macro", Keyword::Macro}, {"decorator", Keyword::Decorator},
    {"as", Keyword::As}, {"with", Keyword::With},
};

std::unordered_set<std::string> delimiters = {"(", ")", "{", "}", ";", ","};

// Private stuff
Token Lexer::nextToken(){
    // If we've reached the end of the code, return the end of file token
    if (pos >= code.length()) {
        return {TokenType::END_OF_FILE, ""};
    }

    char curChar = code[pos]; // Current character

    // Move on, it's just a space, do you really care?
    if (std::isspace(curChar)) {
        pos++;
        return nextToken();
    }

    if (std::isdigit(curChar)) {
        return tokenizeNumber();
    }

    if (std::isalpha(curChar)) {
        return tokenizeIdentifier();
    }

    if (operatorMap.find(std::string(1, curChar)) != operatorMap.end()) {
        return tokenizeOperator();
    }

    if (delimiters.find(std::string(1, curChar)) != delimiters.end()) {
        pos++;
        return {DELIMITER, std::string(1, curChar)};
    }

    return tokenizeUnknown();
}

Token Lexer::tokenizeNumber(){ // Tokenize a number
    std::string number;
    while (pos < code.length() && (std::isdigit(code[pos]) || code[pos] == '.')) {
        number += code[pos++];
    }
    return {NUMBER, number};
}

Token Lexer::tokenizeIdentifier(){ // Tokenize an identifier
    std::string identifier;
    while (pos < code.length() && (std::isalnum(code[pos]) || code[pos] == '_')) {
        identifier += code[pos++];
    }

    if (keywordMap.find(identifier) != keywordMap.end()) {
        return {KEYWORD, identifier};
    }
    return {IDENTIFIER, identifier};
}

Token Lexer::tokenizeOperator(){ // Tokenize an operator
    std::string operatorPos = std::string(1, code[pos]); // Get the operator as a string

    // Double operator
    if (pos + 1 < code.length()){
        std::string doubleOperator = operatorPos + code[pos+1];
        if (operatorMap.find(doubleOperator) != operatorMap.end()) {
            pos += 2; // Move two characters ahead
            return {OPERATOR, doubleOperator};
        }
    }

    // Single operator
    if (operatorMap.find(operatorPos) != operatorMap.end()) {
        pos++; // Move one character ahead
        return {OPERATOR, operatorPos};
    }

    return {UNKNOWN, operatorPos};
};

Token Lexer::tokenizeUnknown() { // Tokenize an unknown character
    std::string value(1, code[pos++]);
    return Token{UNKNOWN, value};
}

// Public stuff

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos < code.length()) {
        Token token = nextToken();
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
        if (token.type == TokenType::UNKNOWN) {
            std::cerr << "Unknown token: " << token.value << ". Skipping." << std::endl;
            continue;
        }
        tokens.push_back(token);
    }
    return tokens;
}
