
#include "common/net/net.h"
#include "protocol/protocol.h"
#include "server.h"
#include <cstring>

namespace tungsten::server 
{

    void Server::process_net_incoming_conn_req()
    {
        auto* p = reinterpret_cast<protocol::packet*>(net_msg.data);
        auto* cn_req = reinterpret_cast<protocol::packet_conn_req*>(p);

        // protocol::validate shouldn't pass wrong version. Why it thing here? 
        //if (cn_req->client_protocol_version) 

        protocol::fixed_string<MAX_PLAYERNAME_SIZE> player_name{.data = "player1"};
        player_name.size = sizeof("player1");

        // int client_id = register_player(player_name);
        
        protocol::packet_conn_ack cn_ack = packet_builder.build_conn_ack();

        cn_ack.need_files_check = false;
        cn_ack.server_nonce     = cn_req->client_nonce;
        cn_ack.server_tick      = server_tick;
        cn_ack.server_tickrate  = (60 * (1'000'000));

        protocol::packet packet;
        packet_builder.build(packet, protocol::packet_type::conn_ack, &cn_ack, sizeof(cn_ack));

        net::net_message snd_msg;
        snd_msg.size = packet.header.header_size + packet.header.payload_size;
        memcpy(snd_msg.data, &packet, snd_msg.size);

        if (!network->send(net_msg.peer, snd_msg, true))
        {
            printf("sv_send Error\n");
        }
    }

    void Server::process_net_incoming_conn_cancel()
    {}
    void Server::process_net_incoming_conn_ready()
    {}
    void Server::process_net_incoming_conn_req_files()
    {}
    void Server::process_net_incoming_conn_ack_file()
    {}
    void Server::process_net_incoming_cl_snapshot()
    {}
    void Server::process_net_incoming_cl_event()
    {}
    void Server::process_net_incoming_cl_req()
    {}
    void Server::process_net_incoming_sv_ack()
    {}
    void Server::process_net_incoming_disconnect_req()
    {}
    void Server::process_net_incoming_disconnect_ack()
    {}
    void Server::process_net_incoming_status_req()
    {}

    void Server::process_net_incoming()
    {
        
        if (net_msg.size < protocol::PACKET_HEADER_SIZE)
        {
            return;
        }

        auto p = reinterpret_cast<protocol::packet*>(net_msg.data);
        
        if (!protocol::packet_builder::validate_packet(*p))
        {
            return;
        }

        using enum protocol::packet_type;
        switch (p->header.type) 
        {
        case conn_req:
            process_net_incoming_conn_req();
            break;
		case conn_cancel:
            process_net_incoming_conn_cancel();
            break;
		case conn_ready:
            process_net_incoming_conn_ready();
            break;
		case conn_req_files:
            process_net_incoming_conn_req_files();
            break;
		case conn_ack_file:
            process_net_incoming_conn_ack_file();
            break;
        case cl_snapshot:
            process_net_incoming_cl_snapshot();
            break;
		case cl_event:
            process_net_incoming_cl_event();
            break;
		case cl_req:
            process_net_incoming_cl_req();
            break;
		case sv_ack:
            process_net_incoming_sv_ack();
            break;
        case disconnect_req:
            process_net_incoming_disconnect_req();
            break;
		case disconnect_ack:
            process_net_incoming_disconnect_ack();
            break;
		case status_req:
            process_net_incoming_status_req();
            break;
        default:
            break;
        }
    }
    void Server::process_net_outgoing()
    {}

    void Server::process_net()
    {
        while (network->recv(net_msg))
        {
            process_net_incoming();
        }
        process_net_outgoing();
    }

}