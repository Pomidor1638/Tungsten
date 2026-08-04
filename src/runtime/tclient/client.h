//
// Created by UBER_USER on 22.12.2025.
//

#pragma once
#include <cstdint>

namespace tungsten::client
{
    namespace window { extern class Window; }
    extern window::Window* window;
    
    namespace audio { extern class Audio; }
    extern audio::Audio* audio;
    
    namespace renderer { extern class Renderer; }
    extern renderer::Renderer* renderer;

    namespace input { extern class Input; }
    extern input::Input* input;

    namespace server { extern class Server; }
    extern server::Server* local_server;

    namespace net { extern class Network; }
    extern net::Network* network;

    bool init ();
    bool frame(uint64_t delta_us);
    void quit ();
}
