#include "Orchestrator.hpp"
#include "Core/Compiler.hpp"

// Helper Functions

bool Orchestrator::hasEntryDecorator(const FunctionNode* function){
    if (!function) return false;
    for (const auto& decorator : function->decorators){
        if (!decorator || !decorator->callee) return false;
        if (decorator->callee->type != ASTNodeType::Variable) return false;
        auto* var = static_cast<VariableNode*>(decorator->callee.get());
        if (var->varName == "entry") return true;
    }
    return false;
}

// https://www.geeksforgeeks.org/dsa/depth-first-search-or-dfs-for-a-graph/
void Orchestrator::dfsVisit( ModuleId id, const std::vector<ModuleInfo>& infos, std::vector<uint8_t>& state,
                            std::vector<ModuleId>& order, const ErrorSpan* fromSpan)
{
    if (state[id] == 2) return; //done
    if (state[id] == 1) {
        if (fromSpan) {
            compiler->errorManager.addError(
                ErrorType::Preprocessor,
                PreprocessorErrors::CircularImport,
                *fromSpan,
                "Circular import detected",
                "Remove the cycle or refactor shared code into a separate module.");
        } else if (compiler){
            auto* m = infos[id].module;
            compiler->errorManager.addError(
                ErrorType::Preprocessor,
                PreprocessorErrors::CircularImport,
                ErrorSpan{m->filePath, "import", m->line, m->column},
                "Circular import detected",
                "Remove the cycle or refactor shared code into a separate module.");
        }
        return;
    }

    state[id] = 1; // visiting

    for (const auto& edge : infos[id].dependencies){
        dfsVisit(edge.moduleId, infos, state, order, &edge.span);
    }

    state[id] = 2; // done
    order.push_back(id);
}

std::vector<std::string> Orchestrator::splitPath(const std::string& path){
    std::vector<std::string> result;
    std::string cur;
    for (char c : path){
        char x = (c == '\\') ? '/' : c;
        if (x == '/') {
            if (!cur.empty()) result.push_back(cur); cur.clear();
        } else cur.push_back(x);
    }
    if (!cur.empty()) result.push_back(cur);
    return result;
}

std::string Orchestrator::joinPath(const std::vector<std::string>& parts){
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i){
        if (i) result += '/';
        result += parts[i];
    }
    return result;
}

std::string Orchestrator::dirOfKey(const std::string& key){
    auto norm = key;
    for (auto& ch : norm) if (ch == '\\') ch = '/';
    auto pos = norm.rfind('/');
    if (pos == std::string::npos) return ""; // root of sourceFolder
    return norm.substr(0, pos); // "more"
}

std::string Orchestrator::resolveRelativeKey(const std::string& currentKey, const std::string& importName){
    auto baseDir = dirOfKey(currentKey);
    std::vector<std::string> stack;

    if (!baseDir.empty()) stack = splitPath(baseDir);

    for (auto& part : splitPath(importName)){
        if (part == "." || part.empty()) continue;
        if (part == "..") {
            if (!stack.empty()) stack.pop_back(); // if empty, stay in the root (todo: make an error later for going out of sourceFolder)
        } else { stack.push_back(part); }
    }

    return joinPath(stack);
}

// Main implementations
ProgramUnit Orchestrator::stitchProgram(const EntryPoint& entryPoint, const std::vector<ModuleInfo>& infos){
    ProgramUnit program;
    program.entryFn = entryPoint.function;

    ModuleId entryId = -1;
    for (const auto& info : infos) {
        if (info.module == entryPoint.module) {
            entryId = info.id;
            break;
        }
    }

    program.entryModule = entryId;
    if (entryId < 0) return program; // i mean this should not ever ever happen but if anything just return the program...

    std::vector<uint8_t> state(infos.size(), 0);
    dfsVisit(entryId, infos, state, program.order, nullptr);

    return program;
}

EntryPoint Orchestrator::findEntryPoint(const std::vector<MemoryPtr<ModuleNode>>& modules) {
    EntryPoint entryPoint{};
    EntryPoint mainFallback{};

    bool foundExplicit = false;

    for (const auto& module : modules) {
        if (!module) continue;
        for (const auto& statement : module->body) {
            if (!statement || statement->type != ASTNodeType::Function) continue;

            auto* func = static_cast<FunctionNode*>(statement.get());

            if (!mainFallback.function && func->name == "main") {
                mainFallback.module = module.get();
                mainFallback.function = func;
            }

            if (hasEntryDecorator(func)) {
                if (!foundExplicit) {
                    foundExplicit = true;
                    entryPoint.module = module.get();
                    entryPoint.function = func;
                } else {
                    auto* firstFn = entryPoint.function;
                    compiler->errorManager.addError(
               ErrorType::Analysis,
               AnalysisErrors::MultipleEntryPoints,
               ErrorSpan{func->filePath, func->name, func->line, func->column},
               "Multiple entry points detected.",
               std::format("Keep only one function marked with @entry. First one detected at {}:{}:{}", firstFn->filePath, firstFn->line, firstFn->column));
                }
            }
        }
    }
    if (foundExplicit) return entryPoint;

    if (mainFallback.function) return mainFallback;

    compiler->errorManager.addError(
            ErrorType::Analysis,
            AnalysisErrors::NoEntryPoints,
            ErrorSpan{modules.front()->filePath, "", 1, 1},
            "No entry points found",
            "Add @entry to a function or define fn main().");

    return {};
}

std::vector<ModuleInfo> Orchestrator::resolveImports(const std::vector<MemoryPtr<ModuleNode>>& modules){
    std::vector<ModuleInfo> infos(modules.size());

    // key to module id
    std::unordered_map<std::string, ModuleId> keyToId;
    keyToId.reserve(modules.size() * 2);
    std::vector<std::string> idToKey(modules.size());

    for (ModuleId i = 0; i < (ModuleId)modules.size(); ++i) {
        ModuleNode* m = modules[i].get();
        if (!m) continue;

        std::string key = compiler->projectManager.filePathToKey(m->filePath);
        idToKey[i] = key;
        keyToId[key] = i;

        infos[i].id = i;
        infos[i].module = m;
    }

    // go through imports and fill out dependencies and aliasMap
    for (ModuleId i = 0; i < (ModuleId)infos.size(); ++i)
    {
        ModuleInfo& mi = infos[i];
        ModuleNode* m = mi.module;
        if (!m) continue;

        for (const auto& st : m->body)
        {
            if (!st || st->type != ASTNodeType::Import) continue;

            auto* imp = static_cast<ImportNode*>(st.get());
            const std::string& name = imp->moduleName;

            if (imp->importType == ASTImportType::Relative){
                std::string resolvedKey = resolveRelativeKey(idToKey[mi.id], imp->moduleName);
                auto it = keyToId.find(resolvedKey);
                if (it == keyToId.end()){
                    compiler->errorManager.addError(
                        ErrorType::Preprocessor,
                        PreprocessorErrors::ImportNotFound,
                        ErrorSpan{imp->filePath, imp->moduleName, imp->line, imp->column},
                        std::format("Import not found: '{}'", name),
                        "Check the path or create a module file with said path");
                    continue;
                }

                ModuleId depId = it->second;
                mi.dependencies.push_back(DependencyEdge{depId, ErrorSpan{imp->filePath, imp->moduleName, imp->line, imp->column}});

                // for aliases
                if (!imp->alias.empty()) {
                    if (mi.aliasMap.count(imp->alias))
                        compiler->errorManager.addError(
                        ErrorType::Preprocessor,
                        PreprocessorErrors::ImportAliasConflict,
                        ErrorSpan{imp->filePath, imp->alias, imp->line, imp->column},
                        std::format("Import alias conflict: '{}'", imp->alias),
                        "Use a different alias name.");

                    else mi.aliasMap.emplace(imp->alias, depId);
                }
            }
            else if (imp->importType == ASTImportType::Native){
                // At first we're gonna assume the file is in the same folder, if not, it's really a native import
                // As of now i have no idea how to detect Native modules properly, but if i find out a better solution,
                // i should check the parsePreprocessor() in Parser and fix the detection.
                std::string resolvedKey = resolveRelativeKey(idToKey[mi.id], imp->moduleName);

                auto it = keyToId.find(resolvedKey);
                if (it != keyToId.end()){
                    // is a relative import
                    mi.dependencies.push_back(DependencyEdge{it->second, ErrorSpan{imp->filePath, imp->moduleName, imp->line, imp->column}});
                } else {
                    // is a native import
                    if (imp->moduleName != "std" && !compiler->projectManager.config.dependencies.contains(imp->moduleName)){
                        compiler->errorManager.addError(
                        ErrorType::Preprocessor,
                        PreprocessorErrors::ImportNotFound,
                        ErrorSpan{imp->filePath, imp->moduleName, imp->line, imp->column},
                        std::format("Native package '{}' is not installed", imp->moduleName),
                        "Add it to project dependencies (.nlp) or install the package.");
                    }
                    else mi.nativeImports.push_back(imp->moduleName);
                }

                // Foreign and Foreign Relative will come out with language packs update.
            }
        }
    }

    return infos;
}