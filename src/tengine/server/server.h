#pragma once

#include "../../common/formats/protocol/protocol.h"
#include "../common/net/net.h"
#include "protocol/protocol.h"
#include <glm/glm.hpp>
#include <bitset>

namespace tungsten::server
{

    constexpr int MAX_SERVER_ENTITIES = 64;
    constexpr int MAX_SERVER_PLAYERS  = 16;
    constexpr int MAX_PLAYERNAME_SIZE = 32;

    enum class EntityClassId
    {
        None,
        Player,
        DebugRotator,
    };

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
        // Networking   //
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

        struct sv_entity
        {
            int id = 0;
            bool active = false;
            EntityClassId           class_id        = EntityClassId::None;
            protocol::geometry_type geometry_type   = protocol::geometry_type::none;

            glm::vec3 origin  { 0.0f };
            glm::vec3 angles  { 0.0f };
            glm::vec3 velocity{ 0.0f };
        };

        int         entities_count = 0;
        sv_entity   entities[MAX_SERVER_ENTITIES]{};

        void        entities_reset();
        sv_entity*  spawn_entity();
        void        think_entity(sv_entity& entity);
        
        //------------//
        // Players    //
        //------------//

        struct sv_player
        {
            bool    active = false;
            int     ent_id  = -1; // entities[ent_id]
            int     peer_id = -1;
            protocol::usercmd cmd{};    
            protocol::server_client_stage stage = protocol::server_client_stage::disconnected;
        };

        int         max_players_count   = 0;
        int         players_count       = 0;
        int         free_player_ptr     = 0;
        sv_player   players[MAX_SERVER_PLAYERS]{};

        void        players_reset();
        int         register_player(protocol::fixed_string<MAX_PLAYERNAME_SIZE> player_name);
        void        release_player (int player_id);
        void        process_usercmd(int player_id, protocol::usercmd cmd);
        
        
        protocol::packet_sv_snapshot makeSnapshot() const;
    };
}
