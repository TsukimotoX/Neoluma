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
#include "../Libraries/localization/localization.hpp"
#include "../Core/Extras/ProjectManager/projectmanager.hpp"
#include "../HelperFunctions.hpp"
#include "Core/compiler.hpp"

// ==== Helping functions

// Lists authors by comma. If author is only mentioned once, just author name is inputted
std::string listAuthors(const std::vector<std::string>& authors) {
    std::ostringstream oss;
    bool first = true;

    for (const auto& raw : authors) {
        std::string name = trim(raw);
        if (name.empty()) continue;

        if (!first) oss << ", ";
        oss << name;
        first = false;
    }

    return oss.str();
}

// Renames the project folder to letters and underscores. Unnecessary, but for clean experience.
std::string formatProjectFolderName(const std::string& input) {
    std::string result = input;

    for (char& c : result) {
        if (c == ' ') {
            c = '_';
        } else {
            c = std::tolower(static_cast<unsigned char>(c));
        }
    }
    
    return result;
}

// Returns progress bar for CLI
void showProgressBar(const std::string& stepName, int step, int total) {
    int percentage = (step * 100) / total;
    int hashes = percentage / 5;
    if (percentage != 100) std::println("{} [ {} ({}{}) {}% ] {}", Color::TextHex("#f6ff75"), stepName, std::string(hashes, '#'), std::string(20 - hashes, '_'), percentage, Color::TextHex("#01e0d4"));
    else std::println("{} [ {} ({}{}) {}%! ] {}", Color::TextHex("#75ff87"), stepName, std::string(hashes, '#'), std::string(20 - hashes, '_'), percentage, Color::TextHex("#01e0d4"));
}

// Clears terminal screen
void clearScreen() {
    std::cout << "\033[0m\033[2J\033[H";
}

// Asks a question
std::string askQuestion(const std::string& question) {
    return asker::input(question, true);
}

// -------- stuff required for parseProjectFile
std::string getString(const Toml::TomlTable& table, const std::string& key, const std::string& def) {
    for (const auto& [k, v] : table) {
        if (k == key && v.type == Toml::TomlType::String)
            return std::get<std::string>(v.value);
    }
    return def;
}

std::vector<std::string> getStringArray(const Toml::TomlTable& table, const std::string& key) {
    for (const auto& [k, v] : table) {
        if (k == key && v.type == Toml::TomlType::Array) {
            std::vector<std::string> result;
            const auto& arr = std::get<Toml::TomlArray>(v.value);
            for (const auto& item : arr) {
                if (item.type == Toml::TomlType::String)
                    result.push_back(std::get<std::string>(item.value));
            }
            return result;
        }
    }
    return {};
}

std::map<std::string, std::string> extractMap(const Toml::TomlTable& root, const std::string& key) {
    std::map<std::string, std::string> result;

    for (const auto& [k, v] : root) {
        if (k == key && v.type == Toml::TomlType::Table) {
            const auto& tbl = std::get<Toml::TomlTable>(v.value);
            for (const auto& [tk, tv] : tbl) {
                if (tv.type == Toml::TomlType::String)
                    result[tk] = std::get<std::string>(tv.value);
            }
        }
    }

    return result;
}
// --------

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
        config.license = parseLicense(getString(project, "license", ""));
        config.sourceFolder = getString(project, "sourceFolder", config.sourceFolder);
        config.output = parseOutput(getString(project, "output", ""));
        config.buildFolder = getString(project, "buildFolder", config.buildFolder);
    }

    // get configured tasks, dependencies, tests and languagePacks
    config.tasks = extractMap(root, "tasks");
    config.dependencies = extractMap(root, "dependencies");
    config.tests = extractMap(root, "tests");
    config.languagePacks = extractMap(root, "languagePacks");

    // configure source path
    config.sourcePath = file;
    
    return config;
}

std::string licenseID(License license) {
    switch (license) {
        case License::AGPL: return "agpl";
        case License::Apache: return "apache";
        case License::Boost: return "boost";
        case License::BSD2: return "bsd2";
        case License::BSD3: return "bsd3";
        case License::CC0: return "cc0";
        case License::Eclipse: return "eclipse";
        case License::GPL2: return "gpl2";
        case License::GPL3: return "gpl3";
        case License::LGPL: return "lgpl";
        case License::MIT: return "mit";
        case License::Mozilla: return "mozilla";
        case License::Unlicense: return "unlicense";
        default: return "custom";
    }
}

std::string outputID(PTOutputType type) {
    switch (type) {
        case PTOutputType::Executable: return "exe";
        case PTOutputType::IntermediateRepresentation: return "ir";
        case PTOutputType::Object: return "obj";
        case PTOutputType::SharedLibrary: return "sharedlib";
        case PTOutputType::StaticLibrary: return "staticlib";
        default: return "";
    }
}

License parseLicense(std::string license) {
    if (license == "mit") return License::MIT;
    else if (license == "apache") return License::Apache;
    else if (license == "gpl3") return License::GPL3;
    else if (license == "bsd2") return License::BSD2;
    else if (license == "bsd3") return License::BSD3;
    else if (license == "boost") return License::Boost;
    else if (license == "cc0") return License::CC0;
    else if (license == "eclipse") return License::Eclipse;
    else if (license == "agpl") return License::AGPL;
    else if (license == "gpl2") return License::GPL2;
    else if (license == "lgpl") return License::LGPL;
    else if (license == "mozilla") return License::Mozilla;
    else if (license == "unlicense") return License::Unlicense;
    else return License::Custom;
}

PTOutputType parseOutput(std::string outputType) {
    if (outputType == "exe") return PTOutputType::Executable;
    else if (outputType == "ir") return PTOutputType::IntermediateRepresentation;
    else if (outputType == "obj") return PTOutputType::Object;
    else if (outputType == "sharedlib") return PTOutputType::SharedLibrary;
    else if (outputType == "staticlib") return PTOutputType::StaticLibrary;
    else std::println(std::cerr, "{}[NeolumaCLI/IDtoOutput] {}", Color::TextHex("#ff5050"), Localization::translate("Compiler.CLI.parseProjectFile.parseOutputError"));
    return PTOutputType::None;
}

// ==== Main functions ====

void build(const std::string& nlpFile) {
    ProjectConfig config = parseProjectFile(nlpFile);
    std::println("{} {}", Localization::translate("Compiler.CLI.build.initialization"), config.name);
    // todo: вызов компилятора, генерация .exe
}

void run(const std::string& nlpFile) {
    build(nlpFile);
    std::println("{}\n", Localization::translate("Compiler.CLI.run.initialization"));
    // todo: запускаем с помощью std::system или CreateProcess
}

void check(const std::string& nlpFile) {
    ProjectConfig config = parseProjectFile(nlpFile);
    Compiler compiler = Compiler(config);
    std::println("{} {}",  Localization::translate("Compiler.CLI.check.initialization"), config.name);
    compiler.check();
    // todo: only lexer, parser and semantic analysis
}

void createProject() {
    ProjectConfig config;
    int steps = 5; int step = 0;
    std::string title = std::format("{} ", Localization::translate("Compiler.CLI.createProject.initialization"));

    clearScreen();
    showProgressBar(title, step++, steps);

    config.name = askQuestion(Localization::translate("Compiler.CLI.createProject.projectName"));
    clearScreen();
    showProgressBar(title, step++, steps);
    config.version = askQuestion(Localization::translate("Compiler.CLI.createProject.projectVersion"));
    clearScreen();
    showProgressBar(title, step++, steps);
    std::vector<std::string> authors = split(askQuestion(Localization::translate("Compiler.CLI.createProject.projectAuthors")), ',');
    std::string authorList = listAuthors(authors);
    config.author = authors;
    clearScreen();
    showProgressBar(title, step++, steps);
    std::string licenses[14] = { "MIT", "Apache 2.0", "GNU GPL v3", "BSD 2-Clause \"Simplified\"", "BSD 3-Clause \"New\" or \"Revised\"", "Boost Software 1.0", "CC0 v1 Universal", "Eclipse", "GNU AGPL v3", "GNU GPL v2", "GNU LGPL v2.1", "Mozilla 2.0", "The Unlicense", "Custom"};
    std::string license = asker::selectList(Localization::translate("Compiler.CLI.createProject.projectLicense"), licenses);

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
    showProgressBar(title, step++, steps);
    bool confirmation = asker::confirm(formatStr(Localization::translate("Compiler.CLI.createProject.confirmation"),
        Color::TextHex("#FF8C75"), config.name, config.version, authorList, license, Color::TextHex("#96fcbd"), Color::Reset));
    if (confirmation) {
        createProject(config);
        clearScreen();
        std::println(std::cout, "{}", formatStr(Localization::translate("Compiler.CLI.createProject.confirmation.yes"), Color::TextHex("#75ff87"), Color::Reset));
    } else {
        std::println(std::cout, "{}", formatStr(Localization::translate("Compiler.CLI.createProject.confirmation.no"), Color::TextHex("#ff5050"), Color::Reset));
    }
    std::println(Color::Reset);
}

void createProject(ProjectConfig config) {
    std::filesystem::path projectPath = std::filesystem::current_path() / config.name;
    std::filesystem::create_directory(projectPath);
    std::filesystem::create_directory(projectPath / "src");

    std::ofstream mainFile(projectPath / "src/main.nm");
    mainFile << std::format("// {} \n@entry\nfn main() {{\n    print(\"{}\");\n}}", Localization::translate("Compiler.CLI.createProject.template.main.comment"), Localization::translate("Compiler.CLI.createProject.template.main.printmsg"));
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
    std::println(std::cout, "{}", Localization::translate("Compiler.CLI.helpMessage"));
}

