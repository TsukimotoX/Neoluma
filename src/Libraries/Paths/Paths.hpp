/*
 * Path is an internal Neoluma library for handling paths near Neoluma executable installation.
 * Really, i made it just for that.
 */

#pragma once
#include <string>

struct Paths {
    ~Paths(){}
    static std::string executablePath();

    static std::string rootDir();
    static std::string dataDir();
    static std::string userDataDir();

    static std::string join(const std::string& a, const std::string& b);
};