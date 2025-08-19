#include "lexer.hpp"
#include "token.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <print>

Lexer::Lexer(const std::string& source) : source(source), pos(0) {}

// ==== Main ====
std::vector<Token> Lexer::tokenize() {
    tokens.clear();

    while (!isAtEnd()) {
        char c = curChar();

        if (isspace(c)) move();
        else if (isalpha(c) || c == '_') parseIK();
        else if (isdigit(c)) parseNumber();
        else if (c == '"') parseString();
        else if (std::string("+-*/%^=<>!&|:").find(c) != std::string::npos) parseOperator();
        else if (std::string("(){};,.[\\]").find(c) != std::string::npos) parseDelimeter();
        else if (c == '#') parsePreprocessor();
        else if (c == '@') parseDecorator();
        else if (c == '/' && (source[pos+1] == '/' || source[pos+1] == '*')) skipComment();
        else {
            std::string unknown(1, move());
            std::println(std::cerr, "[Lexer] Unknown character: '{}'", unknown);
            tokens.push_back(Token{TokenType::Unknown, unknown});
        }
    }

    tokens.push_back(Token{TokenType::EndOfFile, ""});
    return tokens;
}

// ==== Helpers ====

char Lexer::curChar() const { return isAtEnd() ? '\0' : source[pos]; }
char Lexer::move() { return isAtEnd() ? '\0' : source[pos++]; }
bool Lexer::match(char expected) {
    if (isAtEnd() || source[pos] != expected) return false;
    pos++;
    return true;
}
bool Lexer::isAtEnd() const { return pos >= source.size(); }

// ==== Parsing functions ====
void Lexer::parseIK() {
    std::string word;
    
    while (!isAtEnd() && (isalnum(curChar()) || curChar() == '_')) word += move();
    
    if (keywordMap.find(word) != keywordMap.end()) tokens.push_back(Token{TokenType::Keyword, word});
    else if (typesMap.find(word) != typesMap.end()) tokens.push_back(Token{typesMap[word], word});
    else if (word == "true" || word == "false") tokens.push_back(Token{TokenType::Boolean, word});
    else if (word == "null") tokens.push_back(Token{TokenType::NullLiteral, word});
    else if (operatorMap.find(word) != operatorMap.end()) tokens.push_back(Token{TokenType::Operator, word});
    else tokens.push_back(Token{TokenType::Identifier, word});
}
void Lexer::parseNumber() {
    //TODO:: Add exponents (maybe)
    std::string number;

    while (!isAtEnd() && isdigit(curChar())) number += move();
    
    if (!isAtEnd() && curChar() == '.'){
        number += move();
        while (!isAtEnd() && isdigit(curChar())) number += move();
    }

    tokens.push_back(Token{TokenType::Number, number});
}
void Lexer::parseString() {
    /* hardest thing to make. strings in neoluma can be multiline,
       have f-strings (variables inside {}) inside them and support \n \t or anything i forgor.
    */

    move();
    std::string value;
    bool esc = false; // multipurpose \n stuff checker..

    while (!isAtEnd()) {
        char c = curChar();
        if (esc) {
            switch (c) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default:
                    std::println(std::cerr, "[Lexer] Warning: unknown escape character '\\{}'", c);
                    value += c;
                    break;
            }
            esc = false;
        }
        else if (c == '\\') esc = true;
        else if (c == '"') break;
        else value += c;

        move();
    }

    move();
    tokens.push_back(Token{TokenType::String, value});
}
void Lexer::parseOperator() {
    std::string op;
    op += move();

    if (!isAtEnd()) {
        std::string twoChar = op + curChar();
        if (operatorMap.find(twoChar) != operatorMap.end()) op += move();

        if (op == "=>") tokens.push_back(Token{TokenType::AssignmentArrow, op});
        else if (op == "?") tokens.push_back(Token{TokenType::Question, op});
        else tokens.push_back(Token{TokenType::Operator, op});
    }
}
void Lexer::parseDelimeter() {
    std::string delimeter;
    delimeter += move();
    
    if (delimeterMap.find(delimeter) != delimeterMap.end()) tokens.push_back(Token{TokenType::Delimeter, delimeter});
    else {
        std::println(std::cerr, "[Lexer] Unknown delimeter: '{}'", delimeter);
        tokens.push_back(Token{TokenType::Unknown, delimeter});
    }
}
void Lexer::parsePreprocessor() {
    move();
    std::string word;

    while (!isAtEnd() && isalpha(curChar())) word += move();

    if (preprocessorMap.find(word) != preprocessorMap.end()) tokens.push_back(Token{TokenType::Preprocessor, word});
    else {
        std::println(std::cerr, "[Lexer] Unknown preprocessor: #{}", word);
        tokens.push_back(Token{TokenType::Unknown, "#" + word});
    }
}
void Lexer::parseDecorator() {
    move();
    std::string word;

    while (!isAtEnd() && isalpha(curChar())) word += move();

    if (decoratorMap.find(word) != decoratorMap.end()) tokens.push_back(Token{TokenType::Decorator, word});
    else {
        std::println(std::cerr, "[Lexer] Unknown decorator: @{}", word);
        tokens.push_back(Token{TokenType::Unknown, "@" + word});
    }
}
void Lexer::skipComment() {
    // yes i can actually read this monstrocity
    if (match('/')) while (!isAtEnd() && curChar() != '\n') move();
    else if (match('*')) {
        // multiline
        while (!isAtEnd()) {
            if (curChar() == '*' && (pos + 1 < source.size()) && source[pos + 1] == '/') {
                move(); // '*'
                move(); // '/'
                break;
            }
            move();
        }
    }
    // not comment
    else tokens.push_back(Token{TokenType::Operator, "/"});
}

void Lexer::printTokens() const {
    std::println("=== Lexer Output ===");
    for (const auto& token : tokens) {
        switch (token.type) {
            case TokenType::Identifier: std::cout << "[Identifier]"; break;
            case TokenType::Keyword:    std::cout << "[Keyword]   "; break;
            case TokenType::Number:     std::cout << "[Number]    "; break;
            case TokenType::String:     std::cout << "[String]    "; break;
            case TokenType::Operator:   std::cout << "[Operator]  "; break;
            case TokenType::Delimeter:  std::cout << "[Delimeter] "; break;
            case TokenType::Preprocessor: std::cout << "[Preproc]   "; break;
            case TokenType::Decorator:  std::cout << "[Decorator] "; break;
            case TokenType::EndOfFile:        std::cout << "[EOF]       "; break;
            default: std::cout << "[Unknown]"; break;
        }

        std::cout << " -> \"" << token.value << "\"";

        std::cout << "\n";
    }
    std::println("====================");
}