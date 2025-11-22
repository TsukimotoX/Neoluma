#include "lexer.hpp"
#include "../token.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <print>

// ==== Main ====
std::vector<Token> Lexer::tokenize(const std::string& filePath, const std::string& source) {
    tokens.clear();
    this->source = source;
    this->filePath = filePath;
    pos = 0;

    while (!isAtEnd()) {
        char c = curChar();

        if (c=='\n') {
            int sl = line; int sc = column;
            move();
            tokens.push_back(Token{TokenType::Delimeter, "\\n", filePath, sl, sc});
        }
        else if (isspace(c)) move();
        else if (isalpha(c) || c == '_') parseIK();
        else if (isdigit(c)) parseNumber();
        else if (c == '"') parseString();
        else if (c == '/' && pos + 1 < this->source.size() && (this->source[pos+1] == '/' || this->source[pos+1] == '*')) skipComment();
        else if (std::string("+-*/%^=<>!&|?~").find(c) != std::string::npos) parseOperator();
        else if (std::string("(){};:,.[]").find(c) != std::string::npos) parseDelimeter();
        else if (c == '#') parsePreprocessor();
        else if (c == '@') parseDecorator();
        else {
            std::string unknown(1, move());
            std::println(std::cerr, "[Neoluma/Lexer] Unknown character: '{}'", unknown);
            tokens.push_back(Token{TokenType::Unknown, unknown, filePath, line, column});
        }
    }

    tokens.push_back(Token{TokenType::EndOfFile, "", filePath, line, column});
    return tokens;
}

// ==== Helpers ====

char Lexer::curChar() const { return isAtEnd() ? '\0' : source[pos]; }
char Lexer::move() { 
    char c = source[pos++];

    if (c=='\n') {
        line++;
        column = 1;
    } else column++;

    return c;
}
bool Lexer::match(char expected) {
    if (isAtEnd() || source[pos] != expected) return false;
    pos++;
    return true;
}
bool Lexer::isAtEnd() const { return pos >= source.size(); }

// ==== Parsing functions ====
void Lexer::parseIK() {
    int sl = line; int sc = column;
    std::string word;
    
    while (!isAtEnd() && (isalnum(curChar()) || curChar() == '_')) word += move();

    auto km = getKeywordMap();
    auto om = getOperatorMap();
    
    if (km.find(word) != km.end()) tokens.push_back(Token{TokenType::Keyword, word, filePath, sl, sc});
    else if (word == "null") tokens.push_back(Token{TokenType::Null, word, filePath, sl, sc});
    else if (om.find(word) != om.end()) tokens.push_back(Token{TokenType::Operator, word, filePath, sl, sc});
    else tokens.push_back(Token{TokenType::Identifier, word, filePath, sl, sc});
}
void Lexer::parseNumber() {
    int sl = line; int sc = column;
    std::string number;

    while (!isAtEnd() && isdigit(curChar())) number += move();
    
    if (!isAtEnd() && curChar() == '.'){
        number += move();
        while (!isAtEnd() && (isdigit(curChar()) || curChar() == 'e')) number += move();
    }

    tokens.push_back(Token{TokenType::Number, number, filePath, sl, sc});
}
void Lexer::parseString() {
    int sl = line; int sc = column;
    /* hardest thing to make. strings in neoluma can be multiline,
       have f-strings (variables inside ${}) inside them and support \n \t or anything i forgor.
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
                    std::println(std::cerr, "[Neoluma/Lexer] Warning: unknown escape character '\\{}'", c);
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
    tokens.push_back(Token{TokenType::String, value, filePath, sl, sc});
}
void Lexer::parseOperator() {
    int sl = line; int sc = column;
    std::string op;
    op += move();

    auto om = getOperatorMap();

    if (!isAtEnd()) {
        std::string twoChar = op + curChar();
        if (om.find(twoChar) != om.end()) op += move();

        tokens.push_back(Token{TokenType::Operator, op, filePath, sl, sc});
    }
}
void Lexer::parseDelimeter() {
    int sl = line; int sc = column;
    std::string delimeter;
    delimeter += move();

    auto dm = getDelimeterMap();
    
    if (dm.find(delimeter) != dm.end()) tokens.push_back(Token{TokenType::Delimeter, delimeter, filePath, sl, sc});
    else {
        std::println(std::cerr, "[Neoluma/Lexer] Unknown delimeter: '{}'", delimeter);
        tokens.push_back(Token{TokenType::Unknown, delimeter, filePath, sl, sc});
    }
}
void Lexer::parsePreprocessor() {
    int sl = line; int sc = column;
    move();
    std::string word;

    while (!isAtEnd() && isalpha(curChar())) word += move();

    auto pm = getPreprocessorMap();

    if (pm.find(word) != pm.end()) tokens.push_back(Token{TokenType::Preprocessor, word, filePath, sl, sc});
    else {
        std::println(std::cerr, "[Neoluma/Lexer] Unknown preprocessor: #{}", word);
        tokens.push_back(Token{TokenType::Unknown, "#" + word, filePath, sl, sc});
    }
}
void Lexer::parseDecorator() {
    int sl = line; int sc = column;
    move();
    std::string word;

    while (!isAtEnd() && isalpha(curChar())) word += move();

    tokens.push_back(Token{TokenType::Decorator, word, filePath, sl, sc});
}
void Lexer::skipComment() {
    int sl = line; int sc = column;

    // apparently if i don't do this check im gonna regret it
    if (isAtEnd()) {
        move();
        // single '/' at EOF - treat as operator
        tokens.push_back(Token{TokenType::Operator, "/", filePath, sl, sc});
        return;
    }

    // Single-line comment
    if (source[pos] == '/' && source[pos+1] == '/') {
        move(); // '/'
        move(); // '/'
        while (!isAtEnd() && curChar() != '\n') move();
        move(); // consume newline
        return;
    }

    // Block comment '/* ... */'
    if (source[pos] == '/' && source[pos+1] == '*') {
        move(); // '/'
        move(); // '*'
        while (!isAtEnd()) {
            if (curChar() == '*' && (pos + 1 < source.size()) && source[pos + 1] == '/') {
                move(); // '*'
                move(); // '/'
                return;
            }
            move(); // \n_terminator3000
        }
        // Unterminated block comment
        std::println(std::cerr, "[Neoluma/Lexer] Unterminated comment at {}:{}", line, column);
        return;
    }

    // Not actually a comment sequence; treat as operator
    move();
    tokens.push_back(Token{TokenType::Operator, "/", filePath, sl, sc});
}

void Lexer::printTokens(const std::string& filename) const {
    std::println("=== Lexer Output ({}) ===", filename);
    for (const auto& token : tokens) {
        //if (token.type == TokenType::Delimeter && token.value == "\\n") std::cout << "";
        //else
        std::cout << token.toStr();
    }
    std::println("====================");
}