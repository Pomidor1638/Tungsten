
#include "host.h"

#include <SDL2/SDL.h>

#include "../sys/sys.h"
#include "../../tclient/client.h"
#include "../common/host/memory/memory.h"

namespace tungsten::host
{
	namespace
	{
		bool running = false;
	}


	bool is_running()
	{
		return running;
	}


	void w_init()
	{
		client::window = memory::permanent_zone.allocate<client::window::Window, 1>
			(
				"Tungsten",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				1024,
				768,
				SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
			);

	}


	bool init(params prms)
	{
		if (!memory::init_zones(prms.permanent_size, prms.level_size, prms.frame_size, prms.scratch_size))
		{
			return false;
		}

		

		client::init();
		running = true;
	}

	void quit()
	{
		client::quit();
	}

	void error(const char* fmt, ...)
	{
		sys::error(fmt);
	}

	bool frame(uint64_t delta_us)
	{
		if (!running)
			return false;

		client::frame(delta_us);
	}
}