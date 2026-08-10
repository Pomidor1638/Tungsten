

#include "host.h"
#include "host_local.h"

#include "log/log.h"

#include "../sys/sys.h"
#include "../sys/sys_event.h"

namespace tungsten::host
{
	const char title_failure[] = " -> failure :";
	const char title_ok[]      = " -> ok\n";
	const char title_skip[]    = " -> skip\n";

	namespace
	{
		bool running = false;
	}

	bool is_running()
	{
		return running;
	}

	bool init(int argc, char** argv)
	{
		log::printf("host::\tinit()\n");

		init_params.argc = argc;
		init_params.argv = argv;

		if (  parse_args())
		if (    log_init())
		if (console_init())
		if (   zone_init())
		if ( window_init())
		//if ( client_init())
		{
			running = true;
			return true;
		}

		quit();
		return false;
	}

	void quit()
	{
		log::printf("host::quit()\n");

		window_quit();
		zone_quit();
	}

	void error(const char* fmt, ...)
	{

	}

	bool frame(uint64_t delta_us)
	{
		if (!running)
			return false;

		if (!process_events())
		{
			return false;
		}

		/*
		if (server::is_active())
		{
			server::frame(delta_us);
		}
		*/

		return true;
	}
}