#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include "../Core/Extras/ProjectManager/projectmanager.hpp"
#include "../Libraries/toml/toml.hpp"
#include "../Libraries/asker/asker.hpp"

inline std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && isspace((unsigned char)s[start])) start++;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

// Splits the string by delimeter
inline std::vector<std::string> split(std::string str, char delimiter) {
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
inline std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
} 

// Returns progress bar for CLI
inline void showProgressBar(const std::string& stepName, int step, int total) {
    int percentage = (step * 100) / total;
    int hashes = percentage / 5;
    if (percentage != 100) std::println("{} [ {} ({}{}) {}% ] {}", Color::TextHex("#f6ff75"), stepName, std::string(hashes, '#'), std::string(20 - hashes, '_'), percentage, Color::TextHex("#01e0d4"));
    else std::println("{} [ {} ({}{}) {}%! ] {}", Color::TextHex("#75ff87"), stepName, std::string(hashes, '#'), std::string(20 - hashes, '_'), percentage, Color::TextHex("#01e0d4"));
}

// Clears terminal screen
inline void clearScreen() {
    std::cout << "\033[0m\033[2J\033[H";
}

// Asks a question
inline std::string askQuestion(const std::string& question) {
    return asker::input(question, true);
}

// -------- stuff required for parseProjectFile
inline std::string getString(const Toml::TomlTable& table, const std::string& key, const std::string& def = "") {
    for (const auto& [k, v] : table) {
        if (k == key && v.type == Toml::TomlType::String)
            return std::get<std::string>(v.value);
    }
    return def;
}

inline std::vector<std::string> getStringArray(const Toml::TomlTable& table, const std::string& key) {
    for (const auto& [k, v] : table) {
        if (k == key && v.type == Toml::TomlType::Array) {
            std::vector<std::string> result;
            const auto& arr = std::get<Toml::TomlArray>(v.value);
            for (const auto& item : arr) {
                if (item.type == Toml::TomlType::String)
                    result.push_back(std::get<std::string>(item.value));
            }
            return result;
        }
    }
    return {};
}

inline std::map<std::string, std::string> extractMap(const Toml::TomlTable& root, const std::string& key) {
    std::map<std::string, std::string> result;

    for (const auto& [k, v] : root) {
        if (k == key && v.type == Toml::TomlType::Table) {
            const auto& tbl = std::get<Toml::TomlTable>(v.value);
            for (const auto& [tk, tv] : tbl) {
                if (tv.type == Toml::TomlType::String)
                    result[tk] = std::get<std::string>(tv.value);
            }
        }
    }

    return result;
}
// --------


inline std::string licenseID(License license) {
    switch (license) {
        case License::AGPL: return "agpl"; break;
        case License::Apache: return "apache"; break;
        case License::Boost: return "boost"; break;
        case License::BSD2: return "bsd2"; break;
        case License::BSD3: return "bsd3"; break;
        case License::CC0: return "cc0"; break;
        case License::Eclipse: return "eclipse"; break;
        case License::GPL2: return "gpl2"; break;
        case License::GPL3: return "gpl3"; break;
        case License::LGPL: return "lgpl"; break;
        case License::MIT: return "mit"; break;
        case License::Mozilla: return "mozilla"; break;
        case License::Unlicense: return "unlicense"; break;
        default: return "custom"; break;
    }
}

inline std::string outputID(PTOutputType type) {
    switch (type) {
        case PTOutputType::Executable: return "exe"; break;
        case PTOutputType::IntermediateRepresentation: return "ir"; break;
        case PTOutputType::Object: return "obj"; break;
        case PTOutputType::SharedLibrary: return "sharedlib"; break;
        case PTOutputType::StaticLibrary: return "staticlib"; break;
        default: return ""; break;
    }
}

inline License IDtoLicense(std::string license) {
    if (license == "mit") return License::MIT;
    else if (license == "apache") return License::Apache;
    else if (license == "gpl3") return License::GPL3;
    else if (license == "bsd2") return License::BSD2;
    else if (license == "bsd3") return License::BSD3;
    else if (license == "boost") return License::Boost;
    else if (license == "cc0") return License::CC0;
    else if (license == "eclipse") return License::Eclipse;
    else if (license == "agpl") return License::AGPL;
    else if (license == "gpl2") return License::GPL2;
    else if (license == "lgpl") return License::LGPL;
    else if (license == "mozilla") return License::Mozilla;
    else if (license == "unlicense") return License::Unlicense;
    else return License::Custom;
}

inline PTOutputType IDtoOutput(std::string outputType) {
    if (outputType == "exe") return PTOutputType::Executable;
    else if (outputType == "ir") return PTOutputType::IntermediateRepresentation;
    else if (outputType == "obj") return PTOutputType::Object;
    else if (outputType == "sharedlib") return PTOutputType::SharedLibrary;
    else if (outputType == "staticlib") return PTOutputType::StaticLibrary;
    else std::println(std::cerr, "{}[NeolumaCLI/IDtoOutput] The format of PTOutputType is incorrect. Available ones are: exe, ir, obj, sharedlib, staticlib ", Color::TextHex("#ff5050"));
    return PTOutputType::None;
}