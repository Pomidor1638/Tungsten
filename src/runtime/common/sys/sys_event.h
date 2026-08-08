
#pragma once

namespace tungsten::sys
{
	enum class sys_event_type
	{
		none = 0,

		quit,

		key_up,
		key_down,

		mouse_wheel,
		mouse_motion,

		packet,
	};

	struct sys_event
	{
		sys_event_type type = sys_event_type::none;
		union
		{

		};
	};

	bool poll_event(sys_event& event);
}