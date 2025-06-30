#include "cli.hpp"
#include "../HelperFunctions.hpp"

ProjectConfig parseProjectFile(const std::string& file) {
    return ProjectConfig {};
}

void build(const std::string& nlpFile) {
    ProjectConfig config = parseProjectFile(nlpFile);
    print("🔨 Building project: {}", config.name);
    // todo: вызов компилятора, генерация .exe
}

void run(const std::string& nlpFile) {
    build(nlpFile);
    std::cout << "🚀 Running executable...\n";
    // todo: запускаем с помощью std::system или CreateProcess
}

void check(const std::string& nlpFile) {
    ProjectConfig config = parseProjectFile(nlpFile);
    std::cout << "✅ Syntax check for: " << config.name << std::endl;
    // todo: вызываем только лексер+парсер
}

void createProject() {
    // todo: генерируем папку, .nlp и src/main.nm
    print("📃 Creating a new Neoluma project...");
    string name = input<string>("❓ What's the name of your project? ");
    print("Your project name is: {}", name);
}

void printHelp() {
    std::cout << R"(Neoluma is a high-level, all-purpose programming language designed to be a language for everything.
Whether you're writing a small script or building an entire operating system, Neoluma is made to scale with you. With a Python-like syntax and C#/C++-inspired architecture, 
it's both expressive and powerful.

Usage:
  neoluma build <project.nlp>  - Compile project to executable
  neoluma run <project.nlp>    - Compile and immediately run
  neoluma check <project.nlp>  - Syntax-check without building
  neoluma new <name>           - Create new project
  neoluma version              - Print compiler version)" << std::endl;
}