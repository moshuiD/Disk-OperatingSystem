#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Success
};

class Log {
private:
    static inline std::string GetCurrTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
        std::tm tmNow;
        localtime_s(&tmNow, &timeNow);
        std::ostringstream oss;
        oss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    static void SetColor(LogLevel level) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level) {
        case LogLevel::Info:
            SetConsoleTextAttribute(hConsole, 7);
            break;
        case LogLevel::Warning:
            SetConsoleTextAttribute(hConsole, 14);
            break;
        case LogLevel::Error:
            SetConsoleTextAttribute(hConsole, 4);
            break;
        case LogLevel::Success:
            SetConsoleTextAttribute(hConsole, 10);
            break;
        }
    }

    static void ResetColor() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 7);
    }
public:
    template<typename... Args>
    static void Write(LogLevel level, Args&&... args) {
        SetColor(level);
        switch (level)
        {
        case LogLevel::Info:
            std::cout << "[+] " << GetCurrTime() << " ";
            break;
        case LogLevel::Warning:
            std::cout << "[!] " << GetCurrTime() << " ";
            break;
        case LogLevel::Error:
            std::cout << "[x] " << GetCurrTime() << " ";
            break;
        case LogLevel::Success:
            std::cout << "[¡Ì] " << GetCurrTime() << " ";
            break;
        }
        
        ((std::cout << std::forward<Args>(args) << " "), ...);
        std::cout << std::endl;
        ResetColor();
    }

    template<typename... Args>
    static void Info(Args&&... args) {
        Write(LogLevel::Info, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Warning(Args&&... args) {
        Write(LogLevel::Warning, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Error(Args&&... args) {
        Write(LogLevel::Error, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Success(Args&&... args) {
        Write(LogLevel::Success, std::forward<Args>(args)...);
    }
};