//
// Created by UBER_USER on 22.12.2025.
//

#include "client.h"
#include "protocol/protocol.h"
#include "state/state.h"
#include "common/memory/memory.h"
#include "utils/system/system.h"
#include <SDL_main.h>
#include <float.h>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/ext.hpp>


namespace tungsten::client
{
    bool Client::w_init()
    {

        window = memory::permanent_zone.allocate<window::Window, 1>
            (
                "Tungsten",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                1024,
                768,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
            );

        if (!window || !window->getWindow())        
            return false;

        return true;
    }
        
    bool Client::r_init()
    {
        renderer = memory::permanent_zone.allocate<renderer::Renderer, 1>(&client_state, window->getWindow());
        return renderer->init();
    }

    bool Client::a_init()
    {
        audio = nullptr;
        return true;
    }
        
    bool Client::i_init()
    {
        input = memory::permanent_zone.allocate<input::Input, 1>(window->getWindow());
        input->init();
        return true;
    }
        
    bool Client::sv_init()
    {
        local_server = memory::permanent_zone.allocate<server::Server, 1>();
        local_server->set_network(&internal_server_net);
        local_server->init();
        return true;
    }
            
    bool Client::net_init()
    {
        network = &internal_client_net;

        if (!network->init())
            return false;

        internal_server_net.init();
        internal_server_net.start();

        return network->start();
    }
        
    bool Client::cl_init()
    {
        window->setRelativeMode(false);
        input->showCursor(true);

        cur_time = SDL_GetPerformanceCounter();
        last_time = cur_time;
        delta_time = 0;
        frac_delta = 0.0;

        client_state.main_stage = MainStage::main_menu;
        client_state.session_stage = SessionStage::disconnected;
        
        running = true;
        
        return true;
    }

    bool Client::init()
    {
        if (!  w_init()) return false;
        if (!  r_init()) return false;
        if (!  a_init()) return false;
        if (!  i_init()) return false;
        if (! sv_init()) return false;
        if (!net_init()) return false;
        if (! cl_init()) return false;

        return true;
    }


    Client::Client(int argc, char* argv[])
    {
        if (!init())
        {
            throw std::runtime_error("Client init failed");
        }
    }

    Client::~Client()
    {
    }
    
    void Client::disconnect()
    {
        client_state.session_stage = SessionStage::disconnecting;
        client_state.connection_state = protocol::client_state::disconnected;
        client_state.cl_fsm_event = protocol::client_fsm_event::none;
        client_state.main_stage = MainStage::main_menu;
        client_state.session_stage = SessionStage::disconnected;
        client_state.menu = true;
        window->setRelativeMode(false);
        input->showCursor(true);
    }

    void Client::start_single(const std::string& map_path)
    {
        client_state.main_stage         = MainStage::main_menu;
        client_state.session_stage      = SessionStage::starting_local_server;
        client_state.connection_state   = protocol::client_state::disconnected;
        client_state.cl_fsm_event       = protocol::client_fsm_event::none;
        client_state.menu               = false;

        server::Server::Config cfg;
        auto& s = cfg.bspfilename;
        s.size = map_path.length();
        strncpy(s.data, map_path.c_str(), s.size);
        local_server->start(cfg);

        client_state.session_stage      = SessionStage::connecting;
        client_state.cl_fsm_event       = protocol::client_fsm_event::start_connect;
    }

    void Client::process_event(const SDL_Event& e)
    {
        switch (e.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        }
    }

    void Client::process_events()
    {
        static SDL_Event e;

        input->startProcessEvent();
        renderer->startProcessEvent();

        while (SDL_PollEvent(&e))
        {
            process_event(e);
            input->processEvent(e);
            renderer->processEvent(e);
        }

        input->endProcessEvent();
        renderer->endProcessEvent();
    }

    void Client::update_time()
    {
        cur_time    = SDL_GetPerformanceCounter();
        delta_time  = cur_time - last_time;
        last_time   = cur_time;
        frac_delta  = static_cast<double>(delta_time) / SDL_GetPerformanceFrequency();
    }

    
    void Client::update_world()
    {
        if (client_state.main_stage == MainStage::in_game)
        {
            client_state.world.updateCurLeaf(renderer->getCamera().origin);
        }
    }

    void Client::render()
    {
        client_state.ui_cmd = UICommand::none;
        renderer->render();
    }

    void Client::sound()
    {
    }

    int Client::exec()
    {
        while (running)
        {
            update_time();

            process_events();
            process_input();
            process_net();

            if (local_server->is_active())
                local_server->tick(delta_time);

            update_world();

            render();
            sound();
        }

        return EXIT_SUCCESS;
    }
}


