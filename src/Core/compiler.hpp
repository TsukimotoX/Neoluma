#pragma once
#include "Frontend/Lexer/lexer.hpp"
#include "Frontend/Parser/parser.hpp"
#include "Extras/ProjectManager/projectmanager.hpp"
#include "Frontend/ErrorManager/errormanager.hpp"
#include "Frontend/SemanticAnalysis/SemanticAnalysis.hpp"

struct Compiler {
    // All parts of compiler
    Lexer lexer;
    Parser parser;
    //SemanticAnalysis analysis;

    ProjectManager projectManager;
    ErrorManager errorManager;

    // Constructor
    Compiler(ProjectConfig& config);

    // Functions
    void compile(); // compiled way
    void check();
    void run(); // interpreted way

    // Data
    std::vector<MemoryPtr<ModuleNode>> modules = {}; // all files of the project
};
