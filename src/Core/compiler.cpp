#include "compiler.hpp"

Compiler::Compiler(ProjectConfig& config) : projectManager(config) {
    // add recursively adding files
    for (const auto& file : std::filesystem::recursive_directory_iterator(
        std::filesystem::path(config.sourcePath) /
        std::filesystem::path(config.sourceFolder))) {
        if (file.is_regular_file()) {
            projectManager.addFile(file.path().string());
        }
    }
    for (const auto& i  : projectManager.listFiles())
    {
        std::println(std::cout, "{}", i);
    }
}

void Compiler::check(){}