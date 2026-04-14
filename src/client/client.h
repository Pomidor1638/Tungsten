//
// Created by UBER_USER on 22.12.2025.
//

#ifndef MEGAGAME_CLIENT_H
#define MEGAGAME_CLIENT_H

#include "../common/common.h"

#include "window/window.h"
#include "audio/audio.h"
#include "input/input.h"
#include "renderer/renderer.h"
#include "../server/server.h"

#include "state/state.h"

class Client
{
public:

    int exec();

private:
    // Modules
    Window* window = nullptr;
    Audio* audio = nullptr;
    Renderer* renderer = nullptr;
    Input* input = nullptr;
    Server* local_server = nullptr;
    // Timing
    uint64_t cur_time = 0;
    uint64_t last_time = 0;
    uint64_t delta_time = 0;
    double   frac_delta = 0;
    // GameState
    bool debug = false;
    int  client_id = -1;

    // Main State
    bool running = false;

    void updateTime();
    void processEvents();
    void processEvent(const SDL_Event& e);

    void processInput();
    void processNet();

    void render();

    void loadMapFromDisk(const std::string& path);

    glm::vec3 make_wishdir(glm::vec3 angles);
    void update_angles(glm::vec3& angles);

    GameState game_state{};
    
public:

    Client(int argc, char* argv[]);
    virtual ~Client();

    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
};

#endif 