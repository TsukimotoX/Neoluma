#pragma once

#include <iostream>
#include <vector>

template<typename T>
using MemoryPtr = std::unique_ptr<T>;

template<typename T, typename... Args>
MemoryPtr<T> makeMemoryPtr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename U>
MemoryPtr<T> as(MemoryPtr<U> ptr) {
    if constexpr (std::is_polymorphic_v<U> && std::is_base_of_v<T, U>) return dynamic_cast<MemoryPtr<T>>(ptr);
    else return static_cast<MemoryPtr<T>>(ptr);
}

// Other

std::string trim(const std::string& s);

// Splits the string by delimeter
std::vector<std::string> split(std::string str, char delimiter);

// Reads the file
std::string readFile(const std::string& filePath);
