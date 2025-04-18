//
// Created by c on 2025/4/11.
//

#ifndef LOG_H
#define LOG_H

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
inline  void enable_ansi_escape() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
}
#else
inline void enable_ansi_escape() {}
#endif



#include <iostream>
#include <mutex>
#include <format>

namespace CHX {
    extern const char* cBlue ;
    extern const char* cRed;
    extern const char* cGreen;
    extern const char* cCyan;
    extern const char* cMagenta;
    extern const char* cYellow;
    extern const char* cBlack;
    extern const char* cWhite;
    extern const char* cEnd;

    class Log {
    public:
        Log() {
            enable_ansi_escape();
        }
        void Info(const std::string& str);
        void Error(const std::string& str);
        void Debug(const std::string& str);
        void Warn(const std::string& str);
    private:
        auto msg(const std::string& level, const std::string& str) -> void;
        std::mutex cout_lock;
    };
}



#endif //LOG_H
