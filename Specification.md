# Neoluma specification

I need to polish this mess somehow. Here's the specification describing the language from technical perspective.

## 🔹 Data types

| Type | Description |
| ----- | ---------------------|
| `int` | 	Represents an integer (whole number), including negatives. |
| `float` | A floating-point number based on IEEE 754 standard. |
| `number` | A custom high-precision number type. Instead of IEEE 754, it stores base and exponent (10^x), making it suitable for accurate math. Uses extra memory. |
| `string` | A text string. Supports inline interpolation using `${expression}` syntax. |
| `bool` | Boolean value. Can be either `true` or `false`. Also supports implicit conversion: `0` is `false`, and any non-zero value is `true`. |
| `array` | An ordered collection of elements. Can contain mixed types. |
| `set` | An unordered collection of unique values. |
| `dict` | A key-value map (dictionary). Keys are unique, values can be any type. |
| `void` | 	Represents the absence of a value. Typically used as a return type of functions. Optional to use. |
| `result` | Special result wrapper that acts like a `try-catch`. Contains either a successful value or an error. |

Values can be written with `?` to the right of them, like `int?` or `string?`, to mark that it's nullable.

```neoluma
// Numeric
a: int = 10
b: float = 3.14
c: number = 1.23e10

// Strings
name: string = "Tsuki"
greeting: string = "Hello, ${name}!"

// Booleans
flag: bool = true
isReady: bool = 0  // false, since 0 means false

// Arrays, sets, dicts,
nums: array = [1, 2, 3, 4.5, "six"]
uniqueItems: set = (1, 2, 2, 3)  // Set will contain 1, 2, 3

person: dict = {
    "name": "Tsuki",
    "age": 18
}

// Function with strict typing in arguments (types are optional)
fn printAge(p: dict) -> void {
    print("Age: ${p['age']}")
}

// Either returns print("Age: ${p['age']}") or error message
result: result = printAge(person);
```

---

## 🔹 Functions

Functions in Neoluma are declared using either `fn` or `function` keyword.

- `fn` is the primary keyword.
- `function` is syntactic sugar and only enabled if `verbose = true` in the compiler settings.

### Basic syntax

```neoluma
fn add(a: int, b: int) -> int {
    return a + b;
}
```
Use `-> <type>` to specify the return type.
Semicolons (`;`) are optional at the end of expressions but recommended.

Functions with `void` do not require writing it, it will be assigned `void` by default. But if you feel like it, use `-> void`. 

### One-line functions

Neoluma supports single-expression functions using the `=>` syntax:

```neoluma
fn subtract(a, b) -> int => a - b
```

### Access modifiers

Functions can be prefixed with visibility modifiers:

```neoluma
public fn greet(name: string) {
    print("Hello, ${name}!");
}
```

### Lambdas (inline functions)

Lambdas can be declared using either:

```neoluma
lambda x, y: x + y
```

Or:

```neoluma
(x, y) => x + y
```

Both are valid and can be used interchangeably. Braces `{}` can be used for multiline lambdas.

### Docstrings

Functions can have documentation above them using `///`:

```neoluma
/// Adds two numbers
fn add(a: int, b: int) -> int {
    return a + b;
}
```

Multiline docstrings use consecutive lines of `///`.

### Decorators

Functions can be annotated with decorators to modify their behavior or provide metadata. Decorators are applied using the `@` symbol before the function declaration.

Common decorators include:
- `@entry` — Marks the entry point of the program.
- `@override` — Indicates a method overrides a base class method.
- `@comptime` — Marks a function to be executed at compile time.
- `@unsafe` — Marks a function as unsafe.

**Decorator Syntax Example**
```neoluma
@entry
public fn main() -> void {
    print("Program started")
}

@override
public fn speak() {
    print("Overridden method")
}

@comptime
fn compileTimeFunction() {
    // code executed during compilation
}

@unsafe
fn unsafeOperation() {
    // low-level unsafe code here
}
```

---

## 🔹 Classes and Inheritance

Neoluma uses a class-based object-oriented model inspired by C# and JavaScript.

### Class declaration

Use the `class` keyword to define a class:

```neoluma
class Animal {
    fn speak() {
        print("Sound");
    }
}
```
- Classes may contain methods, fields, and constructors.
- Methods inside classes are written the same way as global functions.

### Constructors
Constructors are declared with the `init()` method:

```neoluma
class Dog {
    init(name: string) {
        self.name = name;
    }
}
```

- `self` refers to the current instance (like this in other languages).
- There can only be one constructor per class.

### Inheritance

Neoluma uses the `<-` symbol to define inheritance:

```neoluma
class Dog <- Animal {
    override fn speak() {
        print("Bark");
    }
}
```
- The subclass inherits all methods and properties from the parent class.
- Use the `override` keyword to override a parent method.

### Access modifiers

Each method or field can be preceded with one of the visibility modifiers:

| Modifier | Description |
| -------- | ----------- |
| `public` | Accessible from anywhere |
| `protected` | Accessible from this class and subclasses |
| `private` | Accessible only within the class |

Example:

```neoluma
class Person {
    private name: string;
    public fn getName() -> string {
        return self.name;
    }
}
```
### Static methods

Use the `static` keyword to declare static methods:

```neoluma
class Utils {
    static fn add(a, b) {
        return a + b;
    }
}
```
Static methods can be called without creating an instance:

```neoluma
Utils.add(1, 2);
```

### Abstract and polymorphic behavior

Neoluma supports abstract base classes using interfaces and override logic via `override`:

``` neoluma
interface Drawable {
    fn draw();
}

class Circle <- Drawable {
    override fn draw() {
        print("Drawing a circle");
    }
}
```

Interfaces do not contain implementations, only method signatures.

---

## 🔹 Control Flow

Neoluma provides flexible and readable control structures inspired by JavaScript and Python.

### `if` / `else` conditionals

```neoluma
if (x > 10) {
    print("Greater than 10");
}
else {
    print("10 or less");
}
```
- Conditions are written in parentheses `(...)`.
- Blocks are enclosed in `{}`.
- You may omit the `else` block.

### `switch` statement

```neoluma
switch (value) {
    case 1:
        print("One");
    case 2:
        print("Two");
    default:
        print("Other");
}
```
- Each `case` must be followed by a block or single statement.
- No `break` is required after each case — cases do not fall through.
- `default` is optional and handles unmatched values.

### `for` loops

Neoluma supports two for loop syntaxes:

### Traditional `for` (verbose or strict mode)
```neoluma
for (i in range(10)) {
    print(i);
}
```
- The `range(n)` function generates values from `0` to `n - 1`.

### Type-based `for` loop (default)
```neoluma
for (item : items) {
    print(item);
}
```
- Uses `:` as a short form for iterating over collections.
- Equivalent to `for item in items` in Python.
- `in` is also supported when `verbose = true` in compiler settings.

### `while` loops
```neoluma
while (condition) {
    // code
}
```
- Executes the block while the condition remains `true`.

### Loop control keywords
| Keyword | Description |
| ------- | ----------- |
| `break` | Exits the current loop immediately. |
| `continue` | Skips to the next iteration. |

Example:

```neoluma
for (i: items) {
    if (i == 0) continue;
    if (i > 10) break;
    print(i);
}
```

### Why not `match`?

Neoluma does not include a separate `match` expression in `verbose` mode, as it's quite different from functionality of `switch` in other languages. 

But, its functionality is fully covered by the `switch` statement.

- `switch` supports constants and strings
- It avoids ambiguous behaviors like fallthrough by default
- Cleaner and easier to parse than pattern-matching syntax

This keeps the language simpler and easier to learn.

(it's already painful to shape the language together, when you try to please everyone. this here is a compromise.)

### Exception Handling: try, catch, throw

Neoluma supports structured exception handling with `try`, `catch`, and `throw` keywords.

```neoluma
try {
    // risky code
    mayThrowError()
} catch (e) {
    print("Caught error: ${e}")
}

fn mayThrowError() {
    throw "Something went wrong"
}
```
- `try` — Block of code, where the exception can happen
- `catch` — Block of handling the exception.
- `throw` — Operator of throwing the exception.

### Generators and yield

Neoluma supports generators using `yield` inside functions to produce values lazily.

```neoluma
fn countUpTo(max: int) -> array {
    i = 0
    while (i < max) {
        yield i
        i += 1
    }
}

for (n in countUpTo(5)) {
    print(n)  // prints 0,1,2,3,4
}
```
- Functions that use yield return a generator.
- `yield` produces values one at a time, enabling iteration over sequences without creating a full collection.

--- 

## 🔹 Preprocessors and Decorators

Neoluma supports **preprocessor directives** and **decorators**, offering powerful compile-time behaviors and metadata annotations.

---

### 🛠️ Preprocessor Directives

Preprocessors begin with `#` and affect compilation logic.

#### Built-in directives:

```neoluma
#unsafe        // Marks the file or section as unsafe (allows low-level features)
#baremetal     // Disables standard library; for minimal runtime
#import "math" // Imports a Neoluma module or namespace
#import "cpp:opengl" as gl // Imports a foreign module (e.g., from C++), optionally aliased
#macro         // Marks a macro definition
```

**Syntax**
```neoluma
#directive [value]
```
- Preprocessor lines must start with `#`.
- Strings `"..."` are used for paths or module names.
- `as alias` can be appended to rename imports.

**Notes**
- `#import` can load:
    - Neoluma standard modules (like `"math"`, `"memory"`)
    - external language modules via syntax: `"lang:module"` (e.g., `"cpp:openal"`)

    `from ... import ...` syntax is removed in favor of a cleaner and simpler `#import`.

### ✨ Decorators

Decorators begin with `@` and are placed above functions or variables to modify behavior or attach metadata.

**Built-in decorators:**
| Decorator | Description |
| --------- | ----------- |
| `@entry` | Marks the entry point of the program |
| `@unsafe` | Allows unsafe or raw memory operations |
| `@float` | Forces float usage on a function or value |
| `@comptime` | Executes the function at compile time |
| `@override` | Marks a method overriding a base class one |

### 🧬 Custom Decorators

Use the `decorator` keyword to define your own decorators:
```neoluma
decorator log {
    print("This function is being called");
}
```

You can then attach this decorator to a function:
```neoluma
@log
fn doStuff() {
    print("Running!");
}
```
- Decorators are run at compile time (unless declared otherwise).
- Inside a decorator block, you can access metadata or manipulate the AST (eventually).
- You can stack multiple decorators above a function.

---

## 🔹 Modules and Language Packs

Neoluma allows seamless integration of external languages (like C++, Python, Rust, etc.) using **Language Packs**, written entirely in Neoluma. This system avoids FFI, enabling external code to be parsed and compiled natively via AST generation.

### 📦 What is a `.neopack`?

A `.neopack` is a Language Pack project, similar to `.nlp`, containing the logic for parsing and converting external source code to Neoluma’s AST.
> Think of it as a compiler plugin written in Neoluma itself.

```
python.neopack
├── src/
│   └── main.nm
├── python.neopack  <-- Project file (like .nlp)
```

### 🧠 How does it work?

When a Neoluma project imports an external module:

```neoluma
#import "python:requests" as req
```
1. The compiler finds the language pack registered in `[language-packs]` of the `.nlp` file.
2. It executes the `.neopack` logic (e.g., `main.nm`) to **parse the external code**.
3. The Language Pack returns a `ModuleNode`, which is inserted into the project AST.

> The entry point function must be marked with `@entry` and must return a `ModuleNode`.

```neoluma
@entry
public fn parse(filePath: string) -> ModuleNode {
    // Load file, tokenize, parse, return full AST tree
}
```

### 📄 Registering in `.nlp`

In your `.nlp` project file, declare language packs like this:

```toml
[language-packs]
python = "./packs/python/python.neopack"
cpp = "./packs/cpp/cpp.neopack"

[language-packs.libraries.python]
requests = "./packs/python/libs/requests"
```
> You can also alias libraries:
```neoluma
#import "cpp:gl" as gl
```

### 🧩 What does a Language Pack contain?

A `.neopack` must define:
- A Neoluma function marked with `@entry`
- Lexer and parser logic inside `.nm` files
- Manual or semi-automated AST building
  
The pack is free to use internal helper modules or subfolders (like `src/`, `utils/`, etc.), but they are not required structurally.

### 🌀 Recursive Imports

Language packs can recursively parse other files (e.g., `#include`, `import`, etc.) and return a complete `ModuleNode` tree. The compiler treats external code as if it were native.

### ⚠️ Notes and Limitations
- Transpilation happens **at compile-time**, transparently to the user.
- Each pack defines its own lexer/parsing logic using Neoluma.
- For unsupported features (like C macros), the user must handle conversion.
- No FFI is used. External code is fully compiled into Neoluma’s LLVM pipeline.

---
