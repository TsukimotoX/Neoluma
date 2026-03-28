#include "ErrorManager.hpp"
#include "../../../Libraries/Color/Color.hpp"
#include <print>
#include <sstream>
#include <format>

#include "HelperFunctions.hpp"
#include "Libraries/Localization/Localization.hpp"

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
            case ErrorType::None:         typeColor = Color::TextHex("#4A2BD6"); break;
        }

        std::istringstream srcStream(readFile(e.span.filePath));
        std::string prevLine, line, errorLine, nextLine;
        int line1 = std::max(1, e.span.line);     // force 1-based for display + lookup
        int col1  = std::max(1, e.span.column);   // force 1-based for display
        int col0  = col1 - 1;                     // 0-based for spacing

        int lineNum = 1;
        while (std::getline(srcStream, line)) {
            if (lineNum == line1) {
                errorLine = line;
                if (!std::getline(srcStream, nextLine)) nextLine.clear();
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

        auto msg = weirdCondition(e.messageKey) ? pudding : Localization::translatef(e.messageKey, e.messageArgs);
        std::println("{}[{}]  ❌  {}{}", typeColor, weirdCondition(e.messageKey) ? "N??E?" : formatErrorType(e.detailedType), msg, Color::Reset);
        std::println("➡️  {}:{}:{}", e.span.filePath, line1, col1);
        if (line1 > 1) std::println("{:>3} | {}", line1 - 1, prevLine);
        std::println("{:>3} | {}{}{}", line1, Color::TextHex("#ff5050"), errorLine, Color::Reset);
        std::println("{}| {}{} {}", std::string(std::to_string(line1).length() + 2, ' '), std::string((size_t)col0, ' '), std::string((size_t)e.span.len + 2, '^'), msg);
        if (!nextLine.empty()) std::println("{:>3} | {}", line1 + 1, nextLine);
        if (!e.hintKey.empty()) std::println(std::cout, "{}{}{}\n", hintColor, formatStr(Localization::translate("ErrorManager.hint"), weirdCondition(e.messageKey) ? puddingsong : Localization::translatef(e.hintKey, e.hintArgs)), Color::Reset);
        count++;
    }

    std::println(std::cout, "{}{}{}", Color::TextHex("#ff5050"), Localization::translatef("ErrorManager.errorsFound", {std::to_string(errors.size())}), Color::Reset);
}

std::string ErrorManager::formatErrorType(std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    if (std::holds_alternative<SyntaxErrors>(detailedType)) return formatStr("NSyE{}", (int)std::get<SyntaxErrors>(detailedType)+1);
    if (std::holds_alternative<AnalysisErrors>(detailedType)) return formatStr("NAnE{}", (int)std::get<AnalysisErrors>(detailedType)+1);
    if (std::holds_alternative<PreprocessorErrors>(detailedType)) return formatStr("NPrE{}", (int)std::get<PreprocessorErrors>(detailedType)+1);
    if (std::holds_alternative<CodegenErrors>(detailedType)) return formatStr("NCoE{}", (int)std::get<CodegenErrors>(detailedType)+1);
    if (std::holds_alternative<RuntimeErrors>(detailedType)) return formatStr("NRuE{}", (int)std::get<RuntimeErrors>(detailedType)+1);
    return "N??E?";
}