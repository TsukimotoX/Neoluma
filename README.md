# Neoluma

**Neoluma** is a high-level, all-purpose programming language designed to be a language for everything. Whether you're writing a small script or building an entire operating system, Neoluma is made to scale with you. With a Python-like syntax and C#/C++-inspired architecture, it's both expressive and powerful.

---

## Features

- **Modern, readable syntax** inspired by Python and C#
- **Statically typed** with optional type inference
- **Custom fixed-point `number` type**, plus `int`, `float`, `bool`, `string`, `set`, `dict`, `array`, `result`, `void`
- **Annotations and decorators**: `@entry`, `@comptime`, `@unsafe`, `@override`, etc.
- **Preprocessor directives**:
  - `#use` — for external libraries and non-Neoluma files (e.g., C++ bindings)
  - `#import` — for local `.nm` Neoluma modules
  - `#baremetal` for working with bare-metal and etc.
- **Async/await** built into the language
- **Compile-time evaluation**, lambdas, and macro system (WIP)
- **Automatic and manual memory management**
- **Cross-platform design** with potential for OS development
- **LLVM backend** to compile into fast, native machine code
- **Planned IDE integration** with syntax highlighting and debugging

---

## File Formats

- `.nm` — **Neoluma Module**: source code
- `.nlp` — **Neoluma Project**: project structure/configuration

```
project_name/
├── main.nm
├── utils.nm
└── project.nlp
```

---

## Example

```neoluma
#import "std/io"

@entry
function main() {
    name = "Neoluma";
    print("Hello, {name}!")
}
```
> `main()` is `void` by default — no `return` required.

---

## Usage (planned CLI)

```bash
neoluma build project.nlp #compiled way
neoluma run main.nm #interpreted way
```

---

## Roadmap

- [x] Lexer
- [x] Parser (~70%)
- [ ] Semantic Analyzer
- [ ] LLVM IR Compiler
- [ ] CLI Toolchain
- [ ] Cross-platform support (Linux / Windows / macOS / Android / iOS / etc.)
- [ ] Plugin system and editor integration
- [ ] Transpilation support (C++, JS, etc.)

---

## Vision

Neoluma aims to be a unified language that bridges low-level control and high-level productivity. It is designed for:

- Systems programming (including OS kernels)
- Cross-platform apps and utilities
- Educational tools and experimentation
- Scripting with performance
- Future self-hosting development

---

## Status

Neoluma is in early development. Contributions and feedback are welcome!

---

**Created by [Tsukimoto](https://github.com/TsukimotoX) under [Apache License](./LICENSE)** — more documentation and compiler features coming soon!
