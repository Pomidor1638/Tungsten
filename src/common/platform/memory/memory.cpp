
#include "memory.h"
#include <cstring>

namespace tungsten::platform::memory
{
	void* memset(void* dst, int val, size_t size)
	{
		return std::memset(dst, val, size);
	}
	void* memcpy(void* dst, const void* src, size_t size)
	{
		return std::memcpy(dst, src, size);
	}
	int memcmp(const void* buf1, const void* buf2, size_t size)
	{
		return std::memcmp(buf1, buf2, size);
	}
}