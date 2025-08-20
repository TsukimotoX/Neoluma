#pragma once

#include <optional>
#include <string>
#include <unordered_map>

enum TokenType {
    NullLiteral, Identifier, //Combined Variable, function and class names, and etc.
    Keyword, Decorator, Preprocessor, Delimeter, Operator,
    AssignmentArrow, Question, Unknown, EndOfFile,

    Type, // Represents a type of tokens below
    Integer, Float, Number, Boolean, String, 
    Array, Set, Dictionary, Void, Result,
};

struct Token {
    TokenType type;
    std::string value;
    std::string toString() const;
};

enum class Keywords {
    Function, Class, Enum, Interface, Namespace,
    If, Else,
    For, While, Break, Continue,
    Switch, Case, Default,
    Try, Catch, Throw,
    Async, Await,
    Yield, Return,
    Static, Decorator, Const,
    As, With, In, Lambda,
    Debug, Public, Protected, Private
};

enum class Operators {
    Add, Subtract, Multiply, Divide, Modulo, Power,
    Equal, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual,
    LogicalAnd, LogicalOr, LogicalNot,
    Assign, Nullable,
    AddAssign, SubAssign, MulAssign, DivAssign, ModAssign, PowerAssign
};
enum class Decorators {
    Float, Entry, Unsafe, Comptime, Override
};
enum class Preprocessors {
    Import, From, Use, As, Unsafe, Baremetal, Float, Macro
};
enum class Delimeters {
    LeftParen, RightParen, LeftBracket, RightBracket, Semicolon, Comma, Dot, LeftSquareBracket, RightSquareBracket
};

extern std::unordered_map<std::string, Keywords> keywordMap;
extern std::unordered_map<std::string, TokenType> typesMap;
extern std::unordered_map<std::string, Operators> operatorMap;
extern std::unordered_map<std::string, Decorators> decoratorMap;
extern std::unordered_map<std::string, Preprocessors> preprocessorMap;
extern std::unordered_map<std::string, Delimeters> delimeterMap;