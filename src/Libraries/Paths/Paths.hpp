/*
 * Path is an internal Neoluma library for handling paths near Neoluma executable installation.
 * Really, i made it just for that.
 */

#pragma once
#include <string>

struct Paths {
    static std::string executablePath();
    static std::string executableDir();
    static std::string join(const std::string& a, const std::string& b);
};