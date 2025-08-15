#include "cli.hpp"
#include "../HelperFunctions.hpp"
#include <iostream>
#include <vector>
#include <print>
#include "../Libraries/toml/toml.hpp"
#include "../Libraries/Asker/asker.hpp"

// ==== Helping functions

// Argument parsing
CLIArgs parseArgs(int argc, char** argv) {
    CLIArgs args;
    if (argc > 1) args.command = argv[1];

    for (int i = 2; i < argc; i++) {
        std::string token = argv[i];
        if (token.rfind("--", 0) == 0) {
            std::string key = token.substr(2);
            std::string value;
            if (i+1 < argc && argv[i+1][0] != '-') {
                value = argv[i++];
            }
            args.options[key] = value;
        } else {
            args.positional.push_back(token);
        }
    }
    return args;
}

// Returns progress bar for CLI
void showProgressBar(const std::string& stepName, int step, int total) {
    int percentage = (step * 100) / total;
    int hashes = percentage / 5;
    if (percentage != 100) {
        std::println("{} [ {} ({}{}) {}% ] {}", Color::Text::BrightYellow, stepName, std::string(hashes, '#'), std::string(20 - hashes, '_'), percentage, Color::Text::BrightCyan);
    } else {
        std::println("{} [ {} ({}{}) {}%! ] {}", Color::Text::BrightGreen, stepName, std::string(hashes, '#'), std::string(20 - hashes, '_'), percentage, Color::Text::BrightCyan);
    }
}

// Clears terminal screen
void clearScreen() {
    std::cout << "\033[0m\033[2J\033[H";
}

// Asks a question
std::string askQuestion(const std::string& question) {
    return asker::input(std::format("{}{} {}", Color::Text::BrightBlue, question, Color::Text::Magenta), true);
}

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
    ProjectConfig config;
    int steps = 5; int step = 0;

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
    std::string license = asker::selectList(std::format("{}📃 What license does your project have? {}", Color::Text::BrightBlue, Color::Text::BrightMagenta), licenses);
    
    if (license == "MIT") config.license = "mit";
    else if (license == "Apache 2.0") config.license = "apache";
    else if (license == "GNU GPL v3") config.license = "gpl3";
    else if (license == "BSD 2-Clause \"Simplified\"") config.license = "bsd2";
    else if (license == "BSD 3-Clause \"New\" or \"Revised\"") config.license = "bsd3";
    else if (license == "Boost Software 1.0") config.license = "boost";
    else if (license == "CC0 v1 Universal") config.license = "cc0";
    else if (license == "Eclipse") config.license = "eclipse";
    else if (license == "GNU AGPL v3") config.license = "agpl";
    else if (license == "GNU GPL v2") config.license = "gpl2";
    else if (license == "GNU LGPL v2.1") config.license = "lgpl";
    else if (license == "Mozilla 2.0") config.license = "mozilla";
    else if (license == "The Unlicense") config.license = "unlicense";
    else config.license = "custom";
    
    clearScreen();
    showProgressBar("📃 Creating a new Neoluma project ", step++, steps);
    bool confirmation = asker::confirm(std::format("{}😎 Let's sum up your project!\n   Project Name: {};\n   Version: {};\n   Authors: {};\n   License: {};\n\nIs that correct?{}", Color::Text::BrightYellow, config.name, config.version, authorList, license, Color::Reset));
    if (confirmation) {
        createProject(config);
        clearScreen();
        std::println("{}✅ Project created! Have fun building in Neoluma!{}", Color::Text::BrightGreen, Color::Reset);
    } else {
        std::println("❌ Project cancelled. You didn't write 'Yes'. If it's a mistake, try create a project again! (and don't type anything beyond 'Yes' on comfirmation. No space, not anything.)");
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

    auto table = Toml::Table::make("");
    auto project = Toml::Table::make("project");
    project["name"] = config.name;
    project["version"] = config.version;

    Toml::TomlArray authors_array;
    for (const auto& author : config.author) authors_array.push_back(Toml::TomlValue(author));
    project["authors"] = Toml::TomlValue(authors_array);
    project["license"] = config.license;
    project["output"] = config.output;
    project["sourceFolder"] = config.sourceFolder;
    project["buildFolder"] = config.buildFolder;
    table["project"] = Toml::TomlValue(project.get());

    auto tasks = Toml::Table::make("tasks");
    tasks["dev"] = "neoluma run --debug";
    table["tasks"] = Toml::TomlValue(tasks.get());

    std::ofstream cfg(projectPath / std::format("{}.nlp", formatProjectFolderName(config.name)));
    if (cfg.is_open()){
        Toml::serializeTable(cfg, table);
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
    std::println("Neoluma is a high-level, all-purpose programming language designed to be a language for everything.\nWhether you're writing a small script or building an entire operating system, Neoluma is made to scale with you. With a Python-like syntax and C#/C++-inspired architecture, \nit's both expressive and powerful.\n\nUsage:\n  neoluma build <project.nlp>  - Compile project to executable\n  neoluma run <project.nlp>    - Compile and immediately run\n  neoluma check <project.nlp>  - Syntax-check without building\n  neoluma new <name>           - Create new project\n  neoluma version              - Print compiler version");
}

