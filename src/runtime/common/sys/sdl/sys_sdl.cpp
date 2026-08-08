
#include "../sys.h"
#include "../sysdefs.h"
#include "../sys_local.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <SDL2/SDL.h>


namespace tungsten::sys
{
	namespace
	{
	}

	bool init(int argc, char** argv)
	{
		sys::printf("sys::init()\n");
		if (SDL_Init(SDL_INIT_EVERYTHING))
		{
			error("sys::init() : can't init SDL : %s\n", SDL_GetError());
			return false;
		}
		return true;
	}

	void quit()
	{
		sys::printf("sys::quit()\n");
		SDL_Quit();
	}

	uint64_t time_us()
	{
		return /*for us scale*/1'000'000 * SDL_GetPerformanceCounter() / /*ticks per second*/SDL_GetPerformanceFrequency();
	}


	int vprintf(const char* fmt, va_list args)
	{
		return std::vprintf(fmt, args);
	}

	int printf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		
		int result = vprintf(fmt, args);
		
		va_end(args);
		return result;
	}

	void error(const char* fmt, ...)
	{
		static char error_text[1024];

		va_list args;
		va_start(args, fmt);
		vsnprintf(error_text, sizeof(error_text), fmt, args);
		va_end(args);

		printf("%s\n", error_title);
		printf("%s", error_text);

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SYS ERROR", error_text, nullptr);

		std::exit(EXIT_FAILURE);
	}


	void* alloc_mem_block(size_t align, size_t size)
	{
#if defined(_WIN32)
		return _aligned_malloc(size, align);
#else
		size = (size + align - 1) & ~(align - 1);
		return std::aligned_alloc(align, size);
#endif
	}

	void free_mem_block(void* ptr)
	{
#if defined(_WIN32)
		_aligned_free(ptr);
#else
		std::free(ptr);
#endif
	}

}