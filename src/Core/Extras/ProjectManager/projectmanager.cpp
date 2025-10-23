#include "projectmanager.hpp"
#include "../../Frontend/Lexer/lexer.hpp"
#include "../../Frontend/Parser/parser.hpp"
#include "../../../HelperFunctions.hpp"
#include <fstream>
#include <sstream>

ProjectManager::ProjectManager(const ProjectConfig& config) : config(config) {}

void ProjectManager::build() {}

void ProjectManager::clean() {}

void ProjectManager::run() {}

void ProjectManager::check() {
    // temporarily check things here. after that you move it into compiler
    for (const auto& file : listFiles()) {
        std::string source = readFile(file);
        Lexer lexer;
        lexer.tokenize(file, source);
        lexer.printTokens(file);
        
    }
}