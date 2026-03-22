#pragma once
#include "../Nodes.hpp"
#include "Core/Frontend/ErrorManager/ErrorManager.hpp"
#include <unordered_map>

// helper declarations
using ModuleId = int;
struct Compiler;

struct EntryPoint {
    ModuleNode* module = nullptr;
    FunctionNode* function = nullptr;
};

struct DependencyEdge {
    ModuleId moduleId;
    ErrorSpan span; // where did the error happen
};

// main data declarations

struct ModuleInfo {
    ModuleId id;
    ModuleNode* module;
    std::vector<DependencyEdge> dependencies;
    std::unordered_map<std::string, ModuleId> aliasMap;
    std::vector<std::string> nativeImports; // for packages that import from dependencies or std, like "math", "std", "tazer"
};

struct ProgramUnit {
    ModuleId entryModule = -1; // From what module do we start the program execution?
    FunctionNode* entryFn = nullptr; // What function is used to be an entry one?
    std::vector<ModuleId> order; // Dependency-first order list (for Semantic Analysis)
};

// TODO: namespaces support

// Orchestrator is a struct that allows us to stitch the project together into a working program, that can be fed to the Semantic Analysis and lower parts of the Compiler.
struct Orchestrator {
    Compiler* compiler = nullptr;
    void setCompiler(Compiler* comp) { compiler = comp; }

    // main function
    ProgramUnit stitchProgram(const EntryPoint& entryPoint, const std::vector<ModuleInfo>& infos);
    EntryPoint findEntryPoint(const std::vector<MemoryPtr<ModuleNode>>& modules);
    std::vector<ModuleInfo> resolveImports(const std::vector<MemoryPtr<ModuleNode>>& modules);

    // helper functions
    static bool hasEntryDecorator(const FunctionNode* function);
    void dfsVisit(ModuleId id, const std::vector<ModuleInfo>& infos, std::vector<uint8_t>& state, std::vector<ModuleId>& order, const ErrorSpan* fromSpan);

    static std::vector<std::string> splitPath(const std::string& path);
    static std::string joinPath(const std::vector<std::string>& parts);
    static std::string dirOfKey(const std::string& key);
    static std::string resolveRelativeKey(const std::string& currentKey, const std::string& importName);
};