#include "cli.hpp"
#include <iostream>
#include <vector>
#include <print>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "../Libraries/toml/toml.hpp"
#include "../Libraries/asker/asker.hpp"
#include "../Libraries/color/color.hpp"
#include "../Core/Frontend/Lexer/lexer.hpp"

// ==== Helping functions

std::string licenseID(License license) {
    switch (license) {
        case License::AGPL: return "agpl"; break;
        case License::Apache: return "apache"; break;
        case License::Boost: return "boost"; break;
        case License::BSD2: return "bsd2"; break;
        case License::BSD3: return "bsd3"; break;
        case License::CC0: return "cc0"; break;
        case License::Eclipse: return "eclipse"; break;
        case License::GPL2: return "gpl2"; break;
        case License::GPL3: return "gpl3"; break;
        case License::LGPL: return "lgpl"; break;
        case License::MIT: return "mit"; break;
        case License::Mozilla: return "mozilla"; break;
        case License::Unlicense: return "unlicense"; break;
        default: return "custom"; break;
    }
}

std::string outputID(PTOutputType type) {
    switch (type) {
        case PTOutputType::Executable: return "exe"; break;
        case PTOutputType::IntermediateRepresentation: return "ir"; break;
        case PTOutputType::Object: return "obj"; break;
        case PTOutputType::SharedLibrary: return "sharedlib"; break;
        case PTOutputType::StaticLibrary: return "staticlib"; break;
    }
}

// Argument parsing
CLIArgs parseArgs(int argc, char** argv) {
    CLIArgs args;
    if (argc > 1) args.command = argv[1];

    for (int i = 2; i < argc; i++) {
        std::string token = argv[i];
        if (token.rfind("--", 0) == 0) {
            std::string key = token.substr(2);
            std::string value;

            while (i + 1 < argc && argv[i + 1][0] != '-') {
                if (!value.empty()) value += " ";
                value += argv[++i];
            }
            args.options[key] = value;
        } else {
            args.positional.push_back(token);
        }
    }

    return args;
}

ProjectConfig parseProjectFile(const std::string& file) {
    std::string contents = readFile(file);
    std::istringstream ss(contents);
    Toml::TomlTable root = Toml::parseToml(ss);

    ProjectConfig config;

    // project table parsing
    auto it = std::find_if(root.begin(), root.end(), [](const auto& kv){ return kv.first == "project"; });
    if (it != root.end() && it->second.type == Toml::TomlType::Table) {
        const auto& project = std::get<Toml::TomlTable>(it->second.value);
        config.name = getString(project, "name", config.name);
        config.version = getString(project, "version", config.version);
        config.author = getStringArray(project, "authors");
        config.license = getString(project, "license", config.license);
        config.sourceFolder = getString(project, "sourceFolder", config.sourceFolder);
        config.output = getString(project, "output", config.output);
        config.buildFolder = getString(project, "buildFolder", config.buildFolder);
    }

    // get configured tasks, dependencies, tests and languagePacks
    config.tasks = extractMap(root, "tasks");
    config.dependencies = extractMap(root, "dependencies");
    config.tests = extractMap(root, "tests");
    config.languagePacks = extractMap(root, "languagePacks");

    return config;
}

// ==== Main functions ====

void build(const std::string& nlpFile) {
    ProjectConfig config = parseProjectFile(nlpFile);
    std::println("🔨 Building project: {}", config.name);
    // todo: вызов компилятора, генерация .exe
}

void run(const std::string& nlpFile) {
    build(nlpFile);
    std::println("🚀 Running executable...\n");
    // todo: запускаем с помощью std::system или CreateProcess
}

void check(const std::string& nlpFile) {
    ProjectConfig config = parseProjectFile(nlpFile);
    ProjectManager* manager = new ProjectManager(config);
    std::println("✅ Syntax check for: {}", config.name);
    // add recursively adding files
    manager->check();
    // todo: only lexer and parser
}

void createProject() {
    ProjectConfig config;
    int steps = 5; int step = 1;

    clearScreen();
    showProgressBar("📃 Creating a new Neoluma project ", step++, steps);

    config.name = askQuestion("❓ What's the name of your project?");
    clearScreen();
    showProgressBar("📃 Creating a new Neoluma project ", step++, steps);
    config.version = askQuestion("🆚 What's your project first version (can be anything)?");
    clearScreen();
    showProgressBar("📃 Creating a new Neoluma project ", step++, steps);
    std::vector<std::string> authors = split(askQuestion("🤓 Who is/are the author(s) of the project (separate with ,)?"), ',');
    std::string authorList = listAuthors(authors);
    config.author = authors;
    clearScreen();
    showProgressBar("📃 Creating a new Neoluma project ", step++, steps);
    std::string licenses[14] = { "MIT", "Apache 2.0", "GNU GPL v3", "BSD 2-Clause \"Simplified\"", "BSD 3-Clause \"New\" or \"Revised\"", "Boost Software 1.0", "CC0 v1 Universal", "Eclipse", "GNU AGPL v3", "GNU GPL v2", "GNU LGPL v2.1", "Mozilla 2.0", "The Unlicense", "Custom"};
    std::string license = asker::selectList("📃 What license does your project have?", licenses);

    if (license == "MIT") config.license = License::MIT;
    else if (license == "Apache 2.0") config.license = License::Apache;
    else if (license == "GNU GPL v3") config.license = License::GPL3;
    else if (license == "BSD 2-Clause \"Simplified\"") config.license = License::BSD2;
    else if (license == "BSD 3-Clause \"New\" or \"Revised\"") config.license = License::BSD3;
    else if (license == "Boost Software 1.0") config.license = License::Boost;
    else if (license == "CC0 v1 Universal") config.license = License::CC0;
    else if (license == "Eclipse") config.license = License::Eclipse;
    else if (license == "GNU AGPL v3") config.license = License::AGPL;
    else if (license == "GNU GPL v2") config.license = License::GPL2;
    else if (license == "GNU LGPL v2.1") config.license = License::LGPL;
    else if (license == "Mozilla 2.0") config.license = License::Mozilla;
    else if (license == "The Unlicense") config.license = License::Unlicense;
    else config.license = License::Custom;
    
    clearScreen();
    showProgressBar("📃 Creating a new Neoluma project ", step++, steps);
    bool confirmation = asker::confirm(std::format("{}😎 Let's sum up your project!\n   Project Name: {};\n   Version: {};\n   Authors: {};\n   License: {};\n\n{}Is that correct?{}", Color::TextHex("#FF8C75"), config.name, config.version, authorList, license, Color::TextHex("#96fcbd"), Color::Reset));
    if (confirmation) {
        createProject(config);
        clearScreen();
        std::println("{}✅ Project created! Have fun building in Neoluma!{}", Color::TextHex("#75ff87"), Color::Reset);
    } else {
        std::println("{}❌ Project cancelled. If it's a mistake, try create a project again!{}", Color::TextHex("#ff5050"), Color::Reset);
    }
    std::println(Color::Reset);
}

void createProject(ProjectConfig config) {
    std::filesystem::path projectPath = std::filesystem::current_path() / config.name;
    std::filesystem::create_directory(projectPath);
    std::filesystem::create_directory(projectPath / "src");

    std::ofstream mainFile(projectPath / "src/main.nm");
    mainFile << R""""(// Welcome to Neoluma! Have fun building!
@entry
fn main() {
    print("Hello from Neoluma!");
})"""";
    mainFile.close();

    Toml::Table table;
    Toml::Table project;
    project["name"] = config.name;
    project["version"] = config.version;

    Toml::TomlArray authors_array;
    for (const auto& author : config.author) authors_array.push_back(Toml::TomlValue(author));
    project["authors"] = Toml::TomlValue(authors_array);
    project["license"] = licenseID(config.license);
    project["output"] = outputID(config.output);
    project["sourceFolder"] = config.sourceFolder;
    project["buildFolder"] = config.buildFolder;
    table["project"] = project.get();

    Toml::Table tasks;
    tasks["dev"] = "neoluma run --debug";
    table["tasks"] = tasks.get();

    std::ofstream cfg(projectPath / std::format("{}.nlp", formatProjectFolderName(config.name)));
    if (cfg.is_open()){
        Toml::serializeTable(cfg, table.get());
    }
    cfg.close();

    std::ofstream license(projectPath / "LICENSE");
    if (license.is_open()) {
        Licenses licenses;
        license << licenses.checkLicense(config, config.license);
    }
    license.close();
}

void printHelp() {
    std::println(R""""(Neoluma is a high-level, all-purpose programming language designed to be a language for everything.
Whether you're writing a small script or building an entire operating system, Neoluma is made to scale with you. With a Python-like syntax and C#/C++-inspired architecture,
it's both expressive and powerful.

Usage:
  neoluma build <project.nlp>  - Compile project to executable
  neoluma run <project.nlp>    - Compile and immediately run
  neoluma check <project.nlp>  - Syntax-check without building
  neoluma new <name>           - Create new project
  neoluma version              - Print compiler version
)"""");
}

