#ifndef HELPERFUNCTIONS_HPP
#define HELPERFUNCTIONS_HPP

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <iostream>
#include <format>
#include <sstream>

class ASTNode;

// Memory functions

// MemoryPtr is a unique pointer that automatically manages the memory of the object it points to.
template<typename T>
using MemoryPtr = std::unique_ptr<T>;

// makeMemoryPtr is a function that creates a MemoryPtr for the given type and forwards the arguments to its constructor.
template <typename T, typename... Args>
MemoryPtr<T> makeMemoryPtr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// SharedPtr is a shared pointer that allows multiple pointers to share ownership of an object.
template<typename T>
using SharedPtr = std::shared_ptr<T>;

// makeSharedPtr is a function that creates a SharedPtr for the given type and forwards the arguments to its constructor.
template <typename T, typename... Args>
SharedPtr<T> makeSharedPtr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Reinterprets element in () to element in <>.
template<typename T>
T* as(MemoryPtr<ASTNode>& node) {
    return dynamic_cast<T*>(node.get());
}


// Data types functions

// just a string. what else did you expect?
using string = std::string;

// array = vector. it's a simple alias for std::vector<T> to make it more readable and consistent with other languages.
template<typename T>
using array = std::vector<T>;

// Other
template <typename... Args>
void print(std::format_string<Args...> output, Args&&... args) {
    std::cout << std::format(output, std::forward<Args>(args)...) << "\n";
}

template <typename... Args>
void printerr(std::format_string<Args...> output, Args&&... args) {
    std::cerr << std::format(output, std::forward<Args>(args)...) << "\n";
}

template<typename T>
T input(const std::string& prompt = "") {
    T val;
    std::string line;
    if (!prompt.empty()) std::cerr << prompt;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    iss >> val;
    return val;
}

#endif