#include "errormanager.hpp"
#include "../../../Libraries/color/color.hpp"
#include <print>
#include <sstream>

void ErrorManager::printErrors(const std::string& source) {
    if (errors.empty()) return;

    int count = 1;
    for (auto& e : errors) {
        std::string typeColor;
        std::string hintColor = Color::TextHex("#f6ff75");
        std::string msgColor = Color::TextHex("#ff7575");

        switch (e.type) {
            case ErrorType::Syntax:    typeColor = Color::TextHex("#ff5050"); break;
            case ErrorType::Semantic:  typeColor = Color::TextHex("#ff9f40"); break;
            case ErrorType::Type:      typeColor = Color::TextHex("#c75fff"); break;
            case ErrorType::Preprocessor: typeColor = Color::TextHex("#00bfff"); break;
            case ErrorType::Codegen:   typeColor = Color::TextHex("#ff75d7"); break;
            case ErrorType::Runtime:   typeColor = Color::TextHex("#ffa500"); break;
        }

        std::istringstream srcStream(source);
        std::string line;
        int lineNum = 1;
        std::string errorLine;
        while (std::getline(srcStream, line)) {
            if (lineNum == e.token.line) {
                errorLine = line;
                break;
            }
            lineNum++;
        }

        std::println("{}[{}] ❌ {}{}", typeColor, e.token.line, e.message, Color::Reset);
        std::println("   ➡️ {}:{}:{}", e.token.filePath, e.token.line, e.token.column);
        std::println("   |");
        std::println("{:>3} | {}", e.token.line, errorLine);
        std::string pointerLine(e.token.column - 1, ' ');
        std::println("   |{}^", pointerLine);
        if (!e.hint.empty())
            std::println("   💡 hint: {}{}{}", hintColor, e.hint, Color::Reset);

        count++;
        std::println("");
    }
}