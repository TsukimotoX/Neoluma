#include "cli.hpp"
#include "../HelperFunctions.hpp"
#include <iostream>
#include <vector>
#include <print>
#include "../Libraries/toml/toml.hpp"

// other

ProjectConfig parseProjectFile(const std::string& file) {
    return ProjectConfig {};
}

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
    std::println("✅ Syntax check for: {}", config.name);
    // todo: вызываем только лексер+парсер
}

void createProject() {
    // IS GOING TO BE REMADE WITH INTERACTIVE INQUIRER ASSISTANT!!! 

    // todo: generate folder, .nlp and src/main.nm
    std::println("📃 Creating a new Neoluma project...");
    std::string name = input<std::string>("❓ What's the name of your project? ");
    std::string version = input<std::string>("🆚 What's your project first version (can be anything)? ");
    std::vector<std::string> authors = split(input<std::string>("🤓 Who is/are the author(s) of the project? "), ',');
    std::string authorList = listAuthors(authors);
    std::string license = input<std::string>("📃 What license does your project have (MIT/Apache/GNU/OSL/others you choose)? ");
    std::println("😎 Let's sum up your project!\n   Project Name: {};\n   Version: {};\n   Authors: {};\n   License: {};\n\nIs that correct? Please type in 'Yes' here for approval.", name, version, authorList, license);
    std::string confirmation = input<std::string>("Your project creation sign: ");

    // bad practice but whatever
    if (confirmation == "Yes" || confirmation == "yes" || confirmation == "y" || confirmation == "Y") {
        std::filesystem::path projectPath = std::filesystem::current_path() / name;
        std::filesystem::create_directory(projectPath);
        std::filesystem::create_directory(projectPath / "src");

        std::ofstream mainFile(projectPath / "src/main.nm");
        mainFile << "// Welcome to Neoluma! Have fun building!\n";
        mainFile << "@entry\n";
        mainFile << "fn main() {\n    print(\"Hello from Neoluma!\");\n}\n";
        mainFile.close();

        auto table = Toml::Table::make("");
        auto project = Toml::Table::make("project");
        project["name"] = name;
        project["version"] = version;

        Toml::TomlArray authors_array;
        for (const auto& author : authors) authors_array.push_back(Toml::TomlValue(author));
        project["authors"] = Toml::TomlValue(authors_array);
        project["license"] = license;
        project["output"] = "exe";
        project["sourceFolder"] = "src";
        project["buildFolder"] = "build";
        table["project"] = Toml::TomlValue(project.get());

        auto tasks = Toml::Table::make("tasks");
        tasks["dev"] = "neoluma run --debug";
        table["tasks"] = Toml::TomlValue(tasks.get());

        std::ofstream config(projectPath / std::format("{}.nlp", formatProjectFolderName(name)));
        if (config.is_open()){
            Toml::serializeTable(config, table);
        }
        config.close();

        std::println("✅ Project created! Have fun building in Neoluma!");
    } else {
        std::println("❌ Project cancelled. You didn't write 'Yes'. If it's a mistake, try create a project again! (and don't type anything beyond 'Yes' on comfirmation. No space, not anything.)");
    }
}

void printHelp() {
    std::println("Neoluma is a high-level, all-purpose programming language designed to be a language for everything.\nWhether you're writing a small script or building an entire operating system, Neoluma is made to scale with you. With a Python-like syntax and C#/C++-inspired architecture, \nit's both expressive and powerful.\n\nUsage:\n  neoluma build <project.nlp>  - Compile project to executable\n  neoluma run <project.nlp>    - Compile and immediately run\n  neoluma check <project.nlp>  - Syntax-check without building\n  neoluma new <name>           - Create new project\n  neoluma version              - Print compiler version");
}

