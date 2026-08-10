
#include "log.h"
#include "../../sys/sys.h"

namespace tungsten::log
{
	bool init();
	void quit();

	int vprintf(const char* fmt, va_list args)
	{
		return sys::vprintf(fmt, args);
	}

	int printf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		int count = sys::vprintf(fmt, args);
		va_end(args);
		return count;
	}

	void warning(const char* fmt, ...)
	{
		static const char warning_title[]=
R"(
)";

	}

	void error(const char* fmt, ...)
	{
		static const char error_title[] =
R"(
)";
	}
	void debug(const char* fmt, ...)
	{
		static const char debug_title[] =
R"(
)";
	}
}