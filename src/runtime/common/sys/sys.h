
#pragma once

#include <cstdint>

namespace tungsten::sys
{
	bool init(int argc, char** argv);
	void quit();

	uint64_t time_us();

	void error(const char* fmt, ...);
	void* alloc_mem_block(int align, size_t size);
}