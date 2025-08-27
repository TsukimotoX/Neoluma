#pragma once

#include <iostream>

namespace Color {
    std::string TextRGB(uint8_t r, uint8_t g, uint8_t b);

    std::string BackgroundRGB(uint8_t r, uint8_t g, uint8_t b);

    std::string TextHex(const std::string& hex);

    std::string BackgroundHex(const std::string& hex);

    namespace Effect {
        constexpr const char* BoldOn       = "\033[1m";
        constexpr const char* DimOn        = "\033[2m";
        constexpr const char* UnderlineOn  = "\033[4m";
        constexpr const char* BlinkOn      = "\033[5m";
        constexpr const char* ReverseOn    = "\033[7m";
        constexpr const char* HideOn       = "\033[8m";

        constexpr const char* BoldOff      = "\033[21m";
        constexpr const char* DimOff       = "\033[22m";
        constexpr const char* UnderlineOff = "\033[24m";
        constexpr const char* BlinkOff     = "\033[25m";
        constexpr const char* ReverseOff   = "\033[27m";
        constexpr const char* HideOff      = "\033[28m";
    }

    constexpr const char* Reset = "\033[0m";
}
