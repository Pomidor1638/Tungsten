#pragma once
#include <cstdint>
#include "../world/world.h"
#include "../../../common/formats/protocol/protocol.h"
#include "protocol/protocol.h"

namespace tungsten::client 
{
    enum class MainStage
    {
        none = 0,

        boot,
        main_menu,
        in_game,
        shutdown,
    };

    struct ClientState
    {
        MainStage main_stage = MainStage::none;

        // world data
        world::cl_world world{};
    };
}
