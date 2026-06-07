
#include "protocol.h"

namespace tungsten::protocol 
{

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
        
        using enum client_main_stage;
        switch (main_stage) 
        {
        case disconnected:
            break;
		case connecting:
            break;
		case loading:
            break;
		case active:
            break;
		case disconnecting:
            break;
        default:
            break;
        }
        
    }
    
	void client_fsm::emit_connection_accepted(bool need_filesync)
    {
        events.push(
            event{
                .type = event_type::connection_accepted,
                .conn_accepted = {
                    .need_filesync = need_filesync
                }
            }
        );
    }

	void client_fsm::emit_connection_rejected(reject_reason reason)
    {
        events.push(
            event{
                .type = event_type::connection_rejected,
                .conn_rejected = {.reason = reason }
            }
        );
    }
    
	void client_fsm::emit_error(event_error_type type)
    {
        events.push(
            event{
                .type = event_type::error,
                .error = {.type = type}
            }
        );
    }
    
	void client_fsm::emit_send(const packet& p, bool reliable)
    {
        events.push(event{
            .type = event_type::send,
            .send = {
                .reliable = reliable,
                .p = p, 
            },
        });
    }

    
	void client_fsm::recv_conn_accept(const packet& p)
    {
        auto* conn_accept = reinterpret_cast<const packet_conn_accept*>(p.payload);
        
        server_nonce = conn_accept->server_nonce;

        emit_connection_accepted(conn_accept->need_filesync);
        
        reset_timeouts();
        loading_stage = client_loading_stage::filesync;
        main_stage    = client_main_stage   ::loading;
    }

	void client_fsm::recv_conn_reject(const packet& p)
    {
        auto* conn_reject = reinterpret_cast<const packet_conn_reject*>(p.payload);

        emit_connection_rejected(conn_reject->reason);
        
        reset_timeouts();
        main_stage = client_main_stage::disconnected;
    }

    void client_fsm::on_recv_connecting(const packet& p)
    {
        if (!validate_packet(p, client_nonce))
            return;

        using enum packet_type;
        switch (p.header.type) 
        {
        case conn_accept:
            recv_conn_accept(p);
            break;
        case conn_reject:
            recv_conn_reject(p);
            break;
        default:
            break;
        }
    }

	void client_fsm::on_recv_loading(const packet& p)
    {
    }

	void client_fsm::on_recv_active(const packet& p)
    {
    }
    
	void client_fsm::on_recv_disconnecting(const packet& p)
    {
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

            emit_send(
                make_packet(
                    curr_timestamp_us,
                    packet_type::conn_cancel,
                    sizeof(conn_cancel), 
                    &conn_cancel, 
                    0,
                    server_nonce
                ),
                true
            );

            reset_timeouts();
            main_stage = client_main_stage::disconnected;
        }
            return true;
        default:
            break;
        }

        return false;
    }

	bool client_fsm::connect_to(uint64_t nonce)
    {
        using enum client_main_stage;
        if (main_stage != disconnected)
            return false;

        client_nonce = nonce;
        packet_conn_req conn_req;
        conn_req.client_nonce = nonce;

        emit_send(
            make_packet(
                curr_timestamp_us,
                packet_type::conn_req, 
                sizeof(conn_req), 
                &conn_req, 
                0,
                -1
            ), 
            true
        );

        reset_timeouts();
        main_stage = connecting;
        return true;
    }
    
	void client_fsm::on_recv_packet(const packet& p)
    {
        if (!validate_packet_sizes(p.header))
            return;

        using enum client_main_stage;
        switch (main_stage) 
        {
        case disconnected:
            break;
		case connecting:
            on_recv_connecting(p);
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

	bool client_fsm::poll_event(event& e)
    {
        return events.pop(e);   
    }

}