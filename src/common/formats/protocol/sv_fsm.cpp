
#include "protocol.h"

namespace tungsten::protocol 
{

    bool server_fsm::push_event(const event& e)
    {
        return events.push(e);
    }

    void server_fsm::tick(uint64_t delta_time_us)
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

    
	void server_fsm::check_timeouts()
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
                emit_error(event_error_type::timeout);
                main_stage = server_main_stage::empty;
            }
        }
    }
   

	void server_fsm::reset_timeouts()
    {
        last_recv_timestamp_us = curr_timestamp_us;
		tries_count = retry_count;
    }

	void server_fsm::reset()
    {
        last_timestamp_us = 0;
        curr_timestamp_us = 0;
        delta_us		  = 0;
         
        server_nonce = 0;
        client_nonce = 0;

        main_stage = server_main_stage::empty;
    }

	bool server_fsm::poll_event(event& e)
    {
        return events.pop(e);
    }

    
	void server_fsm::emit_send(const packet& p, bool reliable)
    {
        push_event(
            event{
                .type = event_type::send,
                .send = {
                    .reliable = reliable,
                    .p = p,
                }
            }
        );
    }

	void server_fsm::emit_error(event_error_type type)
    {
        push_event(
            event{
                .type = event_type::error,
                .error = {
                    .type = type
                }
            }
        );
    }

    void server_fsm::emit_connection_canceled()
    {
        push_event(
            event{
                .type = event_type::connection_canceled,
            }
        );
    }
    
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

	bool server_fsm::on_recv_loading(const packet& p)
    {
        return false;
    }

	bool server_fsm::on_recv_active(const packet& p)
    {
        return false;
    }

	bool server_fsm::on_recv_disconnecting(const packet& p)
    {
        return false;
    }

    bool server_fsm::accept(uint64_t cl_nonce, uint64_t sv_nonce, bool need_filesync)
    {
        if (main_stage != server_main_stage::empty)
            return false;

        client_nonce = cl_nonce;
        server_nonce = sv_nonce;

        packet_conn_accept conn_accept
        {
            .server_nonce = server_nonce,
            .need_filesync = need_filesync
        };

        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::conn_accept,
                sizeof(conn_accept), 
                &conn_accept, 
                0, 
                client_nonce
            ),
            true
        );

        reset_timeouts();
        main_stage = server_main_stage::loading;

        return true;
    }

	bool server_fsm::reject(packet& out, uint64_t timestamp_us, uint64_t cl_nonce, reject_reason reason)
    {

        packet_conn_reject conn_reject;
        conn_reject.reason = reason;

        out = make_packet(
            timestamp_us, 
            packet_type::conn_reject, 
            sizeof(conn_reject), 
            &conn_reject, 
            0, 
            cl_nonce
        );

        return true;
    }

    
	bool server_fsm::is_conn_req(const packet& p)
    {
        return validate_magic(p.header) 
            && validate_packet_sizes(p.header)
            && validate_checksum(p)
            && p.header.type == packet_type::conn_req;
    }

	void server_fsm::on_recv_packet(const packet& p)
    {
        bool processed = false;

        if (validate_packet_sizes(p.header))
        {
            using enum server_main_stage;
            switch (main_stage) 
            {
            case empty:
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

        if (processed)
        {
            reset_timeouts();
        }
    }

	void server_fsm::disconnect()
    {
    }
}