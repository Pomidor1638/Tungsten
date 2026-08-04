//
// client_internal.h
//

#pragma once

#include <cstdint>

namespace tungsten::client
{
    //=========================================================================
    // Forward declarations
    //=========================================================================

    struct ClientState;

    namespace window
    {
        class Window;
    }

    namespace audio
    {
        class Audio;
    }

    namespace renderer
    {
        class Renderer;
    }

    namespace input
    {
        class Input;
    }

    namespace server
    {
        class Server;
    }

    namespace net
    {
        class Network;

        struct net_message;
        struct internal_connection;
        struct internal_client_net;
        struct internal_server_net;
    }

    
    // Global client state

    extern ClientState client_state;

    extern uint64_t cur_time;
    extern uint64_t delta_time;

    // cl_window.cpp

    extern window::Window* window;
    bool window_init();

    // cl_renderer.cpp
    
    extern renderer::Renderer* renderer;
    bool renderer_init();
    void render();

    // cl_audio.cpp
    
    extern audio::Audio* audio;
    bool audio_init();
    void sound();
    
    // cl_input.cpp

    extern input::Input* input;
    bool input_init();
    void process_input();

    // cl_server.cpp

    extern server::Server* local_server;
    bool server_init();

    // cl_net.cpp
    
    extern net::Network* network;

    extern net::net_message         net_msg;
    extern net::internal_connection internal_connection;
    extern net::internal_client_net internal_client_net;
    extern net::internal_server_net internal_server_net;

    bool network_init();

    void process_net();

    // cl_events.cpp
    void process_events();

    // client.cpp
    void update_time();
}