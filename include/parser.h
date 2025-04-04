#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "lexer.h"
#include "nodes.h"

enum class LoopType {
    For, Foreach, While
};

inline int getOperatorPrecedence(const std::string& op) {
    if (op == "=") return 0;  // НИЖЕ всех
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "==") return 3;
    if (op == "!=") return 3;
    if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
    if (op == "+" || op == "-") return 5;
    if (op == "*" || op == "/" || op == "%") return 6;
    if (op == "^") return 7;
    return -1; // unknown operator
}

class Parser {
private:
    const std::vector<Token>& tokens;
    size_t pos;

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}


    // 🔹 Основные вспомогательные методы
    Token currentToken() const { return tokens[pos]; }
    void nextToken() { if (!isEnd()) pos++; }
    bool isEnd() const { return pos >= tokens.size(); }

    // Helpful functions to make code look better
    std::unique_ptr<Expression> makeExpression(ExpressionType type, std::string value);
    std::unique_ptr<Expression> makeUnaryOp(const std::string& op, std::unique_ptr<Expression> operand);

    // 🔹 Парсинг выражений и операторов
    std::unique_ptr<Expression> parseExpression(int precedence = 0);
    std::unique_ptr<Expression> parsePrimaryExpression();
    //std::unique_ptr<Expression> parseAssignment();
    //std::unique_ptr<Expression> parseReturnStatement();

    // 🔹 Парсинг управляющих конструкций
    //std::unique_ptr<Expression> parseIfElseStatement();
    //std::unique_ptr<Expression> parseLoop(LoopType type);
    //std::unique_ptr<Expression> parseSwitchCase();
    //std::unique_ptr<Expression> parseTryCatch();

    // 🔹 Парсинг функций и структур
    //std::unique_ptr<Expression> parseFunction();
    //std::unique_ptr<Expression> parseClassOrStruct();
    //std::unique_ptr<Expression> parseBlock();

    // 🔹 Парсинг дополнительных фич
    //std::unique_ptr<Expression> parseImports();
    //std::unique_ptr<Expression> parseMacros();
    //std::unique_ptr<Expression> parseDecorators();

    // 🔹 Парсинг всей программы
    void printAST(const std::unique_ptr<Expression>& expr, const std::string& indent, bool isLast);
    std::unique_ptr<Expression> parseProgram();
};

#endif
