#include "HelperFunctions.hpp"
#include "Libraries/color/color.hpp"
#include <print>

std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && isspace((unsigned char)s[start])) start++;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

// Splits the string by delimeter
std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::string current;
    for (char c : str) {
        if (c == delimiter) {
            result.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(trim(current));
    return result;
}

// Reads the file
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::println(std::cerr, "{}[Neoluma/HelperFunctions] Failed to open file: {}", Color::TextHex("#ff5050"), filePath);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
} 