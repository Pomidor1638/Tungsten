//
// Created by UBER_USER on 22.12.2025.
//

#ifndef MEGAGAME_CLIENT_H
#define MEGAGAME_CLIENT_H

#include "window/window.h"
#include "audio/audio.h"
#include "input/input.h"
#include "render/render.h"
#include "../server/server.h"

class Client
{
public:

    int exec();

private:

    // modules
    Server* inner_server = nullptr;
    Window* window       = nullptr;
    Audio * audio        = nullptr;
    Render* renderer     = nullptr;
    Input * input        = nullptr;

    // timing
    uint64_t   cur_time = 0;
    uint64_t  last_time = 0;
    uint64_t delta_time = 0;
    double   frac_delta = 0;

    // config
    bool debug = false;

    // main 
    bool running = false;

    void updateTime();
    void processEvents();
    void processEvent(const SDL_Event& e);

    void render();
    void processInput();
    void processNet();

    void loadMapFromDisk(const std::string& path);

    float speed = 100.0f;

public:

    Client(int argc, char* argv[]);
    virtual ~Client();

    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;

};

#endif 