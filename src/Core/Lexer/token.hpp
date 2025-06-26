#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include "../../HelperFunctions.hpp"

enum TokenType {
    NullLiteral,
    Identifier, //Combined Variable, function and class names, and etc.
    Keyword,
    Decorator,
    Preprocessor,
    Delimeter,
    Operator,
    AssignmentArrow,
    Question,
    Unknown,
    EndOfFile,

    Type, // Represents a type of tokens below
    Integer,
    Float,
    Number,
    Boolean,
    String,
    Array,
    Set,
    Dictionary,
    Void,
    Result,
};

struct Token {
    TokenType type;
    string value;
    string toString() const;
};

enum class Keywords {
    Function, Class, Enum, Interface, Namespace,
    If, Else,
    For, While, Break, Continue,
    Switch, Case, Default,
    Try, Catch, Throw,
    Async, Await,
    Yield, Return,
    Static, Decorator,
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

extern std::unordered_map<string, Keywords> keywordMap;
extern std::unordered_map<string, TokenType> typesMap;
extern std::unordered_map<string, Operators> operatorMap;
extern std::unordered_map<string, Decorators> decoratorMap;
extern std::unordered_map<string, Preprocessors> preprocessorMap;
extern std::unordered_map<string, Delimeters> delimeterMap;

#endif
