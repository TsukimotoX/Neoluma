#include <string>
#include <iostream>
#include "CLI/cli.hpp"
#include <windows.h>

/*
void handler(int signal) {
    std::cerr << "Caught signal " << signal << std::endl;
    std::exit(1);
}

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath); // open the file

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // read whole file into buffer

    return buffer.str(); // return string
}

int main() {
    std::signal(SIGABRT, handler);
    std::string rawCode;
    rawCode = readFile("./program.nm");
    Lexer lexer = Lexer(rawCode);
    lexer.tokenize();
    lexer.printTokens();
    return 0;
}
*/

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    CLIArgs args = parseArgs(argc, argv);

    if (args.command == "new") {
        if (!args.options.empty()) {
            ProjectConfig config;
            config.name = args.options.count("name") ? args.options.at("name") : config.name;
            if (args.options.count("author")) {
                config.author = split(args.options.at("author"), ',');
            }
            config.version = args.options.count("version") ? args.options.at("version") : config.version;
            config.license = args.options.count("license") ? args.options.at("license") : config.license;
            
            createProject(config);
        } else { // If no arguments provided, aka "neoluma new", it will run an integrated assistant to set up a project.
            createProject();
        }
    } else if (args.command == "build") {
        // build
    } else if (args.command == "run") {
        // run
    } else if (args.command == "check") {
        // check
    } else {
        printHelp();
    }

    return 0;
}