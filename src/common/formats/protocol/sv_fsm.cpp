
#include "protocol.h"

namespace tungsten::protocol 
{

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
        
        if (delta > retry_time)
        {
            if (tries_count)
            {
                tries_count--;
            }
            else
            {
                emit_error(event_error_type::timeout);
                
                reset_timeouts();
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

        reset_timeouts();
        main_stage = server_main_stage::empty;
    }

	bool server_fsm::poll_event(event& e)
    {
        return events.pop(e);
    }

    
	void server_fsm::emit_send(const packet& p, bool reliable)
    {
        events.push(
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
        events.push(
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
        events.push(
            event{
                .type = event_type::connection_canceled,
            }
        );
    }

	void server_fsm::emit_conn_requested()
    {
        events.push(
            event{
                .type = event_type::connection_requested,
                .conn_requested{}
            }
        );
    }
    
	void server_fsm::on_recv_empty(const packet& p)
    {
        if (p.header.type != packet_type::conn_req)
            return;

        auto* conn_req = reinterpret_cast<const packet_conn_req*>(p.payload);

        client_nonce = conn_req->client_nonce;
        emit_conn_requested();

        reset_timeouts();
        main_stage = server_main_stage::waiting_conn_result;
    }

    void server_fsm::on_recv_waiting_conn_result(const packet& p)
    {
        // if (p.header.type != packet_type::conn_cancel)
        //     return;

        // auto* conn_cancel = reinterpret_cast<const packet_conn_cancel*>(p.payload);
        // if (conn_cancel->client_nonce != client_nonce)
        //     return;
        
        // emit_connection_canceled();

        // reset_timeouts();
        // main_stage = server_main_stage::empty;
    }

	void server_fsm::on_recv_loading(const packet& p)
    {
    }

	void server_fsm::on_recv_active(const packet& p)
    {
    }

	void server_fsm::on_recv_disconnecting(const packet& p)
    {
    }

    bool server_fsm::accept(uint64_t nonce, bool need_filesync)
    {
        if (main_stage != server_main_stage::waiting_conn_result)
            return false;

        server_nonce = nonce;
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
        loading_stage = server_loading_stage::filesync;
        main_stage    = server_main_stage   ::loading;

        return true;
    }

	bool server_fsm::reject(reject_reason reason)
    {
        if (main_stage != server_main_stage::waiting_conn_result)
            return false;

        packet_conn_reject conn_reject;
        conn_reject.reason = reason;

        emit_send(
            make_packet(
                curr_timestamp_us, 
                packet_type::conn_reject, 
                sizeof(conn_reject), 
                &conn_reject, 
                0, 
                client_nonce
            ), 
            true
        );

        reset_timeouts();
        main_stage = server_main_stage::empty;

        return true;
    }

	void server_fsm::on_recv_packet(const packet& p)
    {
        if (!validate_packet_sizes(p.header))
            return;

        using enum server_main_stage;
        switch (main_stage) 
        {
        case empty:
            on_recv_empty(p);
            break;
        case waiting_conn_result:
            //on_recv_waiting_conn_result(p);
            break;
        case loading:
            on_recv_loading(p);
            break;
		case active:
            on_recv_active(p);
            break;
		case disconnecting:
            on_recv_disconnecting(p);
            break;
        default:
            break;
        }
    }

	void server_fsm::disconnect()
    {
    }
}