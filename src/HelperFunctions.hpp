#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

std::string trim(const std::string& s);

// Splits the string by delimeter
std::vector<std::string> split(std::string str, char delimiter);

// Reads the file
std::string readFile(const std::string& filePath);