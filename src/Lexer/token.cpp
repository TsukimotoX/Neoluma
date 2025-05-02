#include "token.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

std::unordered_map<std::string, Keywords> keywordMap = {
    {"function", Keywords::Function}, {"class", Keywords::Class}, {"enum", Keywords::Enum}, {"interface", Keywords::Interface}, {"#namespace", Keywords::Namespace},
    {"if", Keywords::If}, {"else", Keywords::Else}, {"elif", Keywords::Elif},
    {"for", Keywords::For}, {"foreach", Keywords::Foreach}, {"while", Keywords::While}, {"break", Keywords::Break}, {"continue", Keywords::Continue},
    {"switch", Keywords::Switch}, {"case", Keywords::Case}, {"default", Keywords::Default},
    {"try", Keywords::Try}, {"catch", Keywords::Catch}, {"throw", Keywords::Throw},
    {"async", Keywords::Async}, {"await", Keywords::Await},
    {"yield", Keywords::Yield}, {"return", Keywords::Return},
    {"static", Keywords::Static}, {"decorator", Keywords::Decorator},
    {"as", Keywords::As}, {"with", Keywords::With},
};

std::unordered_map<std::string, Operators> operatorMap = {
    {"+", Operators::Add}, {"-", Operators::Subtract}, {"*", Operators::Multiply}, {"/", Operators::Divide}, {"%", Operators::Modulo}, {"^", Operators::Power},
    {"==", Operators::Equal}, {"!=", Operators::NotEqual}, {"<", Operators::LessThan}, {">", Operators::GreaterThan}, {"<=", Operators::LessThanOrEqual}, {">=", Operators::GreaterThanOrEqual},
    {"&&", Operators::LogicalAnd}, {"||", Operators::LogicalOr}, {"!", Operators::LogicalNot},
    {"and", Operators::LogicalAnd}, {"or", Operators::LogicalOr}, {"not", Operators::LogicalNot},
};

std::unordered_map<std::string, BoolValues> boolMap = {
    {"true", BoolValues::True}, {"false", BoolValues::False}, {"null", BoolValues::Null}, 
};

std::unordered_map<std::string, Decorators> decoratorMap = {
    {"public", Decorators::Public}, {"protected", Decorators::Protected}, {"private", Decorators::Private},
    {"float", Decorators::Float}, {"entry", Decorators::Entry}, {"unsafe", Decorators::Unsafe}, {"comptime", Decorators::Comptime}, {"override", Decorators::Override},
};

std::unordered_map<std::string, Preprocessors> preprocessorMap = {
    {"import", Preprocessors::Import}, {"from", Preprocessors::From}, {"use", Preprocessors::Use}, {"as", Preprocessors::As}, {"unsafe", Preprocessors::Unsafe}, {"baremetal", Preprocessors::Baremetal}, {"float", Preprocessors::Float}, {"macro", Preprocessors::Macro},
};
std::unordered_map<std::string, Delimeters> delimeterMap {
    {"(", Delimeters::LeftParen}, {")", Delimeters::RightParen},
    {"{", Delimeters::LeftBracket}, {"}", Delimeters::RightBracket},
    {";", Delimeters::Semicolon}, {",", Delimeters::Comma},
    {".", Delimeters::Dot}, {"[", Delimeters::LeftSquareBracket},
    {"]", Delimeters::RightSquareBracket}, {"=>", Delimeters::RightArrow}
};