#include "projectmanager.hpp"
#include "../../Frontend/Lexer/lexer.hpp"
#include <fstream>
#include <sstream>

// ==== Helpers ====

// Reads the file (listen im too lazy to clean it up rn do it later)
std::string readFile2(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
} 

// others

ProjectManager::ProjectManager(const ProjectConfig& config) : config(config) {}

void ProjectManager::build() {}

void ProjectManager::clean() {}

void ProjectManager::run() {}

void ProjectManager::check() {
    Lexer lexer;

    for (const auto& file : listFiles()) {
        std::string source = readFile2(file);
        lexer.tokenize(source);
        lexer.printTokens();
    }
}