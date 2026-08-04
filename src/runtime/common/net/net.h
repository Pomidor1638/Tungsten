#pragma once

#include <cstddef>
#include <cstdint>
#include "../../../common/utils/container/ring_queue.h"


namespace tungsten::net
{
    constexpr size_t MAX_MESSAGE_SIZE = 1400;
    
    using peer_id = uint32_t;
    
    constexpr peer_id invalid_peer = UINT32_MAX;
    constexpr peer_id local_peer   = 0;

    struct net_message
    {
        peer_id  peer = invalid_peer;
        uint16_t size = 0;
        uint8_t  data[MAX_MESSAGE_SIZE];
    };

    class connection
    {
    public:
        
        virtual bool send(int size, const void* data);
        virtual bool recv(int& size, int max_size, void* data);

    };

    class network
    {
    public:
        virtual bool init()     = 0;
        virtual bool start()    = 0;
        virtual void shutdown() = 0;

        virtual bool send(const net_message& msg, bool reliable) = 0;
        virtual bool recv(net_message& msg) = 0;

        virtual ~network() = default;
    };

    class server_net
    {
    public:
        virtual bool init()     = 0;
        virtual bool start()    = 0;
        virtual void shutdown() = 0;

        virtual void disconnect(uint32_t client_id) {};

        virtual bool send(uint32_t client_id, const net_message& msg, bool reliable) = 0;
        virtual bool recv(net_message& msg) = 0;

        virtual ~server_net() = default;
    };
}
