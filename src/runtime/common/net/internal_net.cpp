#include "internal_net.h"
#include "common/net/net.h"

namespace tungsten::net
{
    internal_client_net::internal_client_net(internal_connection& conn)
        : connection{ conn }
    {
    }

    bool internal_client_net::init()
    {
        started = false;
        return true;
    }

    bool internal_client_net::start()
    {
        started = true;
        return true;
    }

    void internal_client_net::shutdown()
    {
        started = false;
    }

    bool internal_client_net::send(const net_message& msg, bool reliable)
    {
        (void)reliable;

        if (!started)
            return false;

        net_message copy = msg;
        copy.peer = local_peer;

        return connection.client_to_server.push(copy);
    }

    bool internal_client_net::recv(net_message& msg)
    {
        if (!started)
            return false;

        return connection.server_to_client.pop(msg);
    }

    internal_server_net::internal_server_net(internal_connection& conn)
        : connection{ conn }
    {
    }

    bool internal_server_net::init()
    {
        connection.client_to_server.clear();
        connection.server_to_client.clear();

        running = false;
        return true;
    }

    bool internal_server_net::start()
    {
        running = true;
        return true;
    }

    void internal_server_net::shutdown()
    {
        running = false;
        connection.client_to_server.clear();
        connection.server_to_client.clear();
    }

    bool internal_server_net::send(uint32_t client_id, const net_message& msg, bool reliable)
    {
        (void)reliable;

        if (!running)
            return false;

        net_message copy = msg;
        copy.peer = local_peer;

        return connection.server_to_client.push(copy);
    }

    bool internal_server_net::recv(net_message& msg)
    {
        if (!running)
            return false;

        return connection.client_to_server.pop(msg);
    }
}
