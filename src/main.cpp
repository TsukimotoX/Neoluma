#include "lexer.h"
#include "parser.h"
#include <iostream>

int main() {
    std::string code = R"(
        2+3
    )";

    Lexer lexer(code);
    auto lexer_res = lexer.tokenize();
    Parser parser(lexer_res);
    auto parser_res = parser.parse();
    std::cout << parser_res.get() << std::endl;
    return 0;
}
