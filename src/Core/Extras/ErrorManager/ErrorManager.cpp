#include "ErrorManager.hpp"
#include "../../../Libraries/Color/Color.hpp"
#include <print>
#include <sstream>
#include <format>

#include "HelperFunctions.hpp"
#include "Libraries/Localization/Localization.hpp"

void ErrorManager::printErrors() {
    if (errors.empty()) return;

    std::unordered_map<std::string, std::string> fileCache;

    for (auto& e : errors) {
        if (!e.span.filePath.empty() && !fileCache.contains(e.span.filePath))
            fileCache[e.span.filePath] = readFile(e.span.filePath);

        std::string typeColor;
        std::string hintColor = Color::TextHex("#f6ff75");

        switch (e.type) {
            case ErrorType::Syntax:       typeColor = Color::TextHex("#ff5050"); break;
            case ErrorType::Analysis:     typeColor = Color::TextHex("#ff9f40"); break;
            case ErrorType::Preprocessor: typeColor = Color::TextHex("#00bfff"); break;
            case ErrorType::Codegen:      typeColor = Color::TextHex("#ff75d7"); break;
            case ErrorType::Runtime:      typeColor = Color::TextHex("#ffa500"); break;
            case ErrorType::None:         typeColor = Color::TextHex("#4A2BD6"); break;
        }

        std::istringstream srcStream(e.span.filePath.empty() ? std::string{} : fileCache[e.span.filePath]);
        std::string prevLine, line, errorLine, nextLine;
        int line1 = std::max(1, e.span.line);
        int col1  = std::max(1, e.span.column);
        int col0  = col1 - 1;

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

        size_t fppos = 0;
        while ((fppos = e.span.filePath.find("\\", fppos)) != std::string::npos) {
            e.span.filePath.replace(fppos, 1, "/");
            fppos += 1;
        }

        auto msg = Localization::translatef(e.messageKey, e.messageArgs);
        std::println("{}[{}]  ❌  {}{}", typeColor, formatErrorType(e.detailedType), msg, Color::Reset);
        if (!e.span.filePath.empty()) {
            std::println("➡️  {}:{}:{}", e.span.filePath, line1, col1);
            if (line1 > 1) std::println("{:>3} | {}", line1 - 1, prevLine);
            std::println("{:>3} | {}{}{}", line1, Color::TextHex("#ff5050"), errorLine, Color::Reset);
            std::println("{}| {}{} {}", std::string(std::to_string(line1).length() + 2, ' '), std::string((size_t)col0, ' '), std::string((size_t)e.span.len + 2, '^'), msg);
            if (!nextLine.empty()) std::println("{:>3} | {}", line1 + 1, nextLine);
        }
        if (!e.hintKey.empty()) std::println(std::cout, "{}{}{}\n", hintColor, formatStr(Localization::translate("ErrorManager.hint"), Localization::translatef(e.hintKey, e.hintArgs)), Color::Reset);
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

std::string ErrorManager::formatStage(ErrorType type) {
    switch (type) {
        case ErrorType::Syntax: return "parser";
        case ErrorType::Analysis: return "semantic";
        case ErrorType::Preprocessor: return "orchestrator";
        case ErrorType::Codegen: return "codegen";
        case ErrorType::Runtime: return "runtime";
        case ErrorType::None: return "none";
    }
    return "none";
}

json::Value ErrorManager::toJson() const {
    json::Object root;
    root.emplace_back("status", errors.empty() ? json::Value("ok") : json::Value("error"));

    json::Array errorArray;
    errorArray.reserve(errors.size());

    for (const auto& error : errors) {
        json::Object item;
        item.emplace_back("stage", formatStage(error.type));
        item.emplace_back("type", formatStage(error.type));
        item.emplace_back("error_code", formatErrorType(error.detailedType));
        item.emplace_back("file", error.span.filePath);
        item.emplace_back("line", (std::int64_t)error.span.line);
        item.emplace_back("column", (std::int64_t)error.span.column);
        item.emplace_back("length", (std::int64_t)error.span.len);
        item.emplace_back("message_key", error.messageKey);
        item.emplace_back("message", Localization::translatef(error.messageKey, error.messageArgs));
        item.emplace_back("hint_key", error.hintKey);
        item.emplace_back("hint", error.hintKey.empty() ? json::Value("") : json::Value(Localization::translatef(error.hintKey, error.hintArgs)));

        json::Array messageArgs;
        for (const auto& arg : error.messageArgs) messageArgs.emplace_back(arg);
        item.emplace_back("message_args", std::move(messageArgs));

        json::Array hintArgs;
        for (const auto& arg : error.hintArgs) hintArgs.emplace_back(arg);
        item.emplace_back("hint_args", std::move(hintArgs));

        errorArray.emplace_back(std::move(item));
    }

    root.emplace_back("errors", std::move(errorArray));
    return json::Value(std::move(root));
}
