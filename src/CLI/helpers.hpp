#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include "../Libraries/toml/toml.hpp"
#include "../Libraries/asker/asker.hpp"

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