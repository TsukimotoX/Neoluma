#ifndef TOKEN_H
#define TOKEN_H 

#include <optional>
#include <string>

enum TokenType {
    NullLiteral,
    Identifier, //Combined Variable, function and class names, and etc.
    Keyword,
    Type,
    Decorator,
    Preprocessor,
    Delimeter,
    Operator,
    AssignmentArrow,
    Question,
    Unknown,
    EndOfFile,
};

enum BuiltinType {
    Number,
    Int,
    Float,
    String,
    Boolean,
    Set,
    Dictionary,
    Array,
    Void,
    Result,
};

struct Token {
    TokenType type;
    std::string value;
    std::optional<BuiltinType> builtinType;
    std::string toString() const;
};

enum class Keywords {
    Function, Class, Enum, Interface, Namespace,
    If, Else, Elif,
    For, Foreach, While, Break, Continue,
    Switch, Case, Default,
    Try, Catch, Throw,
    Async, Await,
    Yield, Return,
    Static, Decorator,
    As, With, In, Lambda
};

enum class Operators {
    Add, Subtract, Multiply, Divide, Modulo, Power,
    Equal, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual,
    LogicalAnd, LogicalOr, LogicalNot,
    Assign, Nullable
};
enum class BoolValues {
    True, False, Null
};
enum class Decorators {
    Public, Protected, Private, Float, Entry, Unsafe, Comptime, Override
};
enum class Preprocessors {
    Import, From, Use, As, Unsafe, Baremetal, Float, Macro
};
enum class Delimeters {
    LeftParen, RightParen, LeftBracket, RightBracket, Semicolon, Comma, Dot, LeftSquareBracket, RightSquareBracket, RightArrow,
};

extern std::unordered_map<std::string, Keywords> keywordMap;
extern std::unordered_map<std::string, Operators> operatorMap;
extern std::unordered_map<std::string, BoolValues> boolMap;
extern std::unordered_map<std::string, Decorators> decoratorMap;
extern std::unordered_map<std::string, Preprocessors> preprocessorMap;
extern std::unordered_map<std::string, Delimeters> delimeterMap;

#endif