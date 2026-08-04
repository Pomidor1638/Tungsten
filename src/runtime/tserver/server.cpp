
#include "server.h"

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
    
    int Server::register_player()
    {
        return -1;
    }

    void Server::remove_player (int player_id)
    {
    }
    
    void Server::process_usercmd(int player_id, protocol::cl_usercmd cmd)
    {
    }
    
    
    bool Server::sv_init()
    {
        entities.clear();
        players.clear();

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
        entities.clear();
        players.clear();

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

        for (auto& e : entities)
        {
            think_entity(e);
        }
        
        process_net();
    }
}
