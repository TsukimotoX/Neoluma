#include "errormanager.hpp"
#include "../../../Libraries/color/color.hpp"
#include <print>
#include <sstream>
#include <format>

#include "HelperFunctions.hpp"
#include "Libraries/localization/localization.hpp"

void ErrorManager::printErrors() {
    if (errors.empty()) return;

    int count = 1;
    for (auto& e : errors) {
        std::string typeColor;
        std::string hintColor = Color::TextHex("#f6ff75");
        std::string msgColor = Color::TextHex("#ff7575");

        switch (e.type) {
            case ErrorType::Syntax:       typeColor = Color::TextHex("#ff5050"); break;
            case ErrorType::Analysis:     typeColor = Color::TextHex("#ff9f40"); break;
            case ErrorType::Preprocessor: typeColor = Color::TextHex("#00bfff"); break;
            case ErrorType::Codegen:      typeColor = Color::TextHex("#ff75d7"); break;
            case ErrorType::Runtime:      typeColor = Color::TextHex("#ffa500"); break;
        }

        std::istringstream srcStream(readFile(e.filePath));
        std::string line;
        int lineNum = 1;
        std::string errorLine;

        while (std::getline(srcStream, line)) {
            if (lineNum == e.token.line) {
                errorLine = line;
                break;
            } lineNum++;
        }

        // Replaces filepath's \ to /
        size_t fppos = 0;
        while ((fppos = e.token.filePath.find("\\", fppos)) != std::string::npos) {
            e.token.filePath.replace(fppos, 1, "/");
            fppos += 1;
        }


        std::println("{}[{}]  ❌  {}{}", typeColor, formatErrorType(e.detailedType), e.message, Color::Reset);
        std::println("➡️  {}:{}:{}", e.token.filePath, e.token.line, e.token.column);
        std::println("    |");
        std::println("{:>3} | {}", e.token.line, errorLine);
        std::string pointerLine(e.token.column - 1, ' ');
        std::println("    |{}^^^^^", pointerLine);
        if (!e.hint.empty())
            std::println(std::cout, "{}", formatStr(Localization::translate("Compiler.Core.ErrorManager.hint"), hintColor, e.hint, Color::Reset));

        count++;
        std::println("");
    }
}

std::string ErrorManager::formatErrorType(std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    if (std::holds_alternative<SyntaxErrors>(detailedType)) return std::format("NSyE{}", (int)std::get<SyntaxErrors>(detailedType)+1);
    if (std::holds_alternative<AnalysisErrors>(detailedType)) return std::format("NAnE{}", (int)std::get<AnalysisErrors>(detailedType)+1);
    if (std::holds_alternative<PreprocessorErrors>(detailedType)) return std::format("NPrE{}", (int)std::get<PreprocessorErrors>(detailedType)+1);
    if (std::holds_alternative<CodegenErrors>(detailedType)) return std::format("NCoE{}", (int)std::get<CodegenErrors>(detailedType)+1);
    if (std::holds_alternative<RuntimeErrors>(detailedType)) return std::format("NRuE{}", (int)std::get<RuntimeErrors>(detailedType)+1);
    return "[ErrorManager/formatErrorType] No error type detected";
}