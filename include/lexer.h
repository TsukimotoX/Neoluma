#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

enum TokenType {
    NUMBER, IDENTIFIER, OPERATOR, KEYWORD, UNKNOWN, DELIMITER, END_OF_FILE,
};

struct Token {
    TokenType type;
    std::string value;
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
    Add, Subtract, Multiply, Divide, Modulo, Power, // Arithmetic operators
    Equal, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual, // Comparison operators
    LogicalAnd, LogicalOr, LogicalNot, // Logical operators
};

extern std::unordered_map<std::string, Operators> operatorMap;
extern std::unordered_map<std::string, Keyword> keywordMap;
extern std::unordered_set<char> delimiters;

class Lexer {
private:
    std::string code;
    size_t pos;

    Token nextToken();
    Token tokenizeNumber();
    Token tokenizeIdentifier();
    Token tokenizeOperator();
    Token tokenizeUnknown();
public:
    explicit Lexer(const std::string& code) : code(code), pos(0) {}
    std::vector<Token> tokenize();
};

#endif
