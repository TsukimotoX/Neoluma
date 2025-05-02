#include "lexer.h"
#include "parser.h"
#include <iostream>

int main() {
    std::string code = R"(
        1+2*3
    )";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    for (auto& token : tokens) {
        std::cout << token.toString() << " ";
    }
    std::cout << std::endl;
    Parser parser(tokens);
    auto program = parser.parseProgram();
    parser.printAST(program, "", true);
    std::cout << std::endl;
    return 0;
}
