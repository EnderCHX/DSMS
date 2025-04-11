//
// Created by c on 2025/4/11.
//

#include "log.h"

namespace chx_log {
    const char* cBlue    = "\033[1;34m";
    const char* cRed      ="\033[1;31m";
    const char* cGreen    ="\033[1;32m";
    const char* cCyan     ="\033[1;36m";
    const char* cMagenta  ="\033[1;35m";
    const char* cYellow   ="\033[1;33m";
    const char* cBlack    ="\033[1;30m";
    const char* cWhite    ="\033[1;37m";
    const char* cEnd      ="\033[0m";
    void Log::info(const std::string& str){
        std::lock_guard<std::mutex> lock(cout_lock);
        time_t now = time(0);
        tm tm_now = *localtime(&now);
        std::cout << std::format("{}[{}-{}-{}-{}:{}:{}]{} {}[INFO]{} {}{}{}\n",
            cGreen,
            tm_now.tm_year + 1900,
            tm_now.tm_mon + 1,
            tm_now.tm_mday,
            tm_now.tm_hour,
            tm_now.tm_min,
            tm_now.tm_sec,
            cEnd,
            cBlue,
            cEnd,
            cCyan,
            str,
            cEnd);
    }
    void Log::warn(const std::string& str){
        std::lock_guard<std::mutex> lock(cout_lock);
        time_t now = time(0);
        tm tm_now = *localtime(&now);
        std::cout << std::format("{}[{}-{}-{}-{}:{}:{}]{} {}[WARN]{} {}{}{}\n",
            cGreen,
            tm_now.tm_year + 1900,
            tm_now.tm_mon + 1,
            tm_now.tm_mday,
            tm_now.tm_hour,
            tm_now.tm_min,
            tm_now.tm_sec,
            cEnd,
            cYellow,
            cEnd,
            cCyan,
            str,
            cEnd);
    }
    void Log::error(const std::string& str){
        std::lock_guard<std::mutex> lock(cout_lock);
        time_t now = time(0);
        tm tm_now = *localtime(&now);
        std::cout << std::format("{}[{}-{}-{}-{}:{}:{}]{} {}[ERRO]{} {}{}{}\n",
            cGreen,
            tm_now.tm_year + 1900,
            tm_now.tm_mon + 1,
            tm_now.tm_mday,
            tm_now.tm_hour,
            tm_now.tm_min,
            tm_now.tm_sec,
            cEnd,
            cRed,
            cEnd,
            cCyan,
            str,
            cEnd);
    }
    void Log::debug(const std::string &str) {
        std::lock_guard<std::mutex> lock(cout_lock);
        time_t now = time(0);
        tm tm_now = *localtime(&now);
        std::cout << std::format("{}[{}-{}-{}-{}:{}:{}]{} {}[DBUG]{} {}{}{}\n",
            cGreen,
            tm_now.tm_year + 1900,
            tm_now.tm_mon + 1,
            tm_now.tm_mday,
            tm_now.tm_hour,
            tm_now.tm_min,
            tm_now.tm_sec,
            cEnd,
            cBlack,
            cEnd,
            cCyan,
            str,
            cEnd);
    }
}
