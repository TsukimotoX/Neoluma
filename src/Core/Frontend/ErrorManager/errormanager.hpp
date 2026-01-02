#pragma once
#include "../token.hpp"
#include <vector>
#include <variant>

#include "Core/Frontend/nodes.hpp"

enum struct ErrorType {
    Syntax,
    Analysis,
    Preprocessor,
    Codegen,
    Runtime,
};

// NSyE{x}
enum struct SyntaxErrors {
    UnexpectedToken,
    MissingToken,
    InvalidStatement,
    UnterminatedString,
    UnterminatedComment,
    InvalidNumberFormat,
    UnexpectedEndOfFile,
    MismatchedBrackets,
};

// NAnE{x}
enum struct AnalysisErrors {
    // Variables & Scope
    UndefinedVariable,
    RedefinedVariable,
    UninitializedVariable,
    ConstantReassignment,
    VariableOutOfScope,
    ShadowedVariable,

    // Functions
    FunctionMismatch,
    UndefinedFunction,
    WrongArgumentCount,
    MissingRequiredParameter,
    DuplicateParameterName,
    InvalidParameterOrder,
    MissingReturnStatement,
    ReturnInVoidFunction,

    // Classes & OOP
    UndefinedMember,
    CircularInheritance,
    InvalidSuperClass,
    MissingSuperCall,
    InvalidSuperCall,
    AccessViolation,
    InvalidOverride,
    OverrideSignatureMismatch,

    // Modifiers & Decorators
    InvalidModifierUsage,
    ConflictingModifiers,
    DecoratorMisuse,
    UndefinedDecorator,
    DecoratorOnInvalidTarget,
    DecoratorArgumentMismatch,
    MultipleEntryPoints,

    // Control Flow
    BreakOutsideLoop,
    ContinueOutsideLoop,
    UnreachableCode,
    DuplicateCaseValue,
    CaseTypeMismatch,

    // Interfaces & Enums
    InterfaceNotImplemented,
    InterfaceSignatureMismatch,
    DuplicateEnumMember,

    // Lambdas & Closures
    InvalidCapture,
    ModifyingCapturedConst,

    // Special Features
    AwaitOutsideAsync,
    YieldOutsideGenerator,

    // Assignment
    AssignmentToNonLValue,

    // Core Type Errors
    TypeMismatch,
    UnknownType,
    InvalidCast,
    LiteralOverflow,

    // Operations
    AssignmentTypeMismatch,
    BinaryOperationTypeMismatch,
    UnaryOperationTypeMismatch,
    ReturnTypeMismatch,
    ArgumentTypeMismatch,

    // Nullable
    NullAssignmentToNonNullable,
    NullableAccessWithoutCheck,

    // Collections
    ArrayElementTypeMismatch,
    DictKeyTypeMismatch,
    DictValueTypeMismatch,
    InvalidIndexType,
    IndexingNonIndexable,

    // Result Type
    ResultUnwrapWithoutCheck,

    // Member Access
    MemberAccessOnNonObject,
    MemberAccessOnNullable,

    // Inference
    TypeInferenceFailed,
    AmbiguousType,
};

// NPrE{x}
enum struct PreprocessorErrors {
    // Import
    ImportNotFound,
    CircularImport,
    ImportAliasConflict,
    InvalidImportPath,
    ForeignImportWithoutLangpack,

    // Macro
    MacroError,
    UndefinedMacro,
    MacroExpansionError,

    // Directive
    InvalidDirective,
    UnsafeWithoutDirective,
    BaremetalWithoutDirective,
    ConflictingDirectives,
    DirectiveInWrongContext,
};

//NCoE{x}
enum struct CodegenErrors {
    LLVMGenerationError,
    UnsupportedFeature,
    OptimizationFailure,
    LinkageError,
    TargetNotSupported,
};

//NRuE{x}
enum struct RuntimeErrors {
    DivisionByZero,
    NullReference,
    IndexOutOfBounds,
    FloatingPointError,
    IntegerOverflow,
    UninitializedVariableAccess,
    KeyNotFound,
    StackOverflow,
    UnhandledError,
};

struct ErrorSpan {
    std::string filePath;
    int line, column, len = 0;
    ErrorSpan(const std::string& filePath, const std::string& value, int line, int column): filePath(filePath), len((int)value.length()), line(line), column(column) {};
};

struct Error {
    ErrorType type;
    std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType;
    ErrorSpan span;
    std::string message; // Message that contains main error + context error
    std::string hint;  // Hint to fix the error

    std::optional<std::string> contextKey; // a text to refer to on context. like "expected '{}' after x"
};

struct ErrorManager {
    std::vector<Error> errors;

    void addError(ErrorType type, std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType, const ErrorSpan& span, const std::string& msg, const std::string& hint = "", const std::optional<std::string>& contextKey = std::nullopt) { errors.push_back(Error{type, detailedType, span, msg, hint, contextKey}); }
    void printErrors();
    [[nodiscard]] bool hasErrors() const { return !errors.empty(); }

    static std::string formatErrorType(std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType);
};