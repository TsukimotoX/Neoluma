#include "token.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

std::unordered_map<std::string, Keywords> keywordMap = {
    {"function", Keywords::Function}, {"class", Keywords::Class}, {"enum", Keywords::Enum}, {"interface", Keywords::Interface}, {"namespace", Keywords::Namespace},
    {"if", Keywords::If}, {"else", Keywords::Else},
    {"for", Keywords::For}, {"while", Keywords::While}, {"break", Keywords::Break}, {"continue", Keywords::Continue},
    {"switch", Keywords::Switch}, {"case", Keywords::Case}, {"default", Keywords::Default},
    {"try", Keywords::Try}, {"catch", Keywords::Catch}, {"throw", Keywords::Throw},
    {"async", Keywords::Async}, {"await", Keywords::Await},
    {"yield", Keywords::Yield}, {"return", Keywords::Return},
    {"static", Keywords::Static}, {"decorator", Keywords::Decorator},
    {"as", Keywords::As}, {"with", Keywords::With}, {"in", Keywords::In}, {":", Keywords::In}, {"lambda", Keywords::Lambda},
    {"debug", Keywords::Debug}, {"public", Keywords::Public}, {"protected", Keywords::Protected}, {"private", Keywords::Private},
};

std::unordered_map<std::string, TokenType> typesMap = {
    {"int", TokenType::Integer}, {"float", TokenType::Float}, {"number", TokenType::Number}, {"bool", TokenType::Boolean}, {"string", TokenType::String},
    {"array", TokenType::Array}, {"set", TokenType::Set}, {"dict", TokenType::Dictionary}, {"void", TokenType::Void}, {"result", TokenType::Result}
};

std::unordered_map<std::string, Operators> operatorMap = {
    {"+", Operators::Add}, {"-", Operators::Subtract}, {"*", Operators::Multiply}, {"/", Operators::Divide}, {"%", Operators::Modulo}, {"^", Operators::Power},
    {"==", Operators::Equal}, {"!=", Operators::NotEqual}, {"<", Operators::LessThan}, {">", Operators::GreaterThan}, {"<=", Operators::LessThanOrEqual}, {">=", Operators::GreaterThanOrEqual},
    {"&&", Operators::LogicalAnd}, {"||", Operators::LogicalOr}, {"!", Operators::LogicalNot},
    {"and", Operators::LogicalAnd}, {"or", Operators::LogicalOr}, {"not", Operators::LogicalNot},
    {"+=", Operators::AddAssign}, {"-=", Operators::SubAssign}, {"*=", Operators::MulAssign}, {"/=", Operators::DivAssign}, {"%=", Operators::ModAssign}, {"^=", Operators::PowerAssign}
};

std::unordered_map<std::string, Decorators> decoratorMap = {
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
    {"]", Delimeters::RightSquareBracket},
};

std::string Token::toString() const {
    std::string typeStr;

    switch (type) {
        case TokenType::NullLiteral: typeStr = "NullLiteral"; break;
        case TokenType::Identifier: typeStr = "Identifier"; break;
        case TokenType::Keyword: typeStr = "Keyword"; break;
        case TokenType::Decorator: typeStr = "Decorator"; break;
        case TokenType::Preprocessor: typeStr = "Preprocessor"; break;
        case TokenType::Delimeter: typeStr = "Delimeter"; break;
        case TokenType::Operator: typeStr = "Operator"; break;
        case TokenType::AssignmentArrow: typeStr = "AssignmentArrow"; break;
        case TokenType::Question: typeStr = "Question"; break;
        case TokenType::Unknown: typeStr = "Unknown"; break;
        case TokenType::EndOfFile: typeStr = "EndOfFile"; break;

        case TokenType::Integer: typeStr = "Integer"; break;
        case TokenType::Float: typeStr = "Float"; break;
        case TokenType::Number: typeStr = "Number"; break;
        case TokenType::String: typeStr = "String"; break;
        case TokenType::Void: typeStr = "Void"; break;
        case TokenType::Array: typeStr = "Array"; break;
        case TokenType::Boolean: typeStr = "Boolean"; break;
        case TokenType::Dictionary: typeStr = "Dictionary"; break;
        case TokenType::Set: typeStr = "Set"; break;
        case TokenType::Result: typeStr = "Result"; break;
        default: typeStr = "<UNK>"; break;
    }

    return "[" + typeStr + "(\"" + value + "\")]";
}

