

#include <SDL2/SDL.h>

#include "host.h"
#include "hostdefs.h"

#include "../sys/sys.h"
#include "../sys/sysdefs.h"

#include "../../tclient/client.h"

#include "../common/host/zone/zone.h"

namespace tungsten::host
{
	namespace
	{
		void* memory_block = nullptr;
		size_t memory_block_size = 0;

		bool initialized = false;
		bool running = false;

		params init_params{};

		bool parse_args()
		{
			init_params.permanent_size = 32 * 1024 * 1024ull;
			init_params.level_size     = 64 * 1024 * 1024ull;
			init_params.frame_size     = 16 * 1024 * 1024ull;
			init_params.scratch_size   = 16 * 1024 * 1024ull;

			return true;
		}



		bool zone_init()
		{
			sys::printf("\tzone_init()\n");

			constexpr size_t align = 32;

			memory_block_size = init_params.permanent_size + init_params.level_size + init_params.frame_size + init_params.scratch_size;
			memory_block = sys::alloc_mem_block(align, memory_block_size);

			if (!memory_block)
			{
				sys::error("zone_init() - can't allocate memory block:\nalign:\t%llu\nsize: \t%llu \tb\n\t%llu \t\tKib\n\t%llu \t\tMib\n", 
					align, 
					memory_block_size, 
					memory_block_size >> 10, 
					memory_block_size >> 20
				);
				return false;
			}

			if (!zone::init(
				memory_block,
				memory_block_size,
				init_params.permanent_size,
				init_params.level_size,
				init_params.frame_size,
				init_params.scratch_size
			)) {
				return false;
			}
		}

		void zone_quit()
		{
		}

		bool window_init()
		{
			return false;
		}

		void window_quit()
		{
		}

		bool client_init()
		{
			sys::printf("\tclient::init()\n");
			if (!client::init())
			{
				sys::error("client_init() : can't init\n");
				return false;
			}

			return true;
		}

		void client_quit()
		{
		}

	}


	bool is_running()
	{
		return running;
	}



	bool init(int argc, char** argv)
	{
		sys::printf("host::init()\n");

		init_params.argc = argc;
		init_params.argv = argv;

		if (!parse_args()) goto fail;
		if (!zone_init()) goto fail;
		if (!window_init()) goto fail;
		if (!client_init()) goto fail;


		running = true;
		return true;

	fail:
		quit();
		return false;
	}

	void quit()
	{

		sys::printf("host::quit()\n");

		sys::printf("\tclient::quit()\n");
		client::quit();

		sys::printf("\tzone::quit()\n");
		zone::quit();
	}

	void error(const char* fmt, ...)
	{
	}

	bool frame(uint64_t delta_us)
	{
		if (!running)
			return false;

		/*
		if (server::is_active())
		{
			server::frame(delta_us);
		}
		*/

		return client::frame(delta_us);
	}
}