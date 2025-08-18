/* 
* Asker is an internal Neoluma library for handling CLI input
*/
#include <iostream>
#include <print>
#include "../color/color.hpp"
#include "../../CLI/cli.hpp"
enum Key { NONE, UP, DOWN, ENTER, BACKSPACE };

#ifdef _WIN32
#include <conio.h>

Key getKey() {
    int c = _getch();
    if (c == 13) return ENTER;
    if (c == 8) return BACKSPACE;
    if (c == 224 || c == 0) {
        c = _getch();
        if (c == 72) return UP;
        if (c == 80) return DOWN;
    }
    return NONE;
}

void enableRaw() {} // do nothing on windows
void disableRaw() {}
#else
#include <termios.h>
#include <unistd.h>

static struct termios term;

void enableRaw() {
    tcgetattr(STDIN_FILENO, &term);
    struct termios raw = term;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disableRaw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

Key getKey() {
    int c = getchar();
    if (c == '\n') return ENTER;
    if (c == 127) return BACKSPACE;
    if (c == 27) {
        char sequence[2];
        if (read(STDIN_FILENO, sequence, 2) == 2 && seq[0] == '[') {
            if (seq[1] == 'A') return UP;
            if (seq[1] == 'B') return DOWN;
        }
    }
    return NONE;
}
#endif

void clearBlock(int lines) {
    std::println("\033[{}A",lines);
}

namespace asker {
    inline constexpr const char* Clear = "\033[0m\033[2J\033[H";

    inline std::string input(const std::string& question, bool required=false) {
        std::cout << Color::TextHex("#75b5ff") << ">> " << question << ' ' << Color::TextHex("#ff28e6");
        std::string response;
        while (std::getline(std::cin, response)) {
            if (required && response.length() == 0) {}
            else break;
        }
        std::cout << Color::Reset;
        return response;
    }

    inline bool confirm(const std::string& question) {
        bool result = true;
        std::string response;
        response = input(question + Color::TextHex("#e84b85") + " (y/n) " + Color::TextHex("#ff28e6"));
        if (response[0] != 'y' && response[0] != 'Y' && response[0] != '\0' ) result = false;
        std::cout << Color::Reset;
        return result;
    }

    template <size_t n>
    inline std::string selectList(const std::string& question, const std::string (&options)[n]) {
        enableRaw();

        int pos = 0;
        Key key = NONE;
        int lines = n + 2;

        while (true) {
            clearBlock(lines);

            std::cout << Color::TextHex("#75b5ff") << ">> " << question << '\n';

            for (int i = 0; i < n; i++) {
                if (i == pos) { 
                    std::cout << Color::TextHex("#00ff48ffff") << "> " << Color::TextHex("#ff28e6") << options[i] << '\n';
                }
                else { 
                    std::cout << Color::TextHex("#ff28e6") << "  " << options[i] << '\n';
                }
            }

            key = getKey();
            if (key == UP && pos > 0) pos--;
            else if (key == DOWN && pos < n - 1) pos++;
            else if (key == ENTER) break;
        }

        disableRaw();
        std::cout << Color::Reset << "\n";
        return options[pos];
    }
};