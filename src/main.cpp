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
    if (argc < 2) {
        printHelp(); return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "build" && argc >= 3) build(argv[2]);
    else if (cmd == "run" && argc >= 3) run(argv[2]);
    else if (cmd == "check" && argc >= 3) check(argv[2]);
    else if (cmd == "new") createProject();
    else if (cmd == "version") std::cout << "Neoluma v0.1.0" << std::endl;
    else printHelp();

    return 0;
}