#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "nodes.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Module is a file. Each file is a module for the program inside Neoluma
class Modules {
    std::string name;
    ASTNode* ast;
    std::vector<std::string> imports;
    bool isEntry = false;
};

class Program {
public:
    Program(const std::string& path, bool isProject);
    void build();
    ASTNode* getGlobalAST();
    ASTNode* getEntryPoint();
private:
    std::unordered_map<std::string, ASTNode*> modules;
    std::unordered_set<std::string> loadedPaths;
    std::string entryPath;
    ASTNode* globalTree = nullptr;
    std::vector<std::string> modulePaths;
    bool isProject; // Language can be both interpreted or compiled
    void loadProjectFile(const std::string& path);
    void loadModule(const std::string& path);
    ASTNode linkModules();
};

#endif