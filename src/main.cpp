#include <string>
#include <iostream>
#include <print>
#include <filesystem>

#include "CLI/cli.hpp"
#include "Libraries/color/color.hpp"
#include "Core/Frontend/Lexer/lexer.hpp"
#include "HelperFunctions.hpp"
#include "Libraries/localization/localization.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

std::string findProjectFile(const std::string& folder) {
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".nlp") {
            return entry.path().string();
        }
    }
    return "";
}

int main(int argc, char** argv) {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    Localization::init();
    CLIArgs args = parseArgs(argc, argv);

    if (args.command == "new") {
        if (!args.options.empty()) {
            ProjectConfig config;
            config.name = args.options.count("name") ? args.options.at("name") : config.name;
            if (args.options.count("author")) {
                config.author = split(args.options.at("author"), ',');
            }
            config.version = args.options.count("version") ? args.options.at("version") : config.version;
            if (args.options.count("license")) {
                config.license = parseLicense(args.options.at("license"));
            }
            
            createProject(config);
        } else { // If no arguments provided, aka "neoluma new", it will run an integrated assistant to set up a project.
            createProject();
        }
    } else if (args.command == "build") {
        // build
    } else if (args.command == "run") {
        // run
    } else if (args.command == "check") {
        std::string projectFilePath;
        if (args.options.count("project")) projectFilePath = args.options.at("project");
        else {
            projectFilePath = findProjectFile(std::filesystem::current_path().string());
        }

        if (projectFilePath.empty()) {
            // translate later
            std::println("{}[Neoluma/Check] Project file was not found!{}", Color::TextHex("#ff5050"), Color::Reset);
            return 2;
        }
        
        check(projectFilePath);
    } else if (args.command == "version") {
        std::println("{}Neoluma v1.0{}", Color::TextHex("#ff28e6"), Color::Reset);
    } else {
        printHelp();
    }

    return 0;
}