#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <type_traits>

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
    static_assert(std::is_base_of_v<U, T>, "[Neoluma/HelperFunctions] as<T>(ptr): T must derive from U");

    if constexpr (std::is_polymorphic_v<U>) {
        if (T* casted = dynamic_cast<T*>(ptr.get())) {
            ptr.release();
            return MemoryPtr<T>(casted);
        }
        return nullptr;
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

// String formatting
template<typename T>
std::string anyToStr(T&& v) {
    std::ostringstream oss;
    oss << std::forward<T>(v);
    return oss.str();
}

// an impl function so i don't have to stupidly duplicate the formatStr code
std::string formatStrVec(const std::string& fmt, const std::vector<std::string>& collectedArgs);

template<typename... Args>
std::string formatStr(const std::string& fmt, Args&&... args)
{
    std::vector<std::string> collectedArgs = { anyToStr(args)... };
    return formatStrVec(fmt, collectedArgs);
}