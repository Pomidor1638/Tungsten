#pragma once

#include "../../common/formats/protocol/protocol.h"
#include "../common/net/net.h"
#include "protocol/protocol.h"
#include "../../common/utils/container/fixed_pool.h"
#include <glm/glm.hpp>

namespace tungsten::server
{

    constexpr int MAX_SERVER_ENTITIES = 64;
    constexpr int MAX_SERVER_PLAYERS  = 16;
    constexpr int MAX_PLAYERNAME_SIZE = 32;

    class Server
    {
    public:

        Server();
        virtual ~Server();

        struct Config 
        {
            protocol::fixed_string<1024> bspfilename;
            int max_players;
        };

        bool set_network(net::server_net* net);
        bool init();

        void start(const Config& cfg);
        void shutdown();

        void tick(uint64_t dt);
        bool is_active() const;

    private:

        //--------------//
        // Init         //
        //--------------//

        bool time_init();
        bool  net_init();
        bool   sv_init();

        bool active = false;

        
        //--------------//
        // Network      //
        //--------------//

        protocol::packet_builder    packet_builder  = {};
        net::net_message            net_msg         = {};
        net::server_net*            network         = nullptr;
        
        void process_net();
        void process_net_incoming();
        void process_net_incoming_conn_req();
        void process_net_incoming_conn_cancel();
        void process_net_incoming_conn_ready();
        void process_net_incoming_conn_req_files();
        void process_net_incoming_conn_ack_file();
        void process_net_incoming_cl_snapshot();
        void process_net_incoming_cl_event();
        void process_net_incoming_cl_req();
        void process_net_incoming_sv_ack();
        void process_net_incoming_disconnect_req();
        void process_net_incoming_disconnect_ack();
        void process_net_incoming_status_req();
        
        void process_net_outgoing();

        //----------//
        // Timing   //
        //----------//

        uint64_t server_time    = 0;
        uint64_t server_delta   = 0;
        double   frac_delta     = 0.0;        
        uint64_t server_tick    = 0;

        //------------//
        // Entities   //
        //------------//

        template<class T, size_t pool_capacity>
        using fx_pool = util::container::fixed_pool<T, pool_capacity>;

        struct sv_entity
        {
            static constexpr int invalid_entity = -1;
            static constexpr int invalid_class_id = -1;

            int id          = invalid_entity;
            int class_id    = invalid_class_id;

            glm::vec3 origin  { 0.0f };
            glm::vec3 angles  { 0.0f };
            glm::vec3 velocity{ 0.0f };
        };

        int max_entities_count = 0;
        fx_pool<sv_entity, MAX_SERVER_ENTITIES> entities{};

        sv_entity*  spawn_entity();
        void        think_entity(sv_entity& entity);
        
        //------------//
        // Players    //
        //------------//

        struct sv_player_connection_state
        {
            protocol::server_client_stage stage = protocol::server_client_stage::disconnected;
        };

        struct sv_player_input_state
        {
            protocol::cl_usercmd cmd{};    
            protocol::console_command console_cmd{};
        };

        struct sv_player
        {
            static constexpr int no_player = -1;

            int     ent_id  = sv_entity::invalid_entity; // entities[ent_id]
            int     peer_id = net      ::invalid_peer;

            sv_player_connection_state connection_state{};
        };

        int         max_players_count   = 0;
        fx_pool<sv_player, MAX_SERVER_PLAYERS> players;

        int         register_player();
        void        remove_player(int player_id);

        void        process_usercmd(int player_id, protocol::cl_usercmd cmd);        
        void        process_console_cmd(int player_id, protocol::console_command cmd);

        protocol::packet_sv_snapshot makeSnapshot() const;
    };
}
