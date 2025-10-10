#pragma once

#include <optional>
#include <string>
#include <unordered_map>

enum class TokenType { 
    Keyword, Identifier, Number, Operator, String, Delimeter, Unknown, Decorator, Preprocessor, EndOfFile, Null,
};

struct Token {
    TokenType type;
    std::string value;
    std::string toStr() const;
    std::string filePath;
    int line;
    int column;
};

// Enums

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
    Debug, Public, Protected, Private, Override
};
enum class Operators {
    Add, Subtract, Multiply, Divide, Modulo, Power,
    Equal, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual,
    LogicalAnd, LogicalOr, LogicalNot,
    Assign, Nullable,
    AddAssign, SubAssign, MulAssign, DivAssign, ModAssign, PowerAssign,
    AssignmentArrow,
    BitwiseAnd, BitwiseOr, BitwiseXOr, BitwiseNot, BitwiseLeftShift, BitwiseRightShift,
    //BitwiseUnsignedRightShift (from JS, not included for now because argued about)
};
enum class Decorators {
    Float, Entry, Unsafe, Comptime
};
enum class Preprocessors {
    Import, As, Unsafe, Baremetal, Float, Macro
};
enum class Delimeters {
    LeftParen, RightParen, LeftBracket, RightBracket, Semicolon, Comma, Dot, LeftBraces, RightBraces, Colon
};

// Mapping entries

struct EKeyword { std::string name; Keywords token; };
struct EOperator { std::string name; Operators token; };
struct EDecorator { std::string name; Decorators token; };
struct EPreprocessor { std::string name; Preprocessors token; };
struct EDelimeter { std::string name; Delimeters token; };

/// this is what it costs us parser no-hardcode proofness btw
const std::unordered_map<std::string, Keywords>& getKeywordMap();
const std::unordered_map<std::string, Operators>& getOperatorMap();
const std::unordered_map<std::string, Decorators>& getDecoratorMap();
const std::unordered_map<std::string, Preprocessors>& getPreprocessorMap();
const std::unordered_map<std::string, Delimeters>& getDelimeterMap();

const std::unordered_map<Keywords, std::string>& getKeywordNames();
const std::unordered_map<Operators, std::string>& getOperatorNames();
const std::unordered_map<Decorators, std::string>& getDecoratorNames();
const std::unordered_map<Preprocessors, std::string>& getPreprocessorNames();
const std::unordered_map<Delimeters, std::string>& getDelimeterNames();