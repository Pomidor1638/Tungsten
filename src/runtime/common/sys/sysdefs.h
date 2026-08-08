
#pragma once

#include <cstdint>
#include <cstdarg>

namespace tungsten::sys
{

	void error(const char* fmt, ...);
	int printf(const char* fmt, ...);
	int vprintf(const char* fmt, va_list args);


	enum class sys_event_type
	{
		none = 0,

	};

	struct sys_event
	{
		sys_event_type type = sys_event_type::none;
		union
		{

		};
	};
}