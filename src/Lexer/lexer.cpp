#include "lexer.h"
#include "token.h"

#include <iostream>
#include <vector>
#include <string>

Lexer::Lexer(const std::string& source): source(source), pos(0) {}

char Lexer::curChar() const {
    return isEnd() ? '\0' : source[pos];
}
char Lexer::move() {
    return isEnd() ? '\0' : source[pos++];
}

bool Lexer::match(char expected) {
    if (isEnd() || source[pos] != expected) return false;
    pos++;
    return true;
}

bool Lexer::isEnd() const {
    return pos >= source.size();
}

std::vector<Token> Lexer::tokenize() {
    tokens.clear();

    while (!isEnd()) {
        char c = curChar();

        if (isspace(c)) {
            move();
        }
        else if (isalpha(c) || c == '_') {
            parseIdentifierOrKeyword();
        }
        else if (isdigit(c)) {
            parseNumber();
        }
        else if (c == '"') {
            parseString();
        }
        else if (/* isOperatorChar(c) */) {
            parseOperator();
        }
        else if (/* isDelimeterChar(c) */) {
            parseDelimeter();
        }
        else {
            std::string unknown(1, move());
            tokens.push_back(Token{TokenType::Unknown, unknown});
        }
    }

    tokens.push_back(Token{TokenType::EndOfFile, ""});
    return tokens;
}
