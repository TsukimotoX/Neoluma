#include "token.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <unordered_map>

std::unordered_map<std::string, Keywords> keywordMap = {
    {"function", Keywords::Function}, {"fn", Keywords::Function}, {"class", Keywords::Class}, {"enum", Keywords::Enum}, {"interface", Keywords::Interface}, {"namespace", Keywords::Namespace},
    {"if", Keywords::If}, {"else", Keywords::Else},
    {"for", Keywords::For}, {"while", Keywords::While}, {"break", Keywords::Break}, {"continue", Keywords::Continue},
    {"switch", Keywords::Switch}, {"case", Keywords::Case}, {"default", Keywords::Default},
    {"try", Keywords::Try}, {"catch", Keywords::Catch}, {"throw", Keywords::Throw},
    {"async", Keywords::Async}, {"await", Keywords::Await},
    {"yield", Keywords::Yield}, {"return", Keywords::Return},
    {"static", Keywords::Static}, {"decorator", Keywords::Decorator}, {"const", Keywords::Const},
    {"as", Keywords::As}, {"with", Keywords::With}, {"in", Keywords::In}, {":", Keywords::In}, {"lambda", Keywords::Lambda},
    {"debug", Keywords::Debug}, {"public", Keywords::Public}, {"protected", Keywords::Protected}, {"private", Keywords::Private},
};

std::unordered_map<std::string, NTokenType> typesMap = {
    {"int", NTokenType::Integer}, {"float", NTokenType::Float}, {"number", NTokenType::Number}, {"bool", NTokenType::Boolean}, {"string", NTokenType::String},
    {"array", NTokenType::Array}, {"set", NTokenType::Set}, {"dict", NTokenType::Dictionary}, {"void", NTokenType::Void}, {"result", NTokenType::Result}
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
        case NTokenType::NullLiteral:     typeStr = "NullLiteral"; break;
        case NTokenType::Identifier:      typeStr = "Identifier"; break;
        case NTokenType::Keyword:         typeStr = "Keyword"; break;
        case NTokenType::Decorator:       typeStr = "Decorator"; break;
        case NTokenType::Preprocessor:    typeStr = "Preprocessor"; break;
        case NTokenType::Delimeter:       typeStr = "Delimeter"; break;
        case NTokenType::Operator:        typeStr = "Operator"; break;
        case NTokenType::AssignmentArrow: typeStr = "AssignmentArrow"; break;
        case NTokenType::Question:        typeStr = "Question"; break;
        case NTokenType::UnknownToken:         typeStr = "Unknown"; break;
        case NTokenType::EndOfFile:       typeStr = "EndOfFile"; break;
        case NTokenType::Type:            typeStr = "Type"; break;

        case NTokenType::Integer:         typeStr = "Integer"; break;
        case NTokenType::Float:           typeStr = "Float"; break;
        case NTokenType::Number:          typeStr = "Number"; break;
        case NTokenType::String:          typeStr = "String"; break;
        case NTokenType::Void:            typeStr = "Void"; break;
        case NTokenType::Array:           typeStr = "Array"; break;
        case NTokenType::Boolean:         typeStr = "Boolean"; break;
        case NTokenType::Dictionary:      typeStr = "Dictionary"; break;
        case NTokenType::Set:             typeStr = "Set"; break;
        case NTokenType::Result:          typeStr = "Result"; break;
        default:                         typeStr = "<UNK>"; break;
    }
    return std::format("[{}] -> \"{}\"\n", typeStr, value);
}

