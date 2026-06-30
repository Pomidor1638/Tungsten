
#include "cl.h"
#include "protocol/packets.h"
#include "protocol/protocol.h"
#include <cstddef>
#include <cstring>

namespace tungsten::protocol 
{

    
	bool client_fsm::push_event(const event& e)
    {
        bool ok = events.push(e);
        assert(ok && "protocol event queue overflow");
        return ok;
    }

	bool client_fsm::push_event(event&& e)
    {
        bool ok = events.push(std::move(e));
        assert(ok && "protocol event queue overflow");
        return ok;
    }

    void client_fsm::reset_timeouts()
    {
        last_recv_timestamp_us = curr_timestamp_us;
		tries_count = retry_count;
    }

    void client_fsm::reset()
    {
		last_timestamp_us = 0;
		curr_timestamp_us = 0;
		delta_us		  = 0; 

        reset_timeouts();
        events.clear();
        main_stage = client_main_stage::disconnected;
    }

    client_fsm::client_fsm()
    {
        reset();
    }

	void client_fsm::check_timeouts()
    {
        if (main_stage == client_main_stage::disconnected)
            return;
        
        uint64_t delta = curr_timestamp_us - last_recv_timestamp_us;
        
        if (delta > retry_interval_us)
        {
            if (tries_count)
            {
                tries_count--;
            }
            else
            {
                error(event_error_type::protocol_error, protocol_error::timeout);
                main_stage = client_main_stage::disconnected;
            }
        }
    }

	void client_fsm::tick(uint64_t delta_time_us)
    {
        last_timestamp_us = curr_timestamp_us;
        curr_timestamp_us += delta_time_us;
        delta_us = delta_time_us; 
        check_timeouts();
    }
    
	void client_fsm::emit_connection_accepted(bool need_file_sync)
    {
        push_event(
            event{
                .type = event_type::connection_accepted,
                .conn_accepted = {
                    .need_file_sync = need_file_sync
                }
            }
        );
    }

	void client_fsm::emit_connection_rejected(reject_reason reason)
    {
        push_event(
            event{
                .type = event_type::connection_rejected,
                .conn_rejected = {.reason = reason }
            }
        );
    }

    void client_fsm::emit_send_error(protocol_error code)
    {

        packet_error error
        {
            .code = code
        };

        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::error, 
                0, 
                server_nonce,
                sizeof(error), 
                &error
            ),
            true
        );
    }

    
    void client_fsm::emit_send_disconnect_req(disconnect_type type, disconnect_reason reason)
    {
        packet_disconnect_req disconnect_req
        {
            .type = type,
            .reason = reason
        };

        disconnect_req.reason.size = to_protocol16(disconnect_req.reason.size);

        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::disconnect_req, 
                0, 
                server_nonce,
                sizeof(disconnect_req), 
                &disconnect_req
            ), 
            true
        );
    }

    void client_fsm::emit_send_disconnect_ack()
    {
        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::disconnect_ack, 
                0, 
                server_nonce,
                0, 
                nullptr
            ), 
            true
        );
    }
    
	void client_fsm::emit_error(event_error_type type, protocol_error protocol)
    {
        push_event(
            event{
                .type = event_type::error,
                .error = {
                    .type = type,
                    .protocol = protocol
                }
            }
        );
    }

    
	void client_fsm::error(event_error_type type, protocol_error protocol)
    {

        switch (main_stage) 
        {
        case client_main_stage::disconnected:
        case client_main_stage::none:
            break;
        default:
            emit_send_error(protocol);       
            break;
        }

        emit_error(type, protocol);
        main_stage = client_main_stage::disconnected;
    }
    
	void client_fsm::emit_send(const packet& p, bool reliable)
    {
        event_send send = {
            .reliable = reliable
        };

        send.size = p.header.header_size + p.header.payload_size;
        
        packet o = p;
        packet_header_to_protocol(o.header);

        memcpy(send.data, &o, send.size);

        push_event(event{
            .type = event_type::send,
            .send = send
        });
    }

    
	bool client_fsm::recv_conn_accept(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_conn_accept))
            return false;

        auto* conn_accept = reinterpret_cast<const packet_conn_accept*>(p.payload);

        server_nonce = to_native64(conn_accept->server_nonce);
        emit_connection_accepted(conn_accept->need_file_sync);

        main_stage = client_main_stage::active;
        
        /*
        main_stage    = client_main_stage   ::loading;
        loading_stage = client_loading_stage::file_sync;
        
        if (conn_accept->need_file_sync)
            file_sync_stage = client_file_sync_stage::waiting_file_manifest;
        else
            level_sync_stage = client_level_sync_stage::waiting_level_info;
        */
        
        return true;
    }

	bool client_fsm::recv_conn_reject(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_conn_reject))
            return false;

        auto* conn_reject = reinterpret_cast<const packet_conn_reject*>(p.payload);
        reject_reason reason = conn_reject->reason;
        
        reason.size = to_native16(reason.size);
        emit_connection_rejected(reason);

        main_stage = client_main_stage::disconnected;
        
        return true;
    }

    bool client_fsm::on_recv_connecting(const packet& p)
    {
        if (validate_all_packet(p, client_nonce))
        {
            switch (p.header.type) 
            {
            case packet_type::conn_accept:
                return recv_conn_accept(p);
            case packet_type::conn_reject:
                return recv_conn_reject(p);
            default:
                break;
            }
        }

        return false;
    }

    
	bool client_fsm::on_loading_file_sync(const packet& p)
    {
        return false;
    }

	bool client_fsm::on_loading_level_info(const packet& p)
    {
        return false;
    }

	bool client_fsm::on_loading_snapshot_sync(const packet& p)
    {
        return false;
    }


    
    bool client_fsm::on_recv_error(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_error))
            return false;
        // TODO: packet check
        auto* pkt_error = reinterpret_cast<const packet_error*>(p.payload);
        // because error sends packet_error
        // when RECEIVED error DO NOT send error too
        emit_error(event_error_type::protocol_error, pkt_error->code);
        main_stage = client_main_stage::disconnected;

        return true;
    }

	bool client_fsm::on_recv_loading(const packet& p)
    {
        using enum client_loading_stage;
        switch (loading_stage) 
        {
        case file_sync:
            return on_loading_file_sync(p);
            break;
        case level_sync:
            return on_loading_level_info(p);
		case snapshot_sync:
            return on_loading_snapshot_sync(p);
        default:
            break;
        }

        return false;
    }

    
    void client_fsm::emit_disconnected(disconnect_type type, disconnect_reason reason)
    {
        push_event(event{
            .type = event_type::disconnected,
            .disconnected = {
                .type = type,
                .reason = reason
            }
        });
    }
    
	bool client_fsm::on_recv_active_disconnect_req(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_disconnect_req))
            return false;

        auto* disc = reinterpret_cast<const packet_disconnect_req*>(p.payload);
        emit_send_disconnect_ack();

        disconnect_reason reason = disc->reason;
        reason.size = to_native16(reason.size);

        emit_disconnected(disc->type, reason);
        main_stage = client_main_stage::disconnected;
        return true;
    }

	bool client_fsm::on_recv_active_sv_status_ack(const packet& p)
    {
        return false;
    }
    
    bool client_fsm::on_recv_active_sv_snapshot(const packet& p)
    {
        return false;
    }

	bool client_fsm::on_recv_active(const packet& p)
    {
        if (validate_all_packet(p, client_nonce))
        {
            switch (p.header.type) 
            {
            case packet_type::disconnect_req:
                return on_recv_active_disconnect_req(p);
                break;
            case packet_type::sv_status_ack:
                return on_recv_active_sv_status_ack(p);
                break;
            case packet_type::sv_snapshot:
		        return on_recv_active_sv_snapshot(p);
                break;
            default:
                break;
            }
        }

        return false;
    }
    
	bool client_fsm::on_recv_disconnecting(const packet& p)
    {
        if (p.header.type != packet_type::disconnect_ack || p.header.payload_size != 0/*sizeof(packet_disconnect_ack)*/)
            return false;

        emit_disconnected({}, {});
        main_stage = client_main_stage::disconnected;

        return true;
    }

	bool client_fsm::cancel()
    {
        using enum client_main_stage;
        switch (main_stage) 
        {
		case connecting:
		case loading:
        {
            packet_conn_cancel conn_cancel;
            conn_cancel.client_nonce = to_protocol64(client_nonce);

            emit_send(
                make_packet(
                    curr_timestamp_us,
                    packet_type::conn_cancel,
                    0,
                    server_nonce,
                    sizeof(conn_cancel), 
                    &conn_cancel 
                ),
                true
            );

            main_stage = client_main_stage::disconnected;
        }
            return true;
        default:
            break;
        }

        return false;
    }

    
	void client_fsm::emit_conn_req()
    {
        packet_conn_req conn_req
        {
            .client_nonce = to_protocol64(client_nonce)
        };

        emit_send(
            make_packet(
                curr_timestamp_us,
                packet_type::conn_req, 
                0,
                -1,
                sizeof(conn_req), 
                &conn_req 
            ),
            true
        );
    }

	bool client_fsm::connect_to(uint64_t nonce)
    {
        if (main_stage != client_main_stage::disconnected)
        {
            error(event_error_type::bad_stage, protocol_error::remote_violation);
            return false;
        }

        client_nonce = nonce;
        emit_conn_req();
        main_stage = client_main_stage::connecting;

        return true;
    }
    
	void client_fsm::on_recv_packet(const byte_span& data)
    {
        bool processed = false;
        packet p;
        
        if (parse_packet(p, data.size(), data.data()))
        {
            if (validate_all_packet(p, client_nonce))
            {
                if (p.header.type == packet_type::error)
                {   
                    processed = on_recv_error(p);
                }
                else 
                {
                    using enum client_main_stage;
                    switch (main_stage) 
                    {
                    case connecting:
                        processed = on_recv_connecting(p);
                        break;
                    case loading:
                        processed = on_recv_loading(p);
                        break;
                    case active:
                        processed = on_recv_active(p);
                        break;
                    case disconnecting:
                        processed = on_recv_disconnecting(p);
                        break;
                    default:
                        break;
                    }
                }
            }
        }  

        if (!processed)
        {
            error(event_error_type::bad_packet, protocol_error::unexcepted_packet);
        }
        else
        {
            reset_timeouts();
        }
    }

	bool client_fsm::poll_event(event& e)
    {
        return events.pop(e);   
    }

}