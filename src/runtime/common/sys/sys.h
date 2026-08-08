
#pragma once

#include <cstdint>
#include <cstdarg>

namespace tungsten::sys
{
	bool init(int argc, char** argv);
	void quit();

	uint64_t time_us();

	int native_threads_count();

	/*call only once at init*/
	void* alloc_mem_block(size_t align, size_t size);
	void free_mem_block(void* ptr);
}