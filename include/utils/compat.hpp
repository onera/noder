#ifndef COMPAT_HPP
#define COMPAT_HPP

#include <stddef.h>

#ifndef HAVE_SSIZE_T
#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN     // Prevents windows.h from including lots of unnecessary stuff
#define NOMINMAX                // Prevents min/max macros from clashing with std::min/max
#include <windows.h>
#endif

#include <string>
#include <iostream>

inline bool isOutputRedirected() {
    #ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        return !GetConsoleMode(hOut, &mode);  // If GetConsoleMode fails, output is redirected
    #else
        return false;
    #endif
}

inline void printUTF8Line(const std::string& utf8Text, std::ostream& os = std::cout) {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        if (GetConsoleMode(hConsole, &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            // Only use WriteConsoleW if output is NOT redirected
            if (!isOutputRedirected()) {
                int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, nullptr, 0);
                if (wideLen > 0) {
                    std::wstring wideStr(wideLen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, &wideStr[0], wideLen);
                    DWORD written;
                    WriteConsoleW(hConsole, wideStr.c_str(), wideLen - 1, &written, nullptr);
                    return;
                }
            }
        }
    #endif
        // Fallback: normal output
        os << utf8Text;
    }


inline bool isAnsiColorSupported() {
    #ifdef _WIN32
        static bool cached = false;
        static bool checked = false;
        if (!checked) {
            checked = true;
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode;
            if (GetConsoleMode(hOut, &mode)) {
                cached = (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }
        return cached;
    #else
        return true;  // assume ANSI is supported on non-Windows systems
    #endif
    }

#endif 
