#include <string>
#include <iostream>

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

void printHelp() {
    std::cout << "Neoluma is a high-level, all-purpose programming language designed to be a language for everything.\nWhether you're writing a small script or building an entire operating system, Neoluma is made to scale with you. With a Python-like syntax and C#/C++-inspired architecture, it's both expressive and powerful. \n\nUsage:\n  neoluma build <project.nlp>  - Compile project to executable\n  neoluma run <project.nlp>    - Compile and immediately run\n  neoluma check <project.nlp>  - Syntax-check without building\n  neoluma new <name>           - Create new project\n  neoluma version              - Print compiler version" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printHelp(); return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "build") {
        // check file exists → parse .nlp → buildExecutable(config)
    }
    else if (cmd == "run") {
        // build + runExecutable(path)
    }
    else if (cmd == "check") {
        // parse project + validate syntax only
    }
    else if (cmd == "new") {
        // createProjectScaffold(argv[2])
    }
    else if (cmd == "version") {
        std::cout << "Neoluma v0.1.0" << std::endl;
    }
    else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        printHelp();
        return 1;
    }

    return 0;
}