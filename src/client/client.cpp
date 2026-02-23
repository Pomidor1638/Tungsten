//
// Created by UBER_USER on 22.12.2025.
//

#include "client.h"

Client::Client(int argc, char* argv[])
{
    window = hunk.allocate<Window, 1>("MegaGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window || !window->getWindow())
        throw std::runtime_error(std::string(__FUNCSIG__) + "Failed to create SDL Window");

    renderer = hunk.allocate<Render, 1>(window->getWindow());
    input = hunk.allocate<Input, 1>(window->getWindow());
    audio = nullptr;
    inner_server = hunk.allocate<Server, 1>();

    running = true;

    cur_time = SDL_GetTicks64();
    last_time = cur_time;

    delta_time = 0;
    frac_delta = 0.0;

    window->setRelativeMode(true);
    renderer->setLoadMapCallback([this]() {

        std::string path = choose_file();

        this->loadMapFromDisk(path); 
    });
}

void Client::loadMapFromDisk(const std::string& path) 
{
    BSPMap map;
    auto bspfile_block = load_file(path);
    map.parse(bspfile_block);
    renderer->loadBSPMap(map);
}


Client::~Client()
{

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
    SDL_Event e;

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

    frac_delta = (double)delta_time / SDL_GetPerformanceFrequency();
}


void Client::render()
{
    renderer->update(frac_delta);
    renderer->renderStart();
    renderer->renderWorld();

    if (debug)
    {
        renderer->renderUI();
    }
    renderer->renderEnd();
}


void Client::processNet()
{
}

void Client::processInput()
{

    if (input->isKeyJustPressed(SDL_SCANCODE_LSHIFT))
    {
        window->setRelativeMode(debug);
        debug = !debug;
    }

    if (debug)
        return;

    auto angles = renderer->getCamAngles();
    auto origin = renderer->getCamOrigin();

    

    const float yaw_rad = glm::radians(angles.z);
    float cos_a = cos(yaw_rad);
    float sin_a = sin(yaw_rad);

    glm::vec3 forward = glm::vec3(-sin_a, cos_a, 0.0f);
    glm::vec3 right = glm::vec3(cos_a, sin_a, 0.0f);

    glm::vec3 move_dir(0.0f);

    if (input->isKeyPressed(SDL_SCANCODE_W)) move_dir += forward;
    if (input->isKeyPressed(SDL_SCANCODE_S)) move_dir -= forward;
    if (input->isKeyPressed(SDL_SCANCODE_D)) move_dir += right;
    if (input->isKeyPressed(SDL_SCANCODE_A)) move_dir -= right;

    float speed = this->speed * frac_delta;

    if (glm::length(move_dir) > 0.001f)
        origin += glm::normalize(move_dir) * speed;

    if (input->isKeyPressed(SDL_SCANCODE_SPACE)) origin.z += speed;
    if (input->isKeyPressed(SDL_SCANCODE_LCTRL)) origin.z -= speed;

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

    angles.z -= x * sensitivity;
    angles.x -= y * sensitivity;

    if (angles.x > 180.0f) 
        angles.x = 180.0f;
    
    if (angles.x < 0.0f)   
        angles.x = 0.0f;

    if (angles.z > 360.0f)
    {
        angles.z = fmodf(angles.z, 360.0f);
    }
    if (angles.z < 0.0f)
    {
        angles.z = fmodf(angles.z, 360.0f) + 360.0f;
    }

    renderer->setCamOrigin(origin);
    renderer->setCamAngles(angles);
}


int Client::exec()
{
    while (running)
    {
        updateTime();
        processEvents();

        processInput();

        processNet();


        render();
    }
    return EXIT_SUCCESS;
}
