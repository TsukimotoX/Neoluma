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
        //std::string errid = "Compiler.Core.ErrorManager.ErrorType.";
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

        std::istringstream srcStream(readFile(e.span.filePath));
        std::string prevLine;
        std::string line; //temp to read those
        std::string nextLine;
        std::string errorLine;
        int lineNum = 1;
        while (std::getline(srcStream, line)) {
            if (lineNum == e.span.line) {
                errorLine = line;
                std::getline(srcStream, nextLine);
                break;
            }
            prevLine = line;
            lineNum++;
        }

        // Replaces filepath's \ to /
        size_t fppos = 0;
        while ((fppos = e.span.filePath.find("\\", fppos)) != std::string::npos) {
            e.span.filePath.replace(fppos, 1, "/");
            fppos += 1;
        }

        auto msg = e.message + (e.contextKey.has_value() ? formatStr(": {}", e.contextKey.value()) : "");
        std::println("{}[{}]  ❌  {}{}", typeColor, formatErrorType(e.detailedType),
            msg,
            Color::Reset);
        std::println("➡️  {}:{}:{}", e.span.filePath, e.span.line, e.span.column);
        std::println("{:>3} | {}", e.span.line-1, prevLine);
        std::println("{:>3} | {}", e.span.line, errorLine);
        auto tmp = std::string(std::to_string(e.span.line).length()+1, ' ');
        std::println("{} | {}{} {}", tmp, std::string(e.span.column-1, ' '), std::string(e.span.len+2, '^'), msg);
        std::println("{:>3} | {}", e.span.line+1, nextLine);
        if (!e.hint.empty()) std::println(std::cout, "{}", formatStr(Localization::translate("Compiler.Core.ErrorManager.hint"), hintColor, e.hint, Color::Reset));

        count++;
    }

    std::println(std::cout, "{}{} errors found!{}", Color::TextHex("#ff5050"), errors.size(), Color::Reset);
}

std::string ErrorManager::formatErrorType(std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    if (std::holds_alternative<SyntaxErrors>(detailedType)) return std::format("NSyE{}", (int)std::get<SyntaxErrors>(detailedType)+1);
    if (std::holds_alternative<AnalysisErrors>(detailedType)) return std::format("NAnE{}", (int)std::get<AnalysisErrors>(detailedType)+1);
    if (std::holds_alternative<PreprocessorErrors>(detailedType)) return std::format("NPrE{}", (int)std::get<PreprocessorErrors>(detailedType)+1);
    if (std::holds_alternative<CodegenErrors>(detailedType)) return std::format("NCoE{}", (int)std::get<CodegenErrors>(detailedType)+1);
    if (std::holds_alternative<RuntimeErrors>(detailedType)) return std::format("NRuE{}", (int)std::get<RuntimeErrors>(detailedType)+1);
    return "[ErrorManager/formatErrorType] No error type detected";
}