#pragma once
#define APPNAME "Renderer"
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
inline void setConsoleAttribute(WORD attribute) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, attribute);
}
inline std::ostream &red(std::ostream &os) {
    setConsoleAttribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
    return os;
}
inline std::ostream &yellow(std::ostream &os) {
    setConsoleAttribute(FOREGROUND_RED | FOREGROUND_GREEN);
    return os;
}
inline std::ostream &white(std::ostream &os) {
    setConsoleAttribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return os;
}
inline std::ostream &green(std::ostream &os) {
    setConsoleAttribute(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    return os;
}
inline void printLive(const std::string &message) {
    static std::string lastLine;
    int pad = std::max<int>(0, lastLine.size() - message.size());
    std::cout << "\r" << message << std::string(pad, ' ') << std::flush;
    lastLine = message;
}
#else
inline void setConsoleAttribute(int attribute [[maybe_unused]]) {
}
inline std::ostream &red(std::ostream &os) {
    return os;
}
inline std::ostream &yellow(std::ostream &os) {
    return os;
}
inline std::ostream &white(std::ostream &os) {
    return os;
}
inline std::ostream &green(std::ostream &os) {
    return os;
}
inline void printLive(const std::string &message) {
    static std::string lastLine;
    int pad = std::max<int>(0, lastLine.size() - message.size());
    std::cout << "\r" << message << std::string(pad, ' ') << std::flush;
    lastLine = message;
}
#endif
#define CLEARLINE "\r" << std::string(100, ' ') << "\r"
#define LOG_INFO(x) std::cout << CLEARLINE << "[" << APPNAME << "]:" << x;
#define LOG_WARN(x) std::cout << CLEARLINE << yellow << "[" << APPNAME << "]:[WARN]:" << x << white << "\n";
#define LOG_ERROR(x) std::cout << CLEARLINE << red << "[" << APPNAME << "]:[ERROR]:" << x << white << "\n";
#define LOG_SUCCESS(x) std::cout << CLEARLINE << green << "[" << APPNAME << "]:" << x << white << "\n";
