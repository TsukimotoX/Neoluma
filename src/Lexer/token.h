#ifndef TOKEN_H
#define TOKEN_H 

#include <optional>
#include <string>

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

    Type_Int,
    Type_Float,
    Type_Number,
    Type_Boolean,
    Type_String,
    Type_Array,
    Type_Set,
    Type_Dictionary,
    Type_Void,
    Type_Result,
};

struct Token {
    TokenType type;
    std::string value;
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

std::optional<TokenType> getBuiltinType(const std::string& word);
bool isBoolLiteral(const std::string& word, BoolValues& out);
bool isKeyword(const std::string& word);
bool isKeyword(const std::string& word, Keywords& out);
bool isOperator(const std::string& word, Operators& out);
bool isPreprocessor(const std::string& word, Preprocessors& out);
bool isDecorator(const std::string& word, Decorators& out);
bool isDelimeter(const std::string& word, Delimeters& out);

#endif