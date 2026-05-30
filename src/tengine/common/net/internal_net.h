
#pragma once
#include "net.h"

namespace tungsten::net 
{
    struct internal_connection
    {
        util::container::ring_queue<net_message, 8> client_to_server;
        util::container::ring_queue<net_message, 8> server_to_client;
    };

    class internal_client_net : public client_net
    {
    public:
        explicit internal_client_net(internal_connection& conn);

        bool init()     override;
        bool start()    override;
        void shutdown() override;

        bool send(const net_message& msg, bool reliable = false) override;
        bool recv(net_message& msg) override;

    private:
        internal_connection& connection;
        bool started = false;
    };

    class internal_server_net : public server_net
    {
    public:
        explicit internal_server_net(internal_connection& conn);

        bool init()     override;
        bool start()    override;
        void shutdown() override;

        bool send(uint32_t client_id, const net_message& msg, bool reliable = false) override;
        bool recv(net_message& msg) override;

    private:
        internal_connection& connection;
        bool running = false;
    };
}