#pragma once
#include "Frontend/Lexer/Lexer.hpp"
#include "Frontend/Parser/Parser.hpp"
#include "Extras/ProjectManager/ProjectManager.hpp"
#include "Extras/ErrorManager/ErrorManager.hpp"
#include "Frontend/SemanticAnalysis/SemanticAnalysis.hpp"

#include "Frontend/Orchestrator/Orchestrator.hpp"
#include "Libraries/Localization/Localization.hpp"

struct Compiler {
    // All parts of compiler
    Lexer lexer;
    Parser parser;
    SemanticAnalysis semanticAnalysis;

    ProjectManager projectManager;
    ErrorManager errorManager;
    Orchestrator orchestrator;

    // Constructor
    Compiler(ProjectConfig& config);

    // Functions
    void compile(); // compiled way
    void check(bool jsonOutput = false);
    void run(); // interpreted way

    // Data
    std::vector<MemoryPtr<ModuleNode>> modules = {}; // all files of the project
};
