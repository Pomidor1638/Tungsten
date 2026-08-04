
#include "sys.h"
#include "sys_local.h"

#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <SDL2/SDL.h>

namespace tungsten::sys
{
	bool init(int argc, char** argv)
	{
		if (SDL_Init(SDL_INIT_EVERYTHING))
		{
			error("tungsten::sys::init() : can't init SDL : %s\n", SDL_GetError());
			return false;
		}

		return true;
	}

	void quit()
	{
		SDL_Quit();
	}

	uint64_t time_us()
	{
		return /*for us scale*/1'000'000 * SDL_GetPerformanceCounter() / /*ticks per second*/SDL_GetPerformanceFrequency();
	}

	void error(const char* fmt, ...)
	{
		static char error_text[1024];
		snprintf(error_text, sizeof(error_text), fmt);

		printf("%s\n", error_title);
		printf(error_text);

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SYS ERROR", error_text, nullptr);

		std::exit(EXIT_FAILURE);
	}

	void* alloc_mem_block(int align, size_t size)
	{
		return malloc(size);
	}
}