#pragma once

namespace Color {
    inline std::string TextRGB(uint8_t r, uint8_t g, uint8_t b) {
        return std::format("\033[38;2;{};{};{}m", r, g, b);
    }

    inline std::string BackgroundRGB(uint8_t r, uint8_t g, uint8_t b) {
        return std::format("\033[48;2;{};{};{}m", r, g, b);
    }

    inline std::string TextHex(const std::string& hex) {
        unsigned int r, g, b;
        if (hex[0] == '#') {
            sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
            return TextRGB(r, g, b);
        }
        return "";
    }

    inline std::string BackgroundHex(const std::string& hex) {
        unsigned int r, g, b;
        if (hex[0] == '#') {
            sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
            return BackgroundRGB(r, g, b);
        }
        return "";
    }

    namespace Effect {
        inline constexpr const char* BoldOn       = "\033[1m";
        inline constexpr const char* DimOn        = "\033[2m";
        inline constexpr const char* UnderlineOn  = "\033[4m";
        inline constexpr const char* BlinkOn      = "\033[5m";
        inline constexpr const char* ReverseOn    = "\033[7m";
        inline constexpr const char* HideOn       = "\033[8m";

        inline constexpr const char* BoldOff      = "\033[21m";
        inline constexpr const char* DimOff       = "\033[22m";
        inline constexpr const char* UnderlineOff = "\033[24m";
        inline constexpr const char* BlinkOff     = "\033[25m";
        inline constexpr const char* ReverseOff   = "\033[27m";
        inline constexpr const char* HideOff      = "\033[28m";
    }

    inline constexpr const char* Reset = "\033[0m";
}
