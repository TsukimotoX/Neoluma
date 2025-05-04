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

        case TokenType::Type_Int: typeStr = "Integer"; break;
        case TokenType::Type_Float: typeStr = "Float"; break;
        case TokenType::Type_Number: typeStr = "Number"; break;
        case TokenType::Type_String: typeStr = "String"; break;
        case TokenType::Type_Void: typeStr = "Void"; break;
        case TokenType::Type_Array: typeStr = "Array"; break;
        case TokenType::Type_Boolean: typeStr = "Boolean"; break;
        case TokenType::Type_Dictionary: typeStr = "Dictionary"; break;
        case TokenType::Type_Set: typeStr = "Set"; break;
        case TokenType::Type_Result: typeStr = "Result"; break;
    }

    return "[" + typeStr + "(\"" + value + "\")]";
}

std::optional<TokenType> getBuiltinType(const std::string& word) {
    static const std::unordered_map<std::string, TokenType> map = {
        {"int", TokenType::Type_Int},
        {"float", TokenType::Type_Float},
        {"number", TokenType::Type_Number},
        {"bool", TokenType::Type_Boolean},
        {"string", TokenType::Type_String},
        {"array", TokenType::Type_Array},
        {"set", TokenType::Type_Set},
        {"dict", TokenType::Type_Dictionary},
        {"void", TokenType::Type_Void},
        {"result", TokenType::Type_Result}
    };
    auto it = map.find(word);
    if (it != map.end()) return it->second;
    return std::nullopt;
}
bool isBoolLiteral(const std::string& word, BoolValues& out) {
    auto it = boolMap.find(word);
    if (it != boolMap.end()) {
        out = it->second;
        return true;
    }
    return false;
}
bool isKeyword(const std::string& word) {
    return keywordMap.count(word);
}
bool isKeyword(const std::string& word, Keywords& out) {
    auto it = keywordMap.find(word);
    if (it != keywordMap.end()) {
        out = it->second;
        return true;
    }
    return false;
}
bool isOperator(const std::string& word, Operators& out) {
    auto it = operatorMap.find(word);
    if (it != operatorMap.end()) {
        out = it->second;
        return true;
    }
    return false;
}
bool isPreprocessor(const std::string& word, Preprocessors& out) {
    auto it = preprocessorMap.find(word);
    if (it != preprocessorMap.end()) {
        out = it->second;
        return true;
    }
    return false;
}
bool isDecorator(const std::string& word, Decorators& out) {
    auto it = decoratorMap.find(word);
    if (it != decoratorMap.end()) {
        out = it->second;
        return true;
    }
    return false;
}
bool isDelimeter(const std::string& word, Delimeters& out) {
    auto it = delimeterMap.find(word);
    if (it != delimeterMap.end()) {
        out = it->second;
        return true;
    }
    return false;
}
