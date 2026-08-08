

#include "../sys/sys.h"

#include "host_local.h"
#include "zone/zone.h"

namespace tungsten::host
{
	namespace
	{
		void*  memory_block      = nullptr;
		size_t memory_block_size = 0;

		bool   zone_initialized  = false;
	}

	bool zone_init()
	{
		sys::printf("\tzone_init()\n");

		constexpr size_t align = 32;
		memory_block_size = init_params.permanent_size + init_params.level_size + init_params.frame_size + init_params.scratch_size;

		sys::printf("\t\talloc_memory_block(align: %llu, size: %llu)", align, memory_block_size);

		memory_block = sys::alloc_mem_block(align, memory_block_size);

		if (!memory_block)
		{
			sys::printf("%s memory_block == nullptr", title_failure);
			return false;
		}

		sys::printf(title_ok);

		return zone_initialized = zone::init(
			memory_block,
			memory_block_size,
			init_params.permanent_size,
			init_params.level_size,
			init_params.frame_size,
			init_params.scratch_size
		);
	}

	void zone_quit()
	{
		sys::printf("\tzone_quit()\n");
		if (zone_initialized)
		{
			// free allocators first
			zone::quit();

			sys::printf("\t\tfree memory_block");
			if (memory_block)
			{
				sys::free_mem_block(memory_block);
				memory_block_size = 0;
				memory_block = nullptr;
				zone_initialized = false;
				sys::printf(title_ok);
			}
			else
			{
				sys::printf(title_skip);
			}
		}
		else
		{
			sys::printf(title_ok);
		}
	}
}