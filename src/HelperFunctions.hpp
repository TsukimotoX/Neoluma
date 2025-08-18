#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <iostream>
#include <format>
#include <filesystem>

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

// Other
inline std::string trim(std::string s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

template<typename T>
T input(const std::string& prompt = "") {
    std::string line;
    if (!prompt.empty()) std::cerr << prompt;
    std::getline(std::cin, line);

    if constexpr (std::is_same<T, std::string>::value) {
        return line;
    } else {
        std::istringstream iss(line);
        T val;
        iss >> val;
        return val;
    }
}
