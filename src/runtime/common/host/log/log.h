
#pragma once

namespace tungsten::log
{
	int printf(const char* fmt, ...);

    void warning(const char* fmt, ...);
    void error(const char* fmt, ...);
    void debug(const char* fmt, ...);
}