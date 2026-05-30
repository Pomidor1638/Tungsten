
#include "server.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace tungsten::server
{

    
    Server::Server()
    {}
    Server::~Server()
    {
        shutdown();
    }

    void Server::shutdown()
    {
        active = false;
    }

    bool Server::set_network(net::server_net* net)
    {
        if (active || !net)
            return false;
        
        network = net;

        return true;
    }

    
    bool Server::time_init()
    {
        server_time  = 0;
        server_delta = 0;
        frac_delta   = 0.0;
        
        server_tick = 0;
    
        return true;
    }
    
    bool Server::net_init()
    {
        if (!network)
            return false;

        return network->init();
    }

    void Server::entities_reset()
    {
        entities_count      = 0;
        for (auto& e : entities)
            e.id = -1;
    }

    void Server::players_reset()
    {        
        max_players_count   = 0;
        players_count       = 0;
        free_player_ptr     = 0;

        for (auto& p : players)
            p.active = false;
    }
    
    
    bool Server::sv_init()
    {
        entities_reset();
        players_reset();

        active = false;
        return true;
    }

    bool Server::init()
    {
        if (!time_init()) return false;    
        if (!net_init()) return false;    
        if (!sv_init()) return false;    

        return true;
    }

    void Server::start(const Config& cfg)
    {
        entities_reset();
        players_reset();

        active = true;
    }

    bool Server::is_active() const
    {
        return active;
    }

    
    void Server::think_entity(sv_entity& entity)
    {   
    }

    void Server::tick(uint64_t dt)
    {
        server_time += dt;
        server_tick++;

        for (uint32_t i = 0; i < entities_count; ++i)
        {
            think_entity(entities[i]);
        }
        
        process_net();
    }
}
