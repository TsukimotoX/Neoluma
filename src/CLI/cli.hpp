#pragma once
#include "../HelperFunctions.hpp"
#include <iostream>
#include <vector>
#include <map>

// ProjectConfig is a struct that allows me to determine project structure.
struct ProjectConfig {
    std::string name = "Untitled Project";
    std::string version = "1.0.0";
    std::vector<std::string> author = { "Untitled Author" };
    std::string license = "MIT";
    std::string sourceFolder = "src/";
    std::string output = "exe";
    std::string buildFolder = ".build/";
    std::map<std::string, std::string> dependencies;
    std::map<std::string, std::string> tasks;
    std::map<std::string, std::string> tests;
    std::map<std::string, std::string> languagePacks;
};

// ==== Main functions ====


void build(const std::string& nlpFile);
void run(const std::string& nlpFile);
void check(const std::string& nlpFile);

void createProject();


// Help function that just tells details
void printHelp();

// ==== Helper functions ====

// Lists authors by comma. If author is only mentioned once, just author name is inputted
inline std::string listAuthors(const std::vector<std::string>& authors) {
    std::ostringstream oss;
    bool first = true;

    for (const auto& raw : authors) {
        std::string name = trim(raw);
        if (name.empty()) continue;

        if (!first) oss << ", ";
        oss << name;
        first = false;
    }

    return oss.str();
}

// Renames the project folder to letters and underscores. Unnecessary, but for clean experience.
inline std::string formatProjectFolderName(const std::string& input) {
    std::string result = input;

    for (char& c : result) {
        if (c == ' ') {
            c = '_';
        } else {
            c = std::tolower(static_cast<unsigned char>(c));
        }
    }
    
    return result;
}