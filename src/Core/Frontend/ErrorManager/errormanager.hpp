#pragma once
#include "../token.hpp"
#include <vector>
#include <variant>

enum struct ErrorType {
    Syntax,
    Semantic,
    Type,
    Preprocessor,
    Codegen,
    Runtime,
};

// NSyE{x}
enum struct SyntaxErrors {
    UnexpectedToken,
    MissingToken,
    InvalidStatement,
    InvalidPreprocessorDirective,
    UnterminatedString,
    UnterminatedComment,
    InvalidNumberFormat,
    UnexpectedEndOfFile,
    MismatchedBrackets,
};

// NSeE{x}
enum struct SemanticErrors {
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
    InvalidReturnType,
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
};

// NTyE{x}
enum struct TypeErrors {
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

struct Error {
    ErrorType type;
    std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType;
    Token token;
    std::string message;
    std::string hint;
};

class ErrorManager {
public:
    void addError(ErrorType type, std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType, const Token& token, const std::string& msg, const std::string& hint = "") { errors.push_back(Error{type, detailedType, token, msg, hint}); }
    void printErrors(const std::string& source);
    [[nodiscard]] bool hasErrors() const { return errors.empty(); }
private:
    std::vector<Error> errors;
    static std::string formatErrorType(std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType);
    static std::string formatMessage(std::variant<SyntaxErrors, SemanticErrors, TypeErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType);
};