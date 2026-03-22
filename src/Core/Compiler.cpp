#include "Compiler.hpp"
#include "Libraries/Color/Color.hpp"

Compiler::Compiler(ProjectConfig& config) : projectManager(config) {
    for (const auto& file : std:: filesystem::recursive_directory_iterator(std::filesystem::path(config.sourcePath) / config.sourceFolder, std::filesystem::directory_options::skip_permission_denied)) {
        if (file.is_regular_file() && file.path().extension() == ".nm") projectManager.addFile(file.path().string());
    }

    lexer.setCompiler(this);
    parser.setCompiler(this);
    orchestrator.setCompiler(this);
    semanticAnalysis.setCompiler(this);
}

void Compiler::check() {
    for (const auto& file : projectManager.listFiles()){
        // Lexer: breaks code down into tokens.
        std::string source = readFile(file);
        std::vector<Token> tokens = lexer.tokenize(file, source);
        //lexer.printTokens(getFileName(file));

        // Parser: builds a module tree out of tokens
        parser.parseModule(tokens, getFileName(file));
        //parser.printModule();
        MemoryPtr<ModuleNode> tree = std::move(parser.moduleSource);

        // Adding modules to program's tree
        modules.push_back(std::move(tree));
    }

    // Orchestrator: stitches files together into a full program, used for Semantic Analysis and more.
    auto entry = orchestrator.findEntryPoint(modules);
    auto infos = orchestrator.resolveImports(modules);
    /*std::println(std::cout, "=== ModuleId map ===");
    for (const auto& info : infos){
        std::println(std::cout, "[{}] file={}", info.id, info.module ? info.module->filePath : "<null>");
        std::println(std::cout, "     deps={}", info.dependencies.size());
        for (auto d : info.dependencies) std::println(std::cout, "        -> {}", d.moduleId);
    }*/
    auto program = orchestrator.stitchProgram(entry, infos);
    /*std::println(std::cout, "Entry module id: {}", program.entryModule);
    std::println(std::cout, "Order:");
    for (auto id : program.order) {
        std::println(std::cout, "    {}", id);
    }*/

    // Semantic Analysis: Make sure the program runs logically correct, before turned into a machine code
    semanticAnalysis.analyzeProgram(program, infos);

    if (errorManager.hasErrors()) {
        errorManager.printErrors();
        std::println(std::cout, "{}{}{}", Color::TextHex("#ff5050"), Localization::translate("CLI.check.failed"), Color::Reset);
    } else std::println(std::cout, "{}{}{}", Color::TextHex("#75ff87"), Localization::translate("CLI.check.complete"), Color::Reset);
}