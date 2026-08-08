

#include "host.h"
#include "hostdefs.h"
#include "host_local.h"

#include "../sys/sys.h"

namespace tungsten::host
{
	const char title_failure[] = " -> failure :";
	const char title_ok[] = " -> ok\n";
	const char title_skip[] = " -> skip\n";

	namespace
	{
		bool zone_initialized = false;
		bool running = false;
	}

	bool is_running()
	{
		return running;
	}

	bool init(int argc, char** argv)
	{
		sys::printf("host::\tinit()\n");

		init_params.argc = argc;
		init_params.argv = argv;

		if (!parse_args()) goto init_failure;
		if (!zone_init()) goto init_failure;
		if (!window_init()) goto init_failure;
		//if (!client_init()) goto init_failure;

		//running = true;
		return true;

	init_failure:
		quit();
		return false;
	}

	void quit()
	{
		sys::printf("host::quit()\n");

		window_quit();
		zone_quit();
	}

	void error(const char* fmt, ...)
	{}

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

		return true;
	}
}