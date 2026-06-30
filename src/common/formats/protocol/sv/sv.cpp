
#include "sv.h"

#include "protocol/protocol.h"
#include "protocol/packets.h"

#include <cstring>
  
/*
    TODO:
        It would be worth creating a method for generating packets 
        so you don't have to specify `timestamp_us` every time.
*/

namespace tungsten::protocol 
{
    bool server_client_fsm::push_event(const event& e)
    {
        // TODO: need to handle queue overflow in some way in the future
        bool ok = events.push(e);
        // Trigger an assertion at runtime and crash the entire program?
        // Just for testing purposes for now.
        assert(ok && "protocol event queue overflow");
        return ok;
    }

	bool server_client_fsm::push_event(event&& e)
    {
        bool ok = events.push(std::move(e));
        // Trigger an assertion at runtime and crash the entire program?
        // Just for testing purposes for now.
        assert(ok && "protocol event queue overflow");
        return ok;
    }
  

    /*
        It must be called before any action 
        at the start of each processing iteration 
        to ensure correct handling of timeouts and packet timestamps.
    */
    void server_client_fsm::tick(uint64_t delta_time_us)
    {
        last_timestamp_us = curr_timestamp_us;
        curr_timestamp_us += delta_time_us;
        delta_us = delta_time_us; 

        check_timeouts();

        using enum server_main_stage;
        switch (main_stage) 
        {
        default:
            break;
        }
    }

    /*
        For handling all timeouts. 
        In principle, every received packet should reset the timeouts.
    */
	void server_client_fsm::check_timeouts()
    {
        if (main_stage == server_main_stage::empty)
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
            }
        }
    }
   
    /*
        Just a counter reset.
    */
	void server_client_fsm::reset_timeouts()
    {
        last_recv_timestamp_us = curr_timestamp_us;
		tries_count = retry_count;
    }

    /*
        Reset the entire state, including the event queue.
    */
	void server_client_fsm::reset()
    {
        last_timestamp_us = 0;
        curr_timestamp_us = 0;
        delta_us		  = 0;
         
        server_nonce = 0;
        client_nonce = 0;

        reset_timeouts();
        events.clear();
        main_stage = server_main_stage::empty;
    }

    /*
        Regarding the primary method for retrieving all events, 
        it might be worth finding a way to handle the "send" 
        events that appear after cancellation.
    */
	bool server_client_fsm::poll_event(event& e)
    {
        return events.pop(e);
    }

    
	void server_client_fsm::emit_send(const packet& p, bool reliable)
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

	void server_client_fsm::emit_error(event_error_type type, protocol_error protocol)
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

    
    void server_client_fsm::emit_send_error(protocol_error code)
    {
        packet_error err
        {
            .code = code
        };

        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::error, 
                0, 
                client_nonce, 
                sizeof(err), 
                &err
            ),
            true
        );
    }

    
    void server_client_fsm::error(event_error_type type, protocol_error protocol)
    {
        emit_send_error(protocol);
        emit_error(type, protocol);
        
        /*
        QUESTION:
            Maybe we should call reset() here?
        ANSWER:
            reset() clears the event queue, but the error event must be processed 
            and a protocol error sent  (the "send" event must be sent first, followed by the error).
        */

        main_stage = server_main_stage::empty;
    }

    void server_client_fsm::emit_connection_canceled()
    {
        push_event(
            event{
                .type = event_type::connection_canceled,
            }
        );
    }

    void server_client_fsm::emit_send_disconnect_req(disconnect_type type, disconnect_reason reason)
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
                client_nonce, 
                sizeof(disconnect_req), 
                &disconnect_req
            ), 
            true
        );
    }

    
    void server_client_fsm::emit_send_disconnect_ack()
    {
        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::disconnect_ack, 
                0, 
                client_nonce, 
                0, 
                nullptr
            ), 
            true
        );
    }

    void server_client_fsm::emit_disconnected(disconnect_type type, disconnect_reason reason)
    {
        events.push(
            event{
                .type = event_type::disconnected,
                .disconnected = {
                    .type = type,
                    .reason = reason
                }
            }
        );
    }
    
    /*
        All connection requests must be processed by the server itself, 
        and only after acceptance, open it.
    */

	/*bool server_fsm::on_recv_empty(const packet& p)
    {
        if (p.header.type != packet_type::conn_req)
            return false;

        auto* conn_req = reinterpret_cast<const packet_conn_req*>(p.payload);

        client_nonce = conn_req->client_nonce;
        emit_conn_requested();

        main_stage = server_main_stage::pending_connection;
        return true;
    }*/

    /*bool server_fsm::on_recv_pending_connection(const packet& p)
    {
        if (p.header.type != packet_type::conn_cancel)
            return false;

        auto* conn_cancel = reinterpret_cast<const packet_conn_cancel*>(p.payload);
        if (conn_cancel->client_nonce != client_nonce)
            return false;
        
        emit_connection_canceled();

        main_stage = server_main_stage::empty;

        return true;
    }*/

    /*
        TODO:
            All substages of the loading stage need to be implemented.
    */
    
	bool server_client_fsm::on_recv_loading_file_sync(const packet& p)
    {
        return false;
    }

	bool server_client_fsm::on_recv_loading_level_sync(const packet& p)
    {
        return false;
    }

    /*
        TODO:
            Need to find a way to make `snapshot_sync` usable during the active stage as well.
    */

	bool server_client_fsm::on_recv_loading_snapshot_sync(const packet& p)
    {
        return false;
    }

    
	bool server_client_fsm::on_recv_loading(const packet& p)
    {
        using enum server_loading_stage;
        switch (loading_stage) 
        {
		case file_sync:
            return on_recv_loading_file_sync(p);
		case level_sync:
            return on_recv_loading_level_sync(p); 
		case snapshot_sync:
            return on_recv_loading_snapshot_sync(p);
        default:
            break;
        }
        return false;
    }

    bool server_client_fsm::on_recv_active_disconnect_req(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_disconnect_req))
            return false;

        auto* disconnect_req = reinterpret_cast<const packet_disconnect_req*>(p.payload);

        // !!! VERY IMPORTANT !!!
        // this packet does not need in swap

        emit_send_disconnect_ack();

        disconnect_reason reason = disconnect_req->reason;

        reason.size = to_native16(reason.size);

        emit_disconnected(disconnect_req->type, reason);
        main_stage = server_main_stage::empty;
        return true;
    }
    
    /*
        TODO:
            need to implement the processing of status_req and usercmd packets.
    */

	bool server_client_fsm::on_recv_active_cl_status_req(const packet& p)
    {
        return false;
    }

    bool server_client_fsm::on_recv_active_cl_usercmd(const packet& p)
    {
        return false;
    }

	bool server_client_fsm::on_recv_active(const packet& p)
    {
        using enum packet_type;
        switch (p.header.type) 
        {
        case disconnect_req:
            return on_recv_active_disconnect_req(p);  
        case cl_status_req:
            return on_recv_active_cl_status_req(p);
        case cl_usercmd:
            return on_recv_active_cl_usercmd(p);
        default:
            break;
        }
        return false;
    }

	bool server_client_fsm::on_recv_disconnecting(const packet& p)
    {
        if (p.header.type != packet_type::disconnect_ack || p.header.payload_size != 0/*sizeof(packet_disconnect_ack)*/)
            return false;

        emit_disconnected({},{});
        main_stage = server_main_stage::empty;
        
        return true;
    }

    bool server_client_fsm::open(uint64_t cl_nonce, uint64_t sv_nonce, bool need_file_sync)
    {
        if (main_stage != server_main_stage::empty)
        {
            error(event_error_type::bad_stage, protocol_error::remote_violation);
            return false;
        }

        client_nonce = cl_nonce;
        server_nonce = sv_nonce;

        packet_conn_accept conn_accept
        {
            .server_nonce = to_protocol64(server_nonce),
            .need_file_sync = need_file_sync
        };

        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::conn_accept,
                0, 
                client_nonce,
                sizeof(conn_accept), 
                &conn_accept
            ),
            true
        );

        reset_timeouts();

        // TODO: need to fix, 
        // now it's only for `active` stage test
        // loading stage is too hard

        main_stage = server_main_stage::active;
        
        return true;
    }

	bool server_client_fsm::reject(event_send& out, uint64_t timestamp_us, uint64_t cl_nonce, reject_reason reason)
    {
        packet_conn_reject conn_reject
        {
            .reason = reason
        };

        conn_reject.reason.size = to_protocol16(conn_reject.reason.size);

        packet p = make_packet(
            timestamp_us, 
            packet_type::conn_reject, 
            0, 
            cl_nonce,
            sizeof(conn_reject), 
            &conn_reject
        );


        out.reliable = true;
        out.size = p.header.header_size + p.header.payload_size;
        packet_header_to_protocol(p.header);
        memcpy(out.data, &p, out.size);

        return true;
    }

    
	bool server_client_fsm::is_conn_req(const byte_span& data, uint64_t& client_nonce, bool& bad_version)
    {
        packet p;
        bad_version = false;

        if (!parse_packet(p, data.size(), data.data()))
            return false;
                
        packet_header& header = p.header;

        if (header.payload_size == sizeof(packet_conn_req)
            && validate_magic(header) 
            && validate_checksum
            (
                header.checksum, 
                static_cast<int>(header.payload_size), 
                p.payload
            ) 
            && header.type == packet_type::conn_req
        ) {
            auto* conn_req = reinterpret_cast<const packet_conn_req*>(p.payload);
            client_nonce = to_native64(conn_req->client_nonce);
            bad_version = validate_protocol_version(header);
            return true;
        }

        return false;
    }

    
	bool server_client_fsm::is_status_req(const byte_span& data)
    {
        // if (data.size() < PACKET_HEADER_SIZE + sizeof(packet_cl_status_req))
        // {
        //     return false;
        // }

        // auto* p = reinterpret_cast<const packet*>(data.data());

        // packet_header header = p->header;

        return false;
    }
    

    
    bool server_client_fsm::on_recv_error(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_error))
            return false;

        // TODO: packet check
        auto* pkt_error = reinterpret_cast<const packet_error*>(p.payload);
        // because error sends packet_error
        // when RECEIVED error DO NOT send error too
        emit_error(event_error_type::protocol_error, pkt_error->code);
        main_stage = server_main_stage::empty;
        return true;
    }

	void server_client_fsm::on_recv_packet(const byte_span& data)
    {
        bool processed = false;
        packet p;
        
        if (parse_packet(p, data.size(), data.data()))
        {
            if (validate_all_packet(p, server_nonce))
            {
                if (p.header.type == packet_type::error)
                {   
                    processed = on_recv_error(p);
                }
                else
                {
                    using enum server_main_stage;
                    switch (main_stage) 
                    {
                    // empty stage don't do anything
                    //case empty:
                    //    break;
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

        // because an unexpected, corrupted, or otherwise invalid packet is unacceptable
        if (!processed)
        {
            error(event_error_type::bad_packet, protocol_error::unexcepted_packet);
        }
        else
        {
            reset_timeouts();
        }
    }

	void server_client_fsm::close(disconnect_type type, disconnect_reason reason)
    {
        switch (main_stage) 
        {
        case server_main_stage::connecting:
        case server_main_stage::loading:
        case server_main_stage::active:
            emit_send_disconnect_req(type, reason);
            main_stage = server_main_stage::disconnecting;
            break;
        default:
            error(event_error_type::bad_stage, protocol_error::remote_violation);
            break;
        }
    }
}