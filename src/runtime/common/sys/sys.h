
#pragma once

#include <cstdint>
#include <cstdarg>

namespace tungsten::sys
{
	// sys_*.cpp
	// where '*' - is an platform name

	bool init(int argc, char** argv);
	void quit();

	uint64_t time_us();

	int native_threads_count();

	/*calls ONLY ONCE at init*/

	void* alloc_mem_block(size_t align, size_t size);
	void free_mem_block(void* ptr);


	[[noreturn]] void panic(const char* fmt, ...);

	int printf(const char* fmt, ...);
	int vprintf(const char* fmt, va_list args);

}