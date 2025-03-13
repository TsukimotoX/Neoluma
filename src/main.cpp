#include "lexer.h"
#include <iostream>

int main() {
    std::string code = R"(
        function add(a, b) {
            return a + b;
        }
        print(add(10, 20));
    )";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token: " << token.value << " (Type: " << token.type << ")\n";
    }

    return 0;
}
