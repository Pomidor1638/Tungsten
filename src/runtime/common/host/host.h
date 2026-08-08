

#pragma once

#include <cstdint>

namespace tungsten::host
{
	struct params
	{
		int      argc			= 0;
		char**   argv			= nullptr;

		size_t   permanent_size = 0;
		size_t   level_size		= 0;
		size_t   frame_size		= 0;
		size_t   scratch_size	= 0;

		bool     dedicated		= false;
		uint32_t thread_count	= 1;
	};

	bool init(int argc, char** argv);
	bool is_running();
	void quit();

	bool frame(uint64_t delta_us);
}