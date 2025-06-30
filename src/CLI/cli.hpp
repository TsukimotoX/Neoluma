#pragma once
#include <iostream>
#include "../HelperFunctions.hpp"
#include <map>

struct ProjectConfig {
    string name;
    string version = "1.0.0";
    array<string> author;
    string license;
    string sourceFolder;
    string output = "exe";
    string buildFolder = ".build/";
    std::map<string, string> dependencies;
    std::map<string, string> tasks;
    std::map<string, string> tests;
    std::map<string, string> languagePacks;
};

void build(const std::string& nlpFile);
void run(const std::string& nlpFile);
void check(const std::string& nlpFile);
void createProject();
void printHelp();