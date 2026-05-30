#pragma once
#include <cstdint>
#include "../world/world.h"
#include "../../../common/formats/protocol/protocol.h"
#include "protocol/protocol.h"

namespace tungsten::client 
{
    enum class SessionStage
    {
        none = 0,

        disconnected,
        starting_local_server,
        connecting,
        loading,
        in_game,
        disconnecting,
        error
    };

    enum class MainStage
    {
        none = 0,

        boot,
        main_menu,
        in_game,
        shutdown,
    };

    enum class UICommand
    {
        none = 0,

        singleplayer,
        
        // Multiplayer
        multiplayer,
        create_server,
        connect_to,

        // settings
        settings,
        fullscreen,
        windowed,
        disconnect,

        cancel,
    };

    struct ClientState
    {
        MainStage       main_stage      = MainStage   ::none;
        SessionStage    session_stage   = SessionStage::none;
        
        bool menu = true;
        
        uint32_t snapshot_entity_count = 0;
        tungsten::protocol::packet_sv_snapshot latest_snapshot{};

        // UI commands
        UICommand ui_cmd = UICommand::none;

        // world data
        world::cl_world world{};

        // Connection state
        protocol::client_fsm_event  cl_fsm_event     = protocol::client_fsm_event::none;
        protocol::client_state      connection_state = protocol::client_state    ::disconnected;
    };

}
