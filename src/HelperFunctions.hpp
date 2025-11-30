#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

template<typename T>
using MemoryPtr = std::unique_ptr<T>;

template<typename T, typename... Args>
MemoryPtr<T> makeMemoryPtr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
MemoryPtr<T> makeSharedPtr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename U>
MemoryPtr<T> as(MemoryPtr<U> ptr) {
    if constexpr (std::is_polymorphic_v<U> && std::is_base_of_v<T, U>) {
        T* casted = dynamic_cast<T*>(ptr.release());
        return MemoryPtr<T>(casted);
    } else {
        return MemoryPtr<T>(static_cast<T*>(ptr.release()));
    }
}

// Other

std::string trim(const std::string& s);

// Splits the string by delimeter
std::vector<std::string> split(std::string str, char delimiter);

// Reads the file
std::string readFile(const std::string& filePath);

// Extracts the file name from a given file path
std::string getFileName(const std::string& filePath);
