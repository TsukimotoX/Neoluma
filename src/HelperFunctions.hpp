#pragma once

#include <vector>
#include <string>
#include <sstream>
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

// String formatting
template<typename T>
std::string anyToStr(T&& v) {
    std::ostringstream oss;
    oss << std::forward<T>(v);
    return oss.str();
}

template<typename... Args>
std::string formatStr(std::string_view fmt, Args&&... args)
{
    std::vector<std::string> collectedArgs = { anyToStr(args)... };
    std::string out;
    size_t argCount = 0;
    for (size_t i = 0; i < fmt.size(); i++) {
        char c = fmt[i];
        if (c == '{') {
            if (i + 1 >= fmt.size()) throw std::runtime_error("[HelperFunctions/formatStr] Invalid '{'");
            char next = fmt[i + 1];
            if (next == '{') { out += '{'; i++; }
            else if (next == '}')
            {
                if (argCount >= collectedArgs.size()) throw std::runtime_error("[HelperFunctions/formatStr] Not enough format arguments");
                out += collectedArgs[argCount++];
                i++;
            }
            else throw std::runtime_error("[HelperFunctions/formatStr] Invalid '{'");
        }
        else if (c == '}')
        {
            if (i + 1 < fmt.size() && fmt[i + 1] == '}') { out += '}'; i++; }
            else throw std::runtime_error("[HelperFunctions/formatStr] Invalid '}'");
        }
        else out += c;
    }
    if (argCount < collectedArgs.size()) throw std::runtime_error("[HelperFunctions/formatStr] Too many format arguments");
    return out;
}