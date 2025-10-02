#include "token.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <unordered_map>
#include <format>

// Maps for every entry
const EKeyword keywordMap[] = {
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
    {"override", Keywords::Override},
};

const EOperator operatorMap[] {
    {"+", Operators::Add}, {"-", Operators::Subtract}, {"*", Operators::Multiply}, {"/", Operators::Divide}, {"%", Operators::Modulo}, {"^", Operators::Power},
    {"==", Operators::Equal}, {"!=", Operators::NotEqual}, {"<", Operators::LessThan}, {">", Operators::GreaterThan}, {"<=", Operators::LessThanOrEqual}, {">=", Operators::GreaterThanOrEqual},
    {"&&", Operators::LogicalAnd}, {"||", Operators::LogicalOr}, {"!", Operators::LogicalNot},
    {"and", Operators::LogicalAnd}, {"or", Operators::LogicalOr}, {"not", Operators::LogicalNot},
    {"=", Operators::Assign}, {"?", Operators::Nullable}, {"=>", Operators::AssignmentArrow},
    {"+=", Operators::AddAssign}, {"-=", Operators::SubAssign}, {"*=", Operators::MulAssign}, {"/=", Operators::DivAssign}, {"%=", Operators::ModAssign}, {"^=", Operators::PowerAssign}
};

const EDecorator decoratorMap[] {
    {"float", Decorators::Float}, {"entry", Decorators::Entry}, {"unsafe", Decorators::Unsafe}, {"comptime", Decorators::Comptime},
};

const EPreprocessor preprocessorMap[] {
    {"import", Preprocessors::Import}, {"from", Preprocessors::From}, {"as", Preprocessors::As}, {"unsafe", Preprocessors::Unsafe}, {"baremetal", Preprocessors::Baremetal}, {"float", Preprocessors::Float}, {"macro", Preprocessors::Macro},
};

const EDelimeter delimeterMap[] {
    {"(", Delimeters::LeftParen}, {")", Delimeters::RightParen},
    {"{", Delimeters::LeftBraces}, {"}", Delimeters::RightBraces},
    {";", Delimeters::Semicolon}, {":", Delimeters::Colon}, {"\n", Delimeters::Semicolon}, {",", Delimeters::Comma},
    {".", Delimeters::Dot}, {"[", Delimeters::LeftBracket},
    {"]", Delimeters::RightBracket},
};

std::string Token::toStr() const {
    std::string typeStr;

    switch (type) {
        case TokenType::Keyword:         typeStr = "Keyword"; break;
        case TokenType::Identifier:      typeStr = "Identifier"; break;
        case TokenType::Number:          typeStr = "Number"; break;
        case TokenType::Operator:        typeStr = "Operator"; break;
        case TokenType::String:          typeStr = "String"; break;
        case TokenType::Delimeter:       typeStr = "Delimeter"; break;
        case TokenType::Unknown:         typeStr = "Unknown"; break;
        case TokenType::Decorator:       typeStr = "Decorator"; break;
        case TokenType::Preprocessor:    typeStr = "Preprocessor"; break;
        case TokenType::EndOfFile:       typeStr = "EndOfFile"; break;
        case TokenType::Null:            typeStr = "Null"; break;
        default:                         typeStr = "<UNK>"; break;
    }

    return std::format("[{}] -> \"{}\"\n", typeStr, value);
}

const std::unordered_map<std::string, Keywords>& getKeywordMap() {
    static std::unordered_map<std::string, Keywords> map;
    if (map.empty()) {
        for (auto& k : keywordMap) map[k.name] = k.token;
    }
    return map;
}
const std::unordered_map<std::string, Operators>& getOperatorMap() {
    static std::unordered_map<std::string, Operators> map;
    if (map.empty()) {
        for (auto& k : operatorMap) map[k.name] = k.token;
    }
    return map;
}
const std::unordered_map<std::string, Decorators>& getDecoratorMap() {
    static std::unordered_map<std::string, Decorators> map;
    if (map.empty()) {
        for (auto& k : decoratorMap) map[k.name] = k.token;
    }
    return map;
}
const std::unordered_map<std::string, Preprocessors>& getPreprocessorMap() {
    static std::unordered_map<std::string, Preprocessors> map;
    if (map.empty()) {
        for (auto& k : preprocessorMap) map[k.name] = k.token;
    }
    return map;
}
const std::unordered_map<std::string, Delimeters>& getDelimeterMap() {
    static std::unordered_map<std::string, Delimeters> map;
    if (map.empty()) {
        for (auto& k : delimeterMap) map[k.name] = k.token;
    }
    return map;
}



const std::unordered_map<Keywords, std::string>& getKeywordNames() {
    static std::unordered_map<Keywords, std::string> map;
    if (map.empty()) {
        for (auto& k : keywordMap) map[k.token] = k.name;
    }
    return map;
}
const std::unordered_map<Operators, std::string>& getOperatorNames() {
    static std::unordered_map<Operators, std::string> map;
    if (map.empty()) {
        for (auto& k : operatorMap) map[k.token] = k.name;
    }
    return map;
}
const std::unordered_map<Decorators, std::string>& getDecoratorNames() {
    static std::unordered_map<Decorators, std::string> map;
    if (map.empty()) {
        for (auto& k : decoratorMap) map[k.token] = k.name;
    }
    return map;
}
const std::unordered_map<Preprocessors, std::string>& getPreprocessorNames() {
    static std::unordered_map<Preprocessors, std::string> map;
    if (map.empty()) {
        for (auto& k : preprocessorMap) map[k.token] = k.name;
    }
    return map;
}
const std::unordered_map<Delimeters, std::string>& getDelimeterNames() {
    static std::unordered_map<Delimeters, std::string> map;
    if (map.empty()) {
        for (auto& k : delimeterMap) map[k.token] = k.name;
    }
    return map;
}

