
#pragma once

#include <cstdarg>

namespace tungsten::log
{
    bool init();
    void quit();

    int vprintf(const char* fmt, va_list args);
	int printf(const char* fmt, ...);

    void warning(const char* fmt, ...);
    void error(const char* fmt, ...);
    void debug(const char* fmt, ...);
}