
#include "host_local.h"

#include "../sys/sys.h"
#include "../sys/sys_event.h"

namespace tungsten::host
{

	namespace
	{

	}

	bool process_events()
	{
		sys::sys_event e;
		while (sys::poll_event(e))
		{
			switch (e.type)
			{
			case sys::sys_event_type::quit:
				return false;
			default:
				break;
			}
		}

		return true;
	}

}