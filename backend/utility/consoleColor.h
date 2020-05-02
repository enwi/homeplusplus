#ifndef _CONSOLECOLOR_H
#define _CONSOLECOLOR_H

#if defined(WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

namespace console
{
    namespace color
    {
        enum code
        {
#if defined(WIN32) || defined(_WIN64)
            FG_RED = 12,
            FG_GREEN = 10,
            FG_YELLOW = 14,
            FG_BLUE = 9,
            FG_MAGENTA = 13,
            FG_CYAN = 11,
            FG_LIGHT_GREY = 7,
            FG_DARK_GREY = 8,
            FG_LIGHT_RED = 12,
            FG_LIGHT_GREEN = 10,
            FG_LIGHT_BLUE = 9,
            FG_LIGHT_MAGENTA = 13,
            FG_LIGHT_CYAN = 11,
            FG_WHITE = 15,
            RESET = 15,
#elif defined(__linux__) || defined(__APPLE__)
            FG_RED = 31,
            FG_GREEN = 32,
            FG_YELLOW = 33,
            FG_BLUE = 34,
            FG_MAGENTA = 35,
            FG_CYAN = 36,
            FG_LIGHT_GREY = 37,
            FG_DARK_GREY = 90,
            FG_LIGHT_RED = 91,
            FG_LIGHT_GREEN = 92,
            FG_LIGHT_YELLOW = 93,
            FG_LIGHT_BLUE = 94,
            FG_LIGHT_MAGENTA = 95,
            FG_LIGHT_CYAN = 96,
            FG_WHITE = 97,
            RESET = 0,
#endif
        };
    } // namespace color
    void SetColor(color::code c)
    {
#if defined(WIN32) || defined(_WIN64)
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, c);
#elif defined(__linux__) || defined(__APPLE__)
        std::cout << "\033[1;" << c << "m";
#endif
    }
} // namespace console

#endif // _CONSOLECOLOR_H
