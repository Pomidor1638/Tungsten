
#include "client.h"
#include "common/net/net.h"
#include "protocol/protocol.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

namespace tungsten::client 
{
    static bool copy_snapshot_from_message(const net::net_message& msg, protocol::packet_sv_snapshot& snapshot)
    {
        const auto* packet = reinterpret_cast<const protocol::packet*>(msg.data);

        if (packet->header.payload_size < sizeof(protocol::packet_sv_snapshot))
            return false;

        std::memcpy(&snapshot, packet->payload, sizeof(snapshot));
        return true;
    }

    void apply_snapshot_camera(renderer::Renderer* renderer, const protocol::packet_sv_snapshot& snapshot)
    {
        if (!renderer || snapshot.entity_count == 0)
            return;

        auto camera = renderer->getCamera();
        const auto& player = snapshot.entities[0];

        camera.origin.x = player.origin[0];
        camera.origin.y = player.origin[1];
        camera.origin.z = player.origin[2];

        renderer->setCamera(camera);
    }
    
    void Client::process_net_disconnected() // nothing to do
    {
    }

    void Client::process_conn_error()
    {
        auto* packet =
            reinterpret_cast<protocol::packet*>(net_msg.data);

        auto* conn_error =
            reinterpret_cast<protocol::packet_conn_error*>(packet->payload);

        const auto& reason = conn_error->reason;

        const int reason_size = std::min<uint16_t>(
            reason.size,
            protocol::MAX_PROTOCOL_REASON_SIZE
        );

        std::string reason_text(reason.data, reason.data + reason_size);

        printf("Connection rejected: %s\n", reason_text.c_str());

        using enum protocol::conn_error_code;
        switch (conn_error->code)
        {
        case bad_protocol_version: // it would never work, because validate_packet() rejects wrong version
            printf("Bad protocol version: current is %u\n", protocol::VERSION);
            break;
        case server_full:
            printf("Server is full\n");
            break;
        case banned:
            printf("Banned\n");
            break;
        case timeout:
            printf("Timeout\n");
            break;
        case files_mismatch:
            printf("Files mismatch\n");
            break;
        case internal_error:
            printf("Internal error\n");
            break;
        default:
            printf("Unexcepted error: %i\n", conn_error->code);
            break;
        }

        client_state.connection_state = protocol::client_state::disconnected;
    }

    void Client::process_conn_ack()
    {
        auto* packet =
            reinterpret_cast<protocol::packet*>(net_msg.data);

        auto* conn_ack =
            reinterpret_cast<protocol::packet_conn_ack*>(packet->payload);

        if (conn_ack->need_files_check)
        {
            client_state.connection_state = protocol::client_state::checking_files;
            return;
        }

        client_state.connection_state = protocol::client_state::loading_resources;
        process_net_loading_resources();
    }

    void Client::process_net_waiting_conn_ack()
    {        
        protocol::packet* packet = reinterpret_cast<protocol::packet*>(net_msg.data);
        using enum protocol::packet_type;
        switch (packet->header.type) 
        {
        case conn_error:
            process_conn_error();
            break;
        case conn_ack:
            process_conn_ack();
            break;
        default:
            break;
        }
    }
	
    void Client::process_net_checking_files()
    {     
    }
	
    void Client::process_net_waiting_files_ack()
    {    
    }
	
    void Client::process_net_downloading_file()
    {        
    }
	
    void Client::process_net_waiting_file_fragment()
    {        
    }
	
    void Client::process_net_waiting_file_ack()
    {
    }
	
    void Client::process_net_loading_resources()
    {
        protocol::packet packet;
        protocol::packet_conn_ready conn_ready{};
        
        if (!packet_builder.build(packet, protocol::packet_type::conn_ready, &conn_ready, sizeof(conn_ready)))
            return;

        net::net_message msg;
        msg.size = sizeof(packet.header) + packet.header.payload_size;
        memcpy(msg.data, &packet, msg.size);
        network->send(msg, true);

        client_state.connection_state = protocol::client_state::ready;
    }
	
    void Client::process_net_first_sv_snapshot()
    {
        if (!copy_snapshot_from_message(net_msg, client_state.latest_snapshot))
            return;

        client_state.snapshot_entity_count = client_state.latest_snapshot.entity_count;
        apply_snapshot_camera(renderer, client_state.latest_snapshot);

        client_state.connection_state = protocol::client_state::in_game;
        client_state.session_stage = SessionStage::in_game;
        client_state.main_stage = MainStage::in_game;
        client_state.menu = false;

        window->setRelativeMode(true);
        input->showCursor(false);
    }

    void Client::process_net_ready()
    {
        protocol::packet* packet = reinterpret_cast<protocol::packet*>(net_msg.data);
        
        using enum protocol::packet_type;
        switch (packet->header.type) 
        {
        case sv_snapshot:
            process_net_first_sv_snapshot();
            break;

        // maybe here must be some error processing

        default:
            break;
        }
    }
	
    void Client::process_net_in_game()
    {
        protocol::packet* packet = reinterpret_cast<protocol::packet*>(net_msg.data);

        using enum protocol::packet_type;
        switch (packet->header.type)
        {
        case sv_snapshot:
            if (copy_snapshot_from_message(net_msg, client_state.latest_snapshot))
            {
                client_state.snapshot_entity_count = client_state.latest_snapshot.entity_count;
                apply_snapshot_camera(renderer, client_state.latest_snapshot);
            }
            break;
        default:
            break;
        }
    }
	
    void Client::process_net_disconnecting()
    {        
    }
	
    void Client::process_net_error()
    {        
    }

    
    void Client::process_net_send_conn_req()
    {
        protocol::packet packet;
        net::net_message msg;

        protocol::packet_conn_req conn_req = packet_builder.build_conn_req();

        if (!packet_builder.build(packet, protocol::packet_type::conn_req, &conn_req, sizeof(conn_req)))
        {
            client_state.connection_state = protocol::client_state::error;
            return;
        }

        msg.size = static_cast<uint16_t>(packet.header.header_size + packet.header.payload_size);
        memcpy(msg.data, &packet, msg.size);

        if (!network->send(msg, true))
        {
            client_state.connection_state = protocol::client_state::error;
            return;
        }

        client_state.connection_state = protocol::client_state::waiting_conn_ack;
    }

    void Client::process_net_send_packets()
    {
        
        using enum protocol::client_fsm_event;
        switch (client_state.cl_fsm_event)
        {
            case start_connect:
                process_net_send_conn_req();
                break;
            case cancel_connection:
                break;
            case resources_loaded:
                break;
            case start_disconnect:
                break;
            case timeout:
                break;
            default:
                break;
        }

        client_state.cl_fsm_event = protocol::client_fsm_event::none;
    }
    
    void Client::process_net_incoming_packet()
    {
        if (net_msg.size < sizeof(protocol::packet_header))
                return;

            protocol::packet* packet = reinterpret_cast<protocol::packet*>(net_msg.data);

            if (!protocol::packet_builder::validate_packet(*packet))
                return;

            if (net_msg.size < packet->header.header_size + packet->header.payload_size)
                return;

            using enum protocol::client_state;
            switch (client_state.connection_state)
            {
            //case disconnected:
            //    process_net_disconnected();
            //    break;
            case waiting_conn_ack:
                process_net_waiting_conn_ack();
                break;
            case downloading_file:
                process_net_downloading_file();
                break;
            case waiting_file_fragment:
                process_net_waiting_file_fragment();
                break;
            case waiting_file_ack:
                process_net_waiting_file_ack();
                break;
            case loading_resources:
                process_net_loading_resources();
                break;
            case ready:
                process_net_ready();
                break;
            case in_game:
                process_net_in_game();
                break;
            case disconnecting:
                process_net_disconnecting();
                break;
            //case error:
            //    process_net_error();
            //    break;
            default:
                break;
            }
    }

    void Client::process_net()
    {
        static uint64_t tick = 0;

        packet_builder.set_tick(tick++);
        packet_builder.set_timestamp(cur_time);

        while (network->recv(net_msg))
        {
            process_net_incoming_packet();
        }

        process_net_send_packets();
    }
}
