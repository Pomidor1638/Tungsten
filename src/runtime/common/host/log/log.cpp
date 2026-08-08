
#include "log.h"
#include "../../sys/sys.h"

namespace tungsten::log
{
	int printf(const char* fmt, ...)
	{
		return sys::printf(fmt);
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