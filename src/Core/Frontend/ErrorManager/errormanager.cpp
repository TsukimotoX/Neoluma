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

std::string ErrorManager::formatErrorType(std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    if (std::holds_alternative<SyntaxErrors>(detailedType)) return std::format("NSyE{}", (int)std::get<SyntaxErrors>(detailedType));
    if (std::holds_alternative<SemanticErrors>(detailedType)) return std::format("NSeE{}", (int)std::get<SemanticErrors>(detailedType));
    if (std::holds_alternative<TypeErrors>(detailedType)) return std::format("NTyE{}", (int)std::get<TypeErrors>(detailedType));
    if (std::holds_alternative<PreprocessorErrors>(detailedType)) return std::format("NPrE{}", (int)std::get<PreprocessorErrors>(detailedType));
    if (std::holds_alternative<CodegenErrors>(detailedType)) return std::format("NCoE{}", (int)std::get<CodegenErrors>(detailedType));
    if (std::holds_alternative<RuntimeErrors>(detailedType)) return std::format("NRuE{}", (int)std::get<RuntimeErrors>(detailedType));
    return "[ErrorManager/formatErrorType] No error type detected";
}

std::string ErrorManager::formatMessage(std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType){
    switch (std::get<SyntaxErrors>(detailedType)){
        case SyntaxErrors::UnexpectedToken: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedToken"); break;
        case SyntaxErrors::MissingToken: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.MissingToken"); break;
        case SyntaxErrors::InvalidStatement: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.InvalidStatement"); break;
        case SyntaxErrors::InvalidPreprocessorDirective: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.InvalidPreprocessorDirective"); break;
        case SyntaxErrors::UnterminatedString: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnterminatedString"); break;
        case SyntaxErrors::UnterminatedComment: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnterminatedComment"); break;
        case SyntaxErrors::InvalidNumberFormat: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.InvalidNumberFormat"); break;
        case SyntaxErrors::UnexpectedEndOfFile: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.UnexpectedEndOfFile"); break;
        case SyntaxErrors::MismatchedBrackets: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.syntax.errors.MismatchedBrackets"); break;
    }/*
    switch (std::get<SemanticErrors>(detailedType)){
        case SemanticErrors::UndefinedVariable: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.UndefinedVariable"); break;
        case SemanticErrors::RedefinedVariable: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.RedefinedVariable"); break;
        case SemanticErrors::UninitializedVariable: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.UninitializedVariable"); break;
        case SemanticErrors::ConstantReassignment: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.ConstantReassignment"); break;
        case SemanticErrors::VariableOutOfScope: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.VariableOutOfScope"); break;
        case SemanticErrors::ShadowedVariable: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.ShadowedVariable"); break;
        case SemanticErrors::FunctionMismatch: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.FunctionMismatch"); break;
        case SemanticErrors::UndefinedFunction: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.UndefinedFunction"); break;
        case SemanticErrors::WrongArgumentCount: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.WrongArgumentCount"); break;
        case SemanticErrors::MissingRequiredParameter: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.MissingRequiredParameter"); break;
        case SemanticErrors::DuplicateParameterName: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.DuplicateParameterName"); break;
        case SemanticErrors::InvalidParameterOrder: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidParameterOrder"); break;
        case SemanticErrors::InvalidReturnType: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidReturnType"); break;
        case SemanticErrors::MissingReturnStatement: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.MissingReturnStatement"); break;
        case SemanticErrors::ReturnInVoidFunction: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.ReturnInVoidFunction"); break;
        case SemanticErrors::UndefinedMember: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.UndefinedMember"); break;
        case SemanticErrors::CircularInheritance: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.CircularInheritance"); break;
        case SemanticErrors::InvalidSuperClass: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidSuperClass"); break;
        case SemanticErrors::MissingSuperCall: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.MissingSuperCall"); break;
        case SemanticErrors::InvalidSuperCall: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidSuperCall"); break;
        case SemanticErrors::AccessViolation: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.AccessViolation"); break;
        case SemanticErrors::InvalidOverride: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidOverride"); break;
        case SemanticErrors::OverrideSignatureMismatch: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.OverrideSignatureMismatch"); break;
        case SemanticErrors::InvalidModifierUsage: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidModifierUsage"); break;
        case SemanticErrors::ConflictingModifiers: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.ConflictingModifiers"); break;
        case SemanticErrors::DecoratorMisuse: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.DecoratorMisuse"); break;
        case SemanticErrors::UndefinedDecorator: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.UndefinedDecorator"); break;
        case SemanticErrors::DecoratorOnInvalidTarget: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.DecoratorOnInvalidTarget"); break;
        case SemanticErrors::DecoratorArgumentMismatch: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.DecoratorArgumentMismatch"); break;
        case SemanticErrors::MultipleEntryPoints: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.MultipleEntryPoints"); break;
        case SemanticErrors::BreakOutsideLoop: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.BreakOutsideLoop"); break;
        case SemanticErrors::ContinueOutsideLoop: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.ContinueOutsideLoop"); break;
        case SemanticErrors::UnreachableCode: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.UnreachableCode"); break;
        case SemanticErrors::DuplicateCaseValue: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.DuplicateCaseValue"); break;
        case SemanticErrors::CaseTypeMismatch: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.CaseTypeMismatch"); break;
        case SemanticErrors::InterfaceNotImplemented: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InterfaceNotImplemented"); break;
        case SemanticErrors::InterfaceSignatureMismatch: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InterfaceSignatureMismatch"); break;
        case SemanticErrors::DuplicateEnumMember: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.DuplicateEnumMember"); break;
        case SemanticErrors::InvalidCapture: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.InvalidCapture"); break;
        case SemanticErrors::ModifyingCapturedConst: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.ModifyingCapturedConst"); break;
        case SemanticErrors::AwaitOutsideAsync: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.AwaitOutsideAsync"); break;
        case SemanticErrors::YieldOutsideGenerator: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.YieldOutsideGenerator"); break;
        case SemanticErrors::AssignmentToNonLValue: return Localization::translate("Compiler.Core.ErrorManager.ErrorType.semantic.errors.AssignmentToNonLValue:"); break;
    }*/
    return "No error detected";
}