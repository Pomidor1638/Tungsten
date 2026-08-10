
#pragma once

#include <cstdint>

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

	enum class KEY_CODE : uint32_t
	{
		none = 0,
	};

	struct sys_event_key // and mouse button
	{
		KEY_CODE code;
		bool down;
		bool repeat;
	};

	struct sys_event_mouse_motion
	{
		int x ,  y;
		int dx, dy;
	};

	struct sys_event_mouse_wheel
	{
		int   delta;
		float precision;
		bool  direction;
	};

	struct sys_event
	{
		sys_event_type type = sys_event_type::none;
		union
		{
			sys_event_key          key;
			sys_event_mouse_motion mouse_motion;
			sys_event_mouse_wheel  mouse_wheel;
		};
	};

	bool poll_event(sys_event& event);
}