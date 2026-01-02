#pragma once

/*
## **Comprehensive Semantic Error Categories for Neoluma**

### **1. Variable & Declaration Errors**
- **Undefined Variable** (`UndefinedVariable` - already in your ErrorManager)
  - Using a variable that hasn't been declared
  - Accessing variables outside their scope

- **Redefined Variable** (`RedefinedVariable` - already in your ErrorManager)
  - Declaring the same variable twice in the same scope
  - Parameter name conflicts with local variables
  - Shadowing issues in nested scopes

- **Uninitialized Variable Usage**
  - Using a non-nullable variable that was declared but not initialized
  - Reading from variables before assignment in control flow paths

- **Constant Reassignment**
  - Attempting to modify a `const` variable after initialization
  - Modifying parameters marked as const

- **Invalid Nullable Usage**
  - Using a nullable variable without null-checking
  - Assigning null to non-nullable variables

### **2. Type System Errors**
- **Type Mismatch** (`TypeMismatch` - already in your ErrorManager)
  - Assignment type incompatibility (e.g., `string x = 5`)
  - Binary operation type conflicts (e.g., `"hello" + 42`)
  - Function argument type mismatch
  - Return value type doesn't match function signature

- **Unknown Type** (`UnknownType` - already in your ErrorManager)
  - Referencing undefined types in declarations
  - Invalid type annotations

- **Invalid Cast** (`InvalidCast` - already in your ErrorManager)
  - Incompatible type conversions
  - Downcasting without proper checks

- **Array/Collection Type Errors**
  - Array element type mismatch with declared type hint
  - Set duplicate type conflicts
  - Dictionary key/value type violations
  - Indexing non-indexable types

- **Result Type Errors**
  - Not handling both success and error cases
  - Unwrapping Result without checking isError flag

### **3. Function & Parameter Errors**
- **Function Mismatch** (`FunctionMismatch` - already in your ErrorManager)
  - Calling undefined functions
  - Wrong number of arguments
  - Missing required parameters
  - Extra arguments provided

- **Invalid Return Type** (`InvalidReturnType` - already in your ErrorManager)
  - Return statement type doesn't match declared return type
  - Missing return statement in non-void function
  - Return statement in void function with value
  - Unreachable code after return

- **Parameter Errors**
  - Required parameter after optional parameter
  - Duplicate parameter names
  - Invalid default value types

- **Lambda Errors**
  - Lambda parameter count mismatch when used
  - Lambda return type inference failures

### **4. Class & Object-Oriented Errors**
- **Undefined Member Access**
  - Accessing non-existent class fields/methods
  - Member access on non-object types

- **Inheritance Errors**
  - Circular inheritance detection
  - Inheriting from non-class types (e.g., interfaces used as superclass)
  - Multiple inheritance conflicts
  - Abstract method not implemented

- **Constructor Errors**
  - Multiple constructors defined
  - Constructor with wrong name
  - Invalid `super()` call in constructor
  - Missing required super() call

- **Access Modifier Violations**
  - Accessing private members outside class
  - Accessing protected members from non-subclass
  - Invalid modifier combinations

- **Override Errors**
  - Override modifier on non-overriding method
  - Method signature mismatch with parent

### **5. Modifier & Decorator Errors**
- **Invalid Modifier Usage** (`InvalidModifierUsage` - already in your ErrorManager)
  - Conflicting modifiers (e.g., `public private`)
  - Invalid modifier for context (e.g., `static` on local variable)
  - Multiple access modifiers (public/private/protected)
  - `async` on non-function
  - `override` on non-method

- **Decorator Misuse** (`DecoratorMisuse` - already in your ErrorManager)
  - Decorator applied to invalid target (e.g., `@Entry` on variable)
  - Decorator with wrong arguments
  - Multiple incompatible decorators
  - Undefined decorator

### **6. Control Flow Errors**
- **Break/Continue Outside Loop**
  - `break` or `continue` not inside for/while loop
  - Used in wrong context (e.g., in switch but meant for loop)

- **Unreachable Code**
  - Code after return/throw/break/continue
  - Dead branches (always false conditions)

- **Throw Without Try-Catch**
  - Uncaught exceptions in non-throwing context
  - Invalid throw expression type

- **Switch Statement Errors**
  - Duplicate case values
  - Non-constant case expressions
  - Case type doesn't match switch expression type
  - Missing default case (when required)

### **7. Import & Module Errors**
- **Import Not Found** (`ImportNotFound` - already in your ErrorManager)
  - Module doesn't exist
  - Invalid import path

- **Circular Import**
  - Module imports itself directly or indirectly

- **Import Type Errors**
  - Foreign import without proper language pack
  - Relative import outside module context

- **Alias Conflicts**
  - Import alias conflicts with existing identifier

### **8. Preprocessor Errors**
- **Invalid Directive** (`InvalidDirective` - already in your ErrorManager)
  - Unknown preprocessor directive
  - Misplaced preprocessor directive

- **Macro Error** (`MacroError` - already in your ErrorManager)
  - Undefined macro
  - Macro expansion errors

- **Unsafe Context Violation**
  - Unsafe operations without `#unsafe` directive
  - Baremetal features without `#baremetal`

### **9. Interface & Enum Errors**
- **Interface Implementation Errors**
  - Class doesn't implement all interface methods
  - Method signature mismatch with interface
  - Interface field type mismatch

- **Enum Errors**
  - Duplicate enum member names
  - Invalid enum member value type
  - Enum value out of range

### **10. Expression Errors**
- **Invalid Unary Operation**
  - Unary operator on incompatible type (e.g., `!5`)
  - Post/pre increment on non-numeric type

- **Invalid Binary Operation**
  - Incompatible operand types
  - Division/modulo by zero (constant folding)
  - Invalid comparison operators on incompatible types

- **Assignment to Non-LValue**
  - Assigning to literals or constants
  - Assigning to function calls
  - Invalid compound assignment target

- **Member Access Errors**
  - Chained member access on null/undefined
  - Accessing members on primitive types incorrectly

### **11. Scope & Lifetime Errors**
- **Variable Out of Scope**
  - Accessing block-local variable outside block
  - Forward reference before declaration

- **Capture Errors in Lambdas**
  - Capturing variables that don't exist
  - Modifying captured const variables

### **12. Special Feature Errors**
- **Async/Await Errors**
  - `await` outside async function
  - Async function not properly handled

- **Yield Errors**
  - `yield` outside generator function

- **Debug Modifier Misuse**
  - Debug-only code in production context

---

## **Implementation Suggestions**

For your semantic analyzer, you'll need:

1. **Symbol Table** - Track variables, functions, classes in scopes
2. **Type Checker** - Validate type compatibility across operations
3. **Scope Manager** - Handle nested scopes for blocks, functions, classes
4. **Control Flow Analyzer** - Track reachability and proper break/continue usage
5. **AST Walker** - Traverse your AST nodes systematically

*/