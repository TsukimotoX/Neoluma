#include "compiler.hpp"

Compiler::Compiler(ProjectConfig& config) : projectManager(config) {
    for (const auto& file : std:: filesystem::recursive_directory_iterator(std::filesystem::path(config.sourcePath) / config.sourceFolder, std::filesystem::directory_options::skip_permission_denied)) {
        if (file.is_regular_file()) projectManager.addFile(file.path().string());
    }
}

void Compiler::check() {
    for (const auto& file : projectManager.listFiles()){
        std::string source = readFile(file);
        std::vector<Token> tokens = lexer.tokenize(file, source);
        lexer.printTokens(getFileName(file));

        parser.parseModule(tokens, getFileName(file));
        parser.printModule();

    }
}