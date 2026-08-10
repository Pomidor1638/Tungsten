

#include "host_local.h"

#include "../sys/sys.h"
#include "log/log.h"

namespace tungsten::host
{
	params init_params{};

	bool parse_args()
	{
		log::printf("\tparse_args() -> ok\n");

		init_params.permanent_size = 32 * 1024 * 1024ull;
		init_params.level_size     = 64 * 1024 * 1024ull;
		init_params.frame_size     = 16 * 1024 * 1024ull;
		init_params.scratch_size   = 16 * 1024 * 1024ull;

		return true;
	}

}