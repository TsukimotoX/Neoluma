#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

enum TokenType {
    NUMBER, VARIABLE, OPERATOR, KEYWORD, UNKNOWN, DELIMITER, END_OF_FILE,
};

struct Token {
    TokenType type;
    std::string value;

    std::string toString() const {
        std::string typeStr;
        switch (type) {
            case TokenType::NUMBER: typeStr = "NUMBER"; break;
            case TokenType::VARIABLE: typeStr = "VARIABLE"; break;
            case TokenType::OPERATOR: typeStr = "OPERATOR"; break;
            case TokenType::KEYWORD: typeStr = "KEYWORD"; break;
            case TokenType::UNKNOWN: typeStr = "UNKNOWN"; break;
            case TokenType::DELIMITER: typeStr = "DELIMITER"; break;
            case TokenType::END_OF_FILE: typeStr = "END_OF_FILE"; break;
        }
        return "[" + typeStr + "(\"" + value + "\")]";
    }
};

enum class Keyword {
    Function, Class, Struct, Enum, Interface, Namespace, // Basic keywords
    Number, Int, Float, Bool, String, Array, Dictionary, Set, // Data types
    True, False, Null, // Logical values keywords
    If, Else, // Conditional statements
    For, Foreach, While, Break, Continue, // Loops and breaking statements
    Switch, Case, Default, // Switch cases
    Try, Catch, Throw, // Exception handling
    Import, Use, // Module importing
    Async, Await, // Asynchronous execution
    Yield, Return, // Keywords for result returning
    Public, Private, Protected, // Keywords for access modifiers
    Static, Override, // Keywords for modifiers
    Macro, Decorator, // Keywords for macros and decorators
    As, With, // Other
};

enum class Operators {
    Add, Subtract, Multiply, Divide, Modulo, Power,
    Equal, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual,
    LogicalAnd, LogicalOr, LogicalNot,
    Not, Negate, Assignment,
};

extern std::unordered_map<std::string, Operators> operatorMap;
extern std::unordered_map<std::string, Keyword> keywordMap;
extern std::unordered_set<std::string> delimiters;

class Lexer {
private:
    std::string code;
    size_t pos;

    Token nextToken();
    Token tokenizeNumber();
    Token tokenizeVariable();
    Token tokenizeOperator();
    Token tokenizeUnknown();
public:
    explicit Lexer(const std::string& code) : code(code), pos(0) {}
    std::vector<Token> tokenize();
};

#endif
