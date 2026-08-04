//
// Created by UBER_USER on 22.12.2025.
//

#include "client.h"
#include "cl_local.h"

#include "../common/memory/memory.h"

#include "window/window.h"
#include "renderer/renderer.h"
#include "audio/audio.h"
#include "input/input.h"

#include "../common/net/net.h"
#include "../common/net/internal_net.h"

#include "state/state.h"



namespace tungsten::client
{

    // Modules
    window  ::Window    * window        = nullptr;
    audio   ::Audio     * audio         = nullptr;
    renderer::Renderer  * renderer      = nullptr;
    input   ::Input     * input         = nullptr;
    server  ::Server    * local_server  = nullptr;
    net     ::Network   * network       = nullptr;

    // State
    extern bool        running = false;
    extern ClientState client_state{};

    // Connection
    extern net::net_message         net_msg;
    extern net::internal_connection internal_connection{};
    extern net::internal_client_net internal_client_net{ internal_connection };
    extern net::internal_server_net internal_server_net{ internal_connection };

    // Timing
    uint64_t cur_time = 0;
    uint64_t last_time = 0;
    uint64_t delta_time = 0;
    double   frac_delta = 0.0;

    namespace
    {
        bool w_init()
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

            if (!window || !window->get_window())
                return false;

            return true;
        }

        bool r_init()
        {
            renderer = memory::permanent_zone.allocate<renderer::Renderer, 1>(&client_state, window->get_window());
            return renderer->init();
        }


        bool a_init()
        {
            audio = nullptr;
            return true;
        }

        bool i_init()
        {
            input = memory::permanent_zone.allocate<input::Input, 1>(window->get_window());
            input->init();
            return true;
        }

        bool sv_init()
        {
            local_server = memory::permanent_zone.allocate<server::Server, 1>();
            local_server->set_network(&internal_server_net);
            local_server->init();
            return true;
        }

        bool net_init()
        {
            network = &internal_client_net;

            if (!network->init())
                return false;

            internal_server_net.init();
            internal_server_net.start();

            return network->start();
        }

        bool cl_init()
        {
            window->set_relative_mode(false);
            input->showCursor(true);

            cur_time = SDL_GetPerformanceCounter();
            last_time = cur_time;
            delta_time = 0;
            frac_delta = 0.0;

            client_state.main_stage = MainStage::main_menu;

            running = true;

            return true;
        }




        void process_event(const SDL_Event& e)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            }
        }



        void process_events()
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



        void update_time()
        {
            cur_time = SDL_GetPerformanceCounter();
            delta_time = cur_time - last_time;
            last_time = cur_time;
            frac_delta = static_cast<double>(delta_time) / SDL_GetPerformanceFrequency();
        }



        void update_world()
        {
            if (client_state.main_stage == MainStage::in_game)
            {
                client_state.world.updateCurLeaf(renderer->getCamera().origin);
            }
        }


        void render()
        {
            renderer->render();
        }



        void quit()
        {}




        void sound()
        {}

    }

    bool init()
    {
        if (!w_init()) return false;
        if (!r_init()) return false;
        if (!a_init()) return false;
        if (!i_init()) return false;
        if (!sv_init()) return false;
        if (!net_init()) return false;
        if (!cl_init()) return false;

        return true;
    }


    bool frame()
    {
        update_time();

        process_events();
        process_input();
        process_net();


        update_world();

        render();
        sound();

        return true;
    }
}


