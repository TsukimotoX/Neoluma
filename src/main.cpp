#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <Core/Lexer/lexer.hpp>

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

int main() {
    return 0;
}

