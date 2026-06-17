//
// Created by UBER_USER on 22.12.2025.
//

#pragma once

#include <SDL2/SDL.h>
#include <string>


#include "window/window.h"
#include "audio/audio.h"
#include "input/input.h"

#include "renderer/renderer.h"
#include "state/state.h"

#include "protocol/protocol.h"
#include "../common/net/net.h"

#include "../common/net/internal_net.h"

#include "../server/server.h"


namespace tungsten::client
{
    class Client
    {
    public:
        Client(int argc, char* argv[]);
        ~Client();

        int exec();

        Client()              = delete;
        Client(const Client&) = delete;
        Client(Client&&)      = delete;

    private:

        // Modules
        window  ::Window    * window       = nullptr;
        audio   ::Audio     * audio        = nullptr;
        renderer::Renderer  * renderer     = nullptr;
        input   ::Input     * input        = nullptr;
        server  ::Server    * local_server = nullptr;
        net     ::client_net* network      = nullptr;

        // Client fsm

        // State
        bool running = false;

        // Timing
        uint64_t cur_time   = 0;
        uint64_t last_time  = 0;
        uint64_t delta_time = 0;
        double   frac_delta = 0.0;

        // Temporary client-side game state.
        // Later split into ClientWorld / SnapshotState / RenderScene.
        ClientState client_state{};

        // Connection
        net     ::net_message         net_msg;
        net     ::internal_connection internal_connection{};
        net     ::internal_client_net internal_client_net { internal_connection };
        net     ::internal_server_net internal_server_net { internal_connection };

    private:

        // ---------- //
        // Initialize //
        // ---------- //

        bool init();
        bool w_init();
        bool a_init();
        bool r_init();
        bool i_init();
        bool sv_init();
        bool net_init();
        bool cl_init();

        // ----------------- //
        // Processing Events //
        // ----------------- //
        
        // cl_event.cpp
        void process_events();
        void process_event(const SDL_Event& e); 
        void update_time();

        // ---------------- //
        // Processing Input //
        // ---------------- //
        
        // cl_input.cpp
        void process_input();

        void process_input_main_menu();
        void process_input_in_game();

        glm::vec2   make_wishdir();
        void        update_angles(glm::vec3& angles);

        // ------------------ //
        // Processing Network //
        // ------------------ //

        // cl_net.cpp

        void process_net();

        void process_net_incoming_packet();
        
        // incoming

        void process_net_disconnected();        
		void process_net_waiting_conn_ack();
        
        void process_conn_error();
        void process_conn_ack();

		void process_net_checking_files();
		void process_net_waiting_files_ack();
		void process_net_downloading_file();
		void process_net_waiting_file_fragment();
		void process_net_waiting_file_ack();
		void process_net_loading_resources();
		
        void process_net_ready();
        void process_net_first_sv_snapshot();

		void process_net_in_game();
		void process_net_disconnecting();
		void process_net_error();

        // outgoing
        void process_net_send_packets();

        void process_net_send_conn_req();

        void update_world();

        void render();
        void sound();

        void start_single(const std::string& map_path);
        void disconnect();

        void start_local_server(const std::string& map_path);
    };
}
