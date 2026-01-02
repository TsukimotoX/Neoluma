#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <map>

// Compiler settings for the project tell the compiler what to set up before building
struct CompilerSettings {
    /** 
     * @brief `Verbose` is an option that allows extra keywords, syntactic sugar. Not recommended for people who loves keeping shape of a language. Recommended for people who don't give a heck and love copypasting answers from StackOverflow.
     * @param true - enables this option
     * @param false - disables this option
    */
    bool verbose = false;
    /**
     * @brief `Baremetal` is an option that toggles the ability to develop software for bare metal hardware. It disables platform-based ABI (including std) and compiles into binary. 
     * @param true - enables this option
     * @param false - disables this option
     */
    bool baremetal = false; 

    struct Memory {
        enum class MemoryOptions { None, Rusty, ARC, Default };

        /**
         * @brief `level` is an option of `Memory` struct that goes through types of Memory options and, depending on chosen option, will manage the memory in the application the preferred way.
         * @param Default - enables default Garbage Collector (Java, C#, others)
         * @param ARC - enables Automatic Reference Counter (Python, JS, others)
         * @param Rusty - enables Borrow Checker (Rust)
         * @param None - No memory management tools (C, C++, maybe others)
         */
        MemoryOptions level = MemoryOptions::Default;
    };
};

struct PlatformTarget {
    enum class Platform { Windows, Linux, MacOS, iOS, Android, Other };
    Platform platform;
    std::string arch;     // x86_64, arm64, wasm32

    std::string toString() const {
        std::string s = arch + "-";

        switch (platform) {
            case Platform::Windows: s += "windows"; break;
            case Platform::Linux:   s += "linux"; break;
            case Platform::MacOS:   s += "macos"; break;
            case Platform::iOS:     s += "ios"; break;
            case Platform::Android: s += "android"; break;
            default:                s += "unknown"; break;
        }
        
        return s;
    }
};

enum class License { MIT, Apache, GPL2, GPL3, BSD2, BSD3, Boost, CC0, Eclipse, AGPL, LGPL, Mozilla, Unlicense, Custom };

enum class PTOutputType { Executable, StaticLibrary, SharedLibrary, Object, IntermediateRepresentation, None };

// ProjectConfig is a struct that allows me to determine project structure.
struct ProjectConfig {
    std::string name = "Untitled Project";
    std::string version = "1.0.0"; // TODO: Make an incremental count for versions.
    std::vector<std::string> author = { "Untitled Author" };
    std::vector<PlatformTarget> targets;
    License license = License::MIT;
    std::string sourceFolder = "src/";
    PTOutputType output = PTOutputType::Executable;
    std::string buildFolder = "build/";
    std::map<std::string, std::string> dependencies;
    std::map<std::string, std::string> tasks;
    std::map<std::string, std::string> tests;
    std::map<std::string, std::string> languagePacks;

    CompilerSettings compilerSettings;
    std::vector<std::string> filesList; // List of files inside the project to feed to compiler.
    std::string sourcePath; // Absolute path to locate the project
};

// ProjectManager is a class that... manages... projects?
struct ProjectManager {
    ProjectConfig config;

    ProjectManager(ProjectConfig& config) : config(config) {};

    // Main functions
    void clean();

    // Helpful utilities
    [[nodiscard]] std::vector<std::string> listFiles() const { return config.filesList; }
    void addFile(const std::string& file) { config.filesList.push_back(file); }
    void removeFile(const std::string& file) {  config.filesList.erase(std::remove(config.filesList.begin(), config.filesList.end(), file), config.filesList.end()); }

    // Access to ProjectConfig
    ProjectConfig& getConfig();
};