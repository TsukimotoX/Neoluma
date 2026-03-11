#include "HelperFunctions.hpp"
#include "Libraries/Color/Color.hpp"
#include <print>
#include <fstream>
#include <sstream>

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

// Extracts the file name from a given file path
std::string getFileName(const std::string& filePath) {
    return std::filesystem::path(filePath).filename().replace_extension("").string();
}

std::string formatStrVec(const std::string& fmt, const std::vector<std::string>& collectedArgs) {
    std::string out;
    size_t argCount = 0;
    for (size_t i = 0; i < fmt.size(); i++) {
        char c = fmt[i];
        if (c == '{') {
            if (i + 1 >= fmt.size()) throw std::runtime_error("[HelperFunctions/formatStr] Invalid '{'");
            char next = fmt[i + 1];
            if (next == '{') { out += '{'; i++; }
            else {
                size_t j = i + 1;

                // {} case
                if (fmt[j] == '}') {
                    if (argCount >= collectedArgs.size())
                        throw std::runtime_error("[HelperFunctions/formatStr] Not enough format arguments");

                    out += collectedArgs[argCount++];
                    i = j;
                }
                // {number} case
                else if (std::isdigit(fmt[j])) {

                    size_t index = 0;

                    while (j < fmt.size() && std::isdigit(fmt[j])) {
                        index = index * 10 + (fmt[j] - '0');
                        j++;
                    }

                    if (j >= fmt.size() || fmt[j] != '}')
                        throw std::runtime_error("[HelperFunctions/formatStr] Invalid positional format");

                    if (index >= collectedArgs.size())
                        throw std::runtime_error("[HelperFunctions/formatStr] Positional argument out of range");

                    out += collectedArgs[index];
                    i = j;
                }
                else {
                    throw std::runtime_error("[HelperFunctions/formatStr] Invalid '{'");
                }
            }
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