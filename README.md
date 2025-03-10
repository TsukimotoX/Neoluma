# Neoluma

**Neoluma** is a high-level, all-purpose programming language designed to be a language for everything.
Whether you're building a simple script or a complex operating system, Neoluma is here to help. With a syntax inspired by Python and C#, it’s both easy to learn and powerful enough for serious development.

## Features

- **Simple Syntax**: Neoluma's syntax is intuitive, combining the best elements of Python's readability and C#'s expressiveness.
- **LLVM Backend**: Neoluma is built on top of LLVM, making it capable of compiling to high-performance machine code. This means it can be used for everything from rapid prototyping to creating performance-critical applications.
- **Memory Management**: With automatic garbage collection and manual memory allocation options, Neoluma offers flexibility to suit different needs.
- **Async and Await**: Asynchronous programming made easy with built-in async/await support.
- **Type Safety**: Includes basic types like `int`, `float`, with additional options to control data size for more efficient memory usage (e.g., `int[8]` or `int[16]`).
- **Macros**: Use macros to simplify your code and make it more reusable and modular.
- **Interoperability**: Easily integrate with existing C++ libraries using the `#use` directive, allowing you to access external functions or even entire libraries.
- **Cross-Platform**: Designed to work across platforms, Neoluma can be used for everything from small applications to full-fledged operating systems.

## Installation

Neoluma is still in development, no tutorial for now.

# Usage

Once the environment is set up, you can start coding in Neoluma by creating ```.nl``` files and running them through the Neoluma compiler.

Example:
```
// Hello, World in Neoluma
print("Hello, World!"); //<- semicolons are optional
```
To run it:
```bash
neoluma hello.nl
```

# Contributing

Neoluma is an open-source project, and I welcome contributions! If you have ideas, bug fixes, or new features you’d like to add, feel free to open an issue or submit a pull request.
	•	Fork the repository
	•	Create your feature branch (git checkout -b feature/my-feature)
	•	Commit your changes (git commit -am 'Add new feature')
	•	Push to the branch (git push origin feature/my-feature)
	•	Open a pull request

# License

This project is licensed under the [Apache License 2.0](./LICENSE).

# Roadmap
	•	Complete language features (macros, decorators, etc.)
	•	Add full cross-platform support (including Windows, Linux and macOS)
	•	Develop the Neoluma plugin extension with built-in syntax highlighting and debugging tools
	•	Expand standard library to include more common utilities and functions tools
	•	Add compatibility with C++ and other languages using #use keyword.

Stay tuned for more updates and releases!

⸻

Neoluma is designed to be a powerful, easy-to-use programming language that can do everything. From everyday tasks to building complex software, Neoluma is here to help make development smoother, faster, and more efficient.
