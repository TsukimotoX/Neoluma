# Neoluma Specification

I need to polish this mess somehow. Here's the specification describing the language from a technical perspective. Let's keep it sharp, but not too sharp—just enough to make sense.

---

## 🔹 Data Types

Neoluma supports a variety of data types to handle different scenarios. Here's the breakdown:

| Type      | Description                                                                 |
|-----------|-----------------------------------------------------------------------------|
| `int`     | Represents integers (whole numbers), including negatives.                  |
| `uint`    | Represents unsigned integers (whole numbers ≥ 0).                          |
| `float`   | A floating-point number based on the IEEE 754 standard.                    |
| `number`  | A high-precision number type using base and exponent (10^x). Ideal for accurate math. |
| `string`  | A text string. Supports inline interpolation with `${expression}`.         |
| `bool`    | Boolean value (`true` or `false`). Implicit conversion: `0` is `false`, non-zero is `true`. |
| `array`   | Ordered collection of elements. Can mix types.                             |
| `set`     | Unordered collection of unique values.                                     |
| `dict`    | Key-value map (dictionary). Keys are unique; values can be any type.       |
| `void`    | Represents the absence of a value. Typically used as a return type.        |
| `result`  | A wrapper for success or error values, like `try-catch`.                   |

### Nullable Types
Values can be written with `?` to the right of them, like `int?` or `string?`, to mark that they are nullable. Nullable types can hold either a value or `null`.

### Variable Sizes
Sizes of variables can be defined with `[<number>]`, like `x: int[8] = 255;`. By default, Neoluma dynamically scales variable sizes unless explicitly specified.

### Examples
```neoluma
// Neoluma does NOT have let/var. Ever. It's declared without it.

// Numeric types
a: int = 10
b: float = 3.14
c: number = 1.23e10

// Strings
name: string = "Tsuki"
greeting: string = "Hello, ${name}!"

// Booleans
flag: bool = true
isReady: bool = 0  // Implicitly false

// Collections
nums: array = [1, 2, 3, "four"]
unique: set = (1, 2, 2, 3)  // Contains 1, 2, 3
info: dict = {"name": "Tsuki", "age": 18}

// Nullable types
age: int? = null

// Functions
fn greet(person: dict) -> void {
    print("Hello, ${person['name']}!")
}

res: result = greet(info)
```

---

## 🔹 Functions

Functions in Neoluma are declared using either `fn` or `function` (if `verbose=true` in the compiler settings). They encapsulate logic and make your code reusable.

### Basic Syntax
```neoluma
fn add(a: int, b: int) -> int {
    return a + b
}
```
- Use `-> <type>` to specify the return type.
- Semicolons are optional but recommended (Neoluma accepts either semicolons or newlines).
- Functions without a return type default to `void`.

### One-Liners
For simple expressions, use `=>`:
```neoluma
fn subtract(a, b) -> int => a - b
```

### Modifiers
Functions can have visibility modifiers:
```neoluma
public fn greet(name: string) {
    print("Hello, ${name}!")
}

protected fn calculate() -> int {
    return 42
}

private fn secret() {
    print("This is private")
}
```
- `public`: Accessible from anywhere.
- `protected`: Accessible within the class and its subclasses.
- `private`: Accessible only within the class.

### Lambdas
Inline functions can be written as:
```neoluma
(x, y) => x + y
// or if you're on verbose mode:
lambda x, y: x + y
```

### Docstrings
Functions can have documentation above them using `///`:
```neoluma
/// Adds two numbers
fn add(a: int, b: int) -> int {
    return a + b
}
```
- Use `///` for single-line comments.
- For multiline comments, use consecutive `///` lines.

### Decorators
Decorators modify function behavior or add metadata. Use `@`:
```neoluma
@entry
fn main() {
    print("Program started")
}

@unsafe
fn riskyOperation() {
    // Low-level code
}

@comptime
fn compileTimeFunction() {
    print("This runs at compile time!")
}

@float
fn calculateArea(radius: float) -> float {
    return 3.14 * radius * radius
}
```
- `@entry`: Marks the entry point of the program.
- `@unsafe`: Allows unsafe or raw memory operations.
- `@comptime`: Executes the function at compile time.
- `@float`: Forces float usage on a function or value.

---

## 🔹 Classes and Inheritance

Neoluma uses a class-based object-oriented model inspired by C# and JavaScript.

### Declaring Classes
```neoluma
class Animal {
    fn speak() {
        print("Sound")
    }
}
```
- Classes can have methods, fields, and constructors.

### Constructors
```neoluma
class Dog {
    init(name: string) {
        self.name = name
    }
}
```
- `self` refers to the current instance.

### Inheritance
Use `<-` to inherit:
```neoluma
class Dog <- Animal {
    override fn speak() {
        print("Bark")
    }
}
```
- The `override` keyword is used to redefine methods from the parent class.

### Static Methods
```neoluma
class Utils {
    static fn add(a, b) -> int {
        return a + b
    }
}

Utils.add(1, 2)
```
- Static methods belong to the class, not instances.

---

## 🔹 Control Flow



### Conditionals
```neoluma
if (x > 10) {
    print("Greater than 10")
} else {
    print("10 or less")
}
```
- Conditions are written in parentheses `(...)`.
- Blocks are enclosed in `{}`.

### Switch
```neoluma
switch (value) {
    case 1:
        print("One")
    case 2:
        print("Two")
    default:
        print("Other")
}
```
- No `break` needed; no fallthrough.

**Why No `match`?**
Neoluma does not include a separate `match` expression. Instead, its functionality is fully covered by the `switch` statement. Here’s why:
- `switch` supports constants and strings.
- It avoids ambiguous behaviors like fallthrough by default.
- It’s cleaner and easier to parse than pattern-matching syntax.

This keeps the language simpler and easier to learn. (It’s already painful to shape the language together when you try to please everyone. This here is a compromise.)

### Loops
```neoluma
for (i in range(10)) {
    print(i)
}

for (item : items) {
    print(item)
}

while (condition) {
    // code
}
```
- `for` loops support both range-based and collection-based iteration.

### Try-Catch
```neoluma
try {
    riskyFunction()
} catch (e) {
    print("Error: ${e}")
}

fn riskyFunction() {
    throw "Something went wrong"
}
```
- `try`: Defines a block of code where exceptions may occur.
- `catch`: Handles exceptions thrown in the `try` block.
- `throw`: Used to raise an exception.

---

## 🔹 Modules and Language Packs

### What Are Language Packs?
Language Packs allow seamless integration of external languages (like C++, Python, Rust, etc.) into Neoluma. They avoid FFI by enabling external code to be parsed and compiled natively via AST generation. Think of them as compiler plugins written in Neoluma itself.

### Why Language Packs?
- **Transparency**: External code is treated as if it were native.
- **Flexibility**: Language Packs define their own lexer and parser logic.
- **Performance**: Transpilation happens at compile-time, avoiding runtime overhead.

### Example
```neoluma
#import "python:requests" as req
```
- The compiler uses the `.neopack` logic to parse the external module.
- The Language Pack returns a `ModuleNode` inserted into the project AST.

### How It Works
1. The compiler finds the language pack registered in `[language-packs]` of the `.nlp` file.
2. It executes the `.neopack` logic (e.g., `main.nm`) to parse the external code.
3. The Language Pack returns a `ModuleNode`, which is inserted into the project AST.

### Registering Language Packs
In your `.nlp` project file:
```toml
[language-packs]
python = "./packs/python/python.neopack"
cpp = "./packs/cpp/cpp.neopack"
```

---
