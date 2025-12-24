#include "errormanager.hpp"
#include "../../../Libraries/color/color.hpp"
#include <print>
#include <sstream>
#include <format>

#include "Libraries/localization/localization.hpp"

void ErrorManager::printErrors(const std::string& source) {
    if (errors.empty()) return;

    int count = 1;
    for (auto& e : errors) {
        std::string typeColor;
        std::string hintColor = Color::TextHex("#f6ff75");
        std::string msgColor = Color::TextHex("#ff7575");

        switch (e.type) {
            case ErrorType::Syntax:       typeColor = Color::TextHex("#ff5050"); break;
            case ErrorType::Semantic:     typeColor = Color::TextHex("#ff9f40"); break;
            case ErrorType::Type:         typeColor = Color::TextHex("#c75fff"); break;
            case ErrorType::Preprocessor: typeColor = Color::TextHex("#00bfff"); break;
            case ErrorType::Codegen:      typeColor = Color::TextHex("#ff75d7"); break;
            case ErrorType::Runtime:      typeColor = Color::TextHex("#ffa500"); break;
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

        std::println("{}[{}] ❌ {}{}", typeColor, formatErrorType(e.detailedType), e.message, Color::Reset);
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
/*
std::string ErrorManager::formatErrorType(std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    if (std::holds_alternative<SyntaxErrors>(detailedType)) return "NSyE{}" + (int)std::get<SyntaxErrors>(detailedType);
    if (std::holds_alternative<SemanticErrors>(detailedType)) return "NSeE{}" + (int)std::get<SemanticErrors>(detailedType);
    if (std::holds_alternative<TypeErrors>(detailedType)) return "NTyE{}" + (int)std::get<TypeErrors>(detailedType);
    if (std::holds_alternative<PreprocessorErrors>(detailedType)) return "NPrE{}" + (int)std::get<PreprocessorErrors>(detailedType);
    if (std::holds_alternative<CodegenErrors>(detailedType)) return "NCoE{}" + (int)std::get<CodegenErrors>(detailedType);
    if (std::holds_alternative<RuntimeErrors>(detailedType)) return "NRuE{}" + (int)std::get<RuntimeErrors>(detailedType);
    return "[ErrorManager/formatErrorType] No error type detected";
}

std::string ErrorManager::formatMessage(std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    switch (std::get<SyntaxErrors>(detailedType)){
        case SyntaxErrors::UnexpectedToken: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::MissingToken: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.MissingToken"); break;
        case SyntaxErrors::InvalidStatement: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::InvalidPreprocessorDirective: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::UnterminatedString: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::UnterminatedComment: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::InvalidNumberFormat: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::UnexpectedEndOfFile: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::MismatchedBrackets: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
    }
}*/