#pragma once
#include <cstdint>
#include "../world/world.h"

enum class GlobalGameState
{
	NONE = 0, // In Main Menu
	LOADING,
	IN_GAME,
};

enum class InGameState : uint64_t
{
	NONE        =       0,
	MAIN_RENDER = (1 << 0),
	SHOW_MENU   = (1 << 1),
};

struct GameState
{
// game states
	GlobalGameState global_state = GlobalGameState::NONE;
	uint64_t        current_state = (uint64_t)InGameState::NONE;
// world data
	cl_world world;

};

