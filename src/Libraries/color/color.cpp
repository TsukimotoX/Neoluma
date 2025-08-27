#include "color.hpp"

namespace Color {
    std::string TextRGB(uint8_t r, uint8_t g, uint8_t b) {
        return std::format("\033[38;2;{};{};{}m", r, g, b);
    }

    std::string BackgroundRGB(uint8_t r, uint8_t g, uint8_t b) {
        return std::format("\033[48;2;{};{};{}m", r, g, b);
    }
    
    std::string TextHex(const std::string& hex) {
        unsigned int r, g, b;
        if (hex[0] == '#') {
            sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
            return TextRGB(r, g, b);
        }
        return "";
    }

    std::string BackgroundHex(const std::string& hex) {
        unsigned int r, g, b;
        if (hex[0] == '#') {
            sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
            return BackgroundRGB(r, g, b);
        }
        return "";
    }
}