

#pragma once

#include <cstdint>

namespace tungsten::host
{
	struct params
	{
		int    argc;
		char** argv;

		size_t permanent_size;
		size_t level_size;
		size_t frame_size;
		size_t scratch_size;
	};

	bool init(params prms);
	bool is_running();
	void quit();

	void error(const char* fmt, ...);
	bool frame(uint64_t delta_us);
}