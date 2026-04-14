//
// Created by UBER_USER on 22.12.2025.
//

#include "client.h"
#include "../common/bspfile/bspfile.h"

#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

Client::Client(int argc, char* argv[])
{
    window = hunk.allocate<Window, 1>
        (
            "MegaGame",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1024,
            768,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

    if (!window || !window->getWindow())
    {
        throw std::runtime_error(std::string(__FUNCSIG__) + "Failed to create SDL Window");
    }

    renderer = hunk.allocate<Renderer, 1>(&game_state, window->getWindow());
    renderer->init();

    input = hunk.allocate<Input, 1>(window->getWindow());

    audio = nullptr;
    local_server = hunk.allocate<Server, 1>();

    window->setRelativeMode(true);

    cur_time = SDL_GetPerformanceCounter();
    last_time = cur_time;
    delta_time = 0;
    frac_delta = 0.0;

    running = true;
}

Client::~Client()
{
}

int Client::exec()
{
    while (running)
    {
        updateTime();

        processInput();
        processEvents();


        if (game_state.global_state == GlobalGameState::IN_GAME)
            game_state.world.updateCurLeaf(renderer->getCamera().origin);

        processNet();
        render();
    }

    return EXIT_SUCCESS;
}

void Client::loadMapFromDisk(const std::string& path)
{
    auto bspfile_block = load_file(path);
    BSPMap bspmap;
    bspmap.parse(bspfile_block);
    
    game_state.world.parseBSPMap(bspmap);
    renderer->loadBSPMap(bspmap);

}

void Client::processEvent(const SDL_Event& e)
{
    if (input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        running = false;
        return;
    }

    switch (e.type)
    {
    case SDL_QUIT:
        running = false;
        break;
    }
}

void Client::processEvents()
{
    static SDL_Event e;

    input->startProcessEvent();
    renderer->startProcessEvent();

    while (SDL_PollEvent(&e))
    {
        processEvent(e);
        input->processEvent(e);
        renderer->processEvent(e);
    }

    input->endProcessEvent();
    renderer->endProcessEvent();
}

void Client::updateTime()
{
    cur_time = SDL_GetPerformanceCounter();
    delta_time = cur_time - last_time;
    last_time = cur_time;

    frac_delta = static_cast<double>(delta_time) / SDL_GetPerformanceFrequency();
}

void Client::processNet()
{
}

void Client::render()
{
    renderer->render();
}



void Client::update_angles(glm::vec3& angles)
{
    int x, y;
    int hw = 0, hh = 0;

    input->getMousePos(x, y);
    window->getSize(hw, hh);

    hw >>= 1;
    hh >>= 1;

    input->setMousePos(hw, hh);

    x -= hw;
    y -= hh;

    const float sensitivity = 0.15f;

    angles.z -= glm::radians(x * sensitivity);
    angles.x -= glm::radians(y * sensitivity);

    if (angles.x > M_PI)
    {
        angles.x = M_PI;
    }
    if (angles.x < 0.0f)
    {
        angles.x = 0.0f;
    }
    if (angles.z > 2 * M_PI)
    {
        angles.z = fmodf(angles.z, 2*M_PI);
    }
    if (angles.z < 0.0f)
    {
        angles.z = fmodf(angles.z, 2 * M_PI) + 2 * M_PI;
    }
}

glm::vec3 Client::make_wishdir(glm::vec3 angles)
{

    

    glm::mat3x3 base = glm::rotate(glm::identity<glm::mat4x4>(), angles.z, { 0.0f, 0.0f, 1.0f });

    glm::vec3 wishdir{ 0 };

    if (input->isKeyPressed(SDL_SCANCODE_D))
    {
        wishdir[0] += 1;
    }
    if (input->isKeyPressed(SDL_SCANCODE_A))
    {
        wishdir[0] -= 1;
    }

    if (input->isKeyPressed(SDL_SCANCODE_W))
    {
        wishdir[1] += 1;
    }
    if (input->isKeyPressed(SDL_SCANCODE_S))
    {
        wishdir[1] -= 1;
    }


    if (input->isKeyPressed(SDL_SCANCODE_SPACE))
    {
        wishdir[2] += 1;
    }
    if (input->isKeyPressed(SDL_SCANCODE_LSHIFT))
    {
        wishdir[2] -= 1;
    }


    return base * wishdir;
}

void Client::processInput()
{
    using enum GlobalGameState;
    switch (game_state.global_state)
    {
    case IN_GAME:
    {
        auto camera = renderer->getCamera();

        update_angles(camera.angles);
        camera.origin += (float)(100 * frac_delta) * make_wishdir(camera.angles);
        renderer->setCamera(camera);
    }
    break;
    case NONE:
    {
        if (input->isKeyPressed(SDL_SCANCODE_RETURN))
        {
            //game_state.current_state |= (uint64_t)InGameState::SHOW_MENU;
            auto path = choose_file();
            loadMapFromDisk(path);
            game_state.global_state = IN_GAME;
        }
        /*
        if (input->isKeyJustPressed(SDL_SCANCODE_ESCAPE))
        {

        }
        */
    }
    break;
    default:
        break;
    }
}