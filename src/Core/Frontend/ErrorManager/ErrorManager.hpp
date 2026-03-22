#pragma once
#include "../Token.hpp"
#include <vector>
#include <variant>

#include "Core/Frontend/Nodes.hpp"

enum struct ErrorType {
    Syntax,
    Analysis,
    Preprocessor,
    Codegen,
    Runtime,
    None
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
    ReturnOutsideFunction,

    // Classes & OOP
    UndefinedMember,
    CircularInheritance,
    InvalidConstructor,
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
    NoEntryPoints,

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

    // ???
    InvalidConsoleArgument
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
    int line, column = 1;
    int len = 0;
    std::string filePath;
    ErrorSpan(const std::string& filePath, const std::string& value, int line, int column): filePath(filePath), len((int)value.length()), line(line), column(column) {};
};

struct Error {
    ErrorType type;
    std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType;
    ErrorSpan span;

    std::string messageKey; // Message explaining what's wrong, takes localization key
    std::vector<std::string> messageArgs; // Arguments to make message more precise
    std::string hintKey; // Hint to fix the error, takes localization key
    std::vector<std::string> hintArgs;// Arguments to make hints more precise
};

struct ErrorManager {
    std::vector<Error> errors;

    void addError(ErrorType type, std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType, const ErrorSpan& span, const std::string& messageKey, std::vector<std::string> messageArgs = {}, const std::string& hintKey = "", std::vector<std::string> hintArgs = {}) { errors.push_back(Error{type, detailedType, span, messageKey, messageArgs, hintKey, hintArgs}); }
    void printErrors();
    [[nodiscard]] bool hasErrors() const { return !errors.empty(); }

    static std::string formatErrorType(std::variant<SyntaxErrors, AnalysisErrors, PreprocessorErrors, CodegenErrors, RuntimeErrors> detailedType);
};