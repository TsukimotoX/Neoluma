#pragma once

namespace Color {
    namespace Text {
        inline constexpr const char* Black        = "\033[30m";
        inline constexpr const char* Red          = "\033[31m";
        inline constexpr const char* Green        = "\033[32m";
        inline constexpr const char* Yellow       = "\033[33m";
        inline constexpr const char* Blue         = "\033[34m";
        inline constexpr const char* Magenta      = "\033[35m";
        inline constexpr const char* Cyan         = "\033[36m";
        inline constexpr const char* White        = "\033[37m";

        inline constexpr const char* BrightBlack  = "\033[90m";
        inline constexpr const char* BrightRed    = "\033[91m";
        inline constexpr const char* BrightGreen  = "\033[92m";
        inline constexpr const char* BrightYellow = "\033[93m";
        inline constexpr const char* BrightBlue   = "\033[94m";
        inline constexpr const char* BrightMagenta= "\033[95m";
        inline constexpr const char* BrightCyan   = "\033[96m";
        inline constexpr const char* BrightWhite  = "\033[97m";
    }

    namespace Background {
        inline constexpr const char* Black        = "\033[40m";
        inline constexpr const char* Red          = "\033[41m";
        inline constexpr const char* Green        = "\033[42m";
        inline constexpr const char* Yellow       = "\033[43m";
        inline constexpr const char* Blue         = "\033[44m";
        inline constexpr const char* Magenta      = "\033[45m";
        inline constexpr const char* Cyan         = "\033[46m";
        inline constexpr const char* White        = "\033[47m";

        inline constexpr const char* BrightBlack  = "\033[100m";
        inline constexpr const char* BrightRed    = "\033[101m";
        inline constexpr const char* BrightGreen  = "\033[102m";
        inline constexpr const char* BrightYellow = "\033[103m";
        inline constexpr const char* BrightBlue   = "\033[104m";
        inline constexpr const char* BrightMagenta= "\033[105m";
        inline constexpr const char* BrightCyan   = "\033[106m";
        inline constexpr const char* BrightWhite  = "\033[107m";
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
