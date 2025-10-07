#include "../token.hpp"
#include <vector>

enum struct ErrorType {
    Syntax,
    Semantic,
    Type,
    Preprocessor,
    Codegen,
    Runtime,
};

enum struct SyntaxErrors {
    UnexpectedToken,
    MissingToken,
    InvalidStatement,
    InvalidPreprocessorDirective,
};

enum struct SemanticErrors {
    UndefinedVariable, 
    RedefinedVariable,
    FunctionMismatch,
    InvalidReturnType,
    InvalidModifierUsage,
    DecoratorMisuse,
};

enum struct TypeErrors {
    TypeMismatch,
    UnknownType,
    InvalidCast,
    LiteralOverflow,
};

enum struct PreprocessorErrors {
    ImportNotFound,
    MacroError,
    InvalidDirective,
};

enum struct CodegenErrors {
    LLVMGenerationError,
    UnsupportedFeature,
};

enum struct RuntimeErrors {
    DivisionByZero,
    NullReference,
    IndexOutOfBounds,
    FloatingPointError,
};

struct Error {
    ErrorType type;
    Token token;
    std::string message;
    std::string hint;
};

class ErrorManager {
public:
    void addError(ErrorType type, const Token& token, const std::string& msg, const std::string& hint = "") { errors.push_back(Error{type, token, msg, hint}); }
    void printErrors(const std::string& source);
private:
    std::vector<Error> errors;
};