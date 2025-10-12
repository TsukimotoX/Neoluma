#pragma once
#include "Frontend/Lexer/lexer.hpp"
#include "Frontend/Parser/parser.hpp"
#include "Extras/ProjectManager/projectmanager.hpp"

struct Compiler {
    // Data
    ProjectConfig config;

    // All parts of compiler
    Lexer lexer;
    Parser parser;

    // Constructor
    Compiler(ProjectConfig config);

    // Functions
    void compile(); // compiled way
    void check();
    void run(); // interpreted way
};