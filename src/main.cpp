#include "lexer.h"
#include "parser.h"
#include <iostream>

int main() {
    std::string code = R"(
        if (a > b) {
            x = 10;
        } else {
            x = 20;
        }
    )";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    for (auto& token : tokens) {
        std::cout << token.toString() << " ";
    }
    std::cout << std::endl;
    Program expr = parseProgram(tokens);
    printProgram(expr);
    std::cout << std::endl;
    return 0;
}
