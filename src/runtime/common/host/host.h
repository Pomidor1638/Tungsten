

#pragma once

#include <cstdint>

namespace tungsten::host
{
	// host.cpp
	bool init(int argc, char** argv);
	bool is_running();
	void quit();

	bool frame(uint64_t delta_us);
}