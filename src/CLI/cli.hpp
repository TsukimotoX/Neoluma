#pragma once
#include <iostream>
#include <vector>
#include <map>

struct ProjectConfig {
    std::string name = "Untitled Project";
    std::string version = "1.0.0";
    std::vector<std::string> author;
    std::string license = "MIT";
    std::string sourceFolder = "src/";
    std::string output = "exe";
    std::string buildFolder = ".build/";
    std::map<std::string, std::string> dependencies;
    std::map<std::string, std::string> tasks;
    std::map<std::string, std::string> tests;
    std::map<std::string, std::string> languagePacks;
};

//main func

void build(const std::string& nlpFile);
void run(const std::string& nlpFile);
void check(const std::string& nlpFile);
void createProject();
void printHelp();