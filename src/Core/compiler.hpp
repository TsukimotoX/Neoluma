#pragma once
#include "Frontend/Lexer/lexer.hpp"
#include "Frontend/Parser/parser.hpp"
#include "Extras/ProjectManager/projectmanager.hpp"
#include "Frontend/ErrorManager/errormanager.hpp"

struct Compiler {
    // All parts of compiler
    Lexer lexer;
    Parser parser;
    ProjectManager projectManager;
    ErrorManager errorManager;

    // Constructor
    Compiler(ProjectConfig& config);

    // Functions
    void compile(); // compiled way
    void check();
    void run(); // interpreted way
};
