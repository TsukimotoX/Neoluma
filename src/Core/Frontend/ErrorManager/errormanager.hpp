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
    UndefinedVariable,      // TODO: TO BE IMPLEMENTED
    RedefinedVariable,      // TODO: TO BE IMPLEMENTED
    UninitializedVariable,  // TODO: TO BE IMPLEMENTED
    ConstantReassignment,   // TODO: TO BE IMPLEMENTED
    VariableOutOfScope,
    ShadowedVariable,

    // Functions
    FunctionMismatch,        // TODO: TO BE IMPLEMENTED
    UndefinedFunction,
    WrongArgumentCount,      // TODO: TO BE IMPLEMENTED
    MissingRequiredParameter,
    DuplicateParameterName,
    InvalidParameterOrder,
    MissingReturnStatement,  // TODO: TO BE IMPLEMENTED
    ReturnInVoidFunction,    // TODO: TO BE IMPLEMENTED

    // Classes & OOP
    UndefinedMember,         // TODO: TO BE IMPLEMENTED
    CircularInheritance,     // TODO: TO BE IMPLEMENTED
    InvalidConstructor,      // TODO: TO BE IMPLEMENTED
    MissingSuperCall,
    InvalidSuperCall,
    AccessViolation,
    InvalidOverride,         // TODO: TO BE IMPLEMENTED
    OverrideSignatureMismatch,

    // Modifiers & Decorators
    InvalidModifierUsage,
    ConflictingModifiers,
    DecoratorMisuse,
    UndefinedDecorator,
    DecoratorOnInvalidTarget,
    DecoratorArgumentMismatch,
    MultipleEntryPoints,       // TODO: TO BE IMPLEMENTED

    // Control Flow
    BreakOutsideLoop,          // TODO: TO BE IMPLEMENTED
    ContinueOutsideLoop,       // TODO: TO BE IMPLEMENTED
    UnreachableCode,           // TODO: TO BE IMPLEMENTED
    DuplicateCaseValue,        // TODO: TO BE IMPLEMENTED
    CaseTypeMismatch,          // TODO: TO BE IMPLEMENTED

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
    TypeMismatch,       // TODO: TO BE IMPLEMENTED
    UnknownType,        // TODO: TO BE IMPLEMENTED
    InvalidCast,
    LiteralOverflow,

    // Operations
    AssignmentTypeMismatch,
    BinaryOperationTypeMismatch, // TODO: TO BE IMPLEMENTED
    UnaryOperationTypeMismatch,
    ReturnTypeMismatch,          // TODO: TO BE IMPLEMENTED
    ArgumentTypeMismatch,        // TODO: TO BE IMPLEMENTED

    // Nullable
    NullAssignmentToNonNullable, // TODO: TO BE IMPLEMENTED
    NullableAccessWithoutCheck,  // TODO: TO BE IMPLEMENTED

    // Collections
    ArrayElementTypeMismatch,
    SetDuplicateValue,
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
    ImportNotFound,          // TODO: TO BE IMPLEMENTED
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