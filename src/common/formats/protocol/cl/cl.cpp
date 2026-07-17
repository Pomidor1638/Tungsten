
#include "cl.h"
#include "../packets/packets.h"

namespace tungsten::protocol 
{

    client_fsm::client_fsm(
        void* ctx,
        timeout_config cfg,
        base_fsm_callbacks cmn_callbacks,
        client_callbacks cl_callbacks
    ) 
        : base_fsm{ ctx, cfg, cmn_callbacks }
        , cl_callbacks{ cl_callbacks }
    {
        pure_reset();
    }

    void client_fsm::on_reset()
    {
        pure_reset();
    }

    void client_fsm::on_tick()
    {
    }
    
    bool client_fsm::on_disconnect_req(disconnect_type type, const disconnect_reason& reason)
    {
        if (main_stage != client_main_stage::active)
        {
            error(fsm_error{ fsm_error_type::unexpected_packet, protocol_error::remote_violation }, true);
            return false;
        }

        call_disconnect_req(type, reason);
        main_stage = client_main_stage::disconnecting;

        return true;
    }

    bool client_fsm::on_disconnect_ack()
    {
        if (main_stage != client_main_stage::active)
        {
            error(fsm_error{ fsm_error_type::unexpected_packet, protocol_error::remote_violation }, true);
            return false;
        }

        call_disconnect_ack();
        main_stage = client_main_stage::disconnected;

        return true;
    }

    void client_fsm::pure_reset()
    {
        main_stage       = client_main_stage      ::none;
        loading_stage    = client_loading_stage   ::none;
        file_sync_stage  = client_file_sync_stage ::none;
        level_sync_stage = client_level_sync_stage::none;
    }



    bool client_fsm::call_connection_accepted(bool need_file_sync)
    {
        if (cl_callbacks.on_conn_accepted)
        {
            cl_callbacks.on_conn_accepted(need_file_sync);
        }
        return true;
    }

    bool client_fsm::call_connection_rejected(const reject_reason& reason)
    {
        if (cl_callbacks.on_conn_rejected)
        {
            cl_callbacks.on_conn_rejected(reason);
        }
        return true;
    }

	bool client_fsm::process_conn_accept(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_conn_accept))
            return false;

        auto* conn_accept = reinterpret_cast<const packet_conn_accept*>(p.payload);
        remote_nonce = conn_accept->server_nonce;

        call_connection_accepted(conn_accept->need_file_sync);
        
        disarm_timeout();
        main_stage = client_main_stage::active;

        return true;
    }

	bool client_fsm::process_conn_reject(const packet& p)
    {
        if (p.header.payload_size != sizeof(packet_conn_reject))
            return false;

        auto* conn_reject = reinterpret_cast<const packet_conn_reject*>(p.payload);
        reject_reason reason = conn_reject->reason;
        call_connection_rejected(reason);
        main_stage = client_main_stage::disconnected;
        
        return true;
    }

    bool client_fsm::on_recv_connecting(const packet& p)
    {
        switch (p.header.type) 
        {
        case packet_type::conn_accept:
            return process_conn_accept(p);
        case packet_type::conn_reject:
            return process_conn_reject(p);
        default:
            error(fsm_error{
                .type = fsm_error_type::unexpected_packet,
                .protocol_err = protocol_error::unexcepted_packet
                }, false);
            break;
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


    bool client_fsm::send_conn_req()
    {
        packet p;
        packet_conn_req conn_req;

        conn_req.client_nonce = my_nonce;

        if (!make_packet_header_internal(p, packet_type::conn_req, 0, sizeof(conn_req), &conn_req))
        {
            return false;
        }

        return send(p);
    }

    bool client_fsm::send_disconnect_ack()
    {
        packet p;
        if (!make_packet_header_internal(p, packet_type::disconnect_ack, 0, 0, nullptr))
        {
            return false;
        }
        return send(p);
    }

    bool client_fsm::call_disconnect_ack()
    {
        if (cl_callbacks.on_disconnect_ack)
        {
            cl_callbacks.on_disconnect_ack();
        }
        return true;
    }

    bool client_fsm::call_disconnect_req(disconnect_type type, const disconnect_reason& reason)
    {
        if (cl_callbacks.on_disconnect_req)
        {
            cl_callbacks.on_disconnect_req(type, reason);
        }
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
        switch (p.header.type) 
        {
        //case packet_type::disconnect_req:
        //    return on_recv_active_disconnect_req(p);
        //case packet_type::sv_status_ack:
        //    return on_recv_active_sv_status_ack(p);
        //case packet_type::sv_snapshot:
	    //    return on_recv_active_sv_snapshot(p);
        default:
            break;
        }
        return false;
    }
    
	bool client_fsm::on_recv_disconnecting(const packet& p)
    {
        if (p.header.type != packet_type::disconnect_ack || p.header.payload_size != 0/*sizeof(packet_disconnect_ack)*/)
            return false;

        // call_disconnected({}, {});
        main_stage = client_main_stage::disconnected;

        return true;
    }

	//bool client_fsm::cancel()
 //   {
 //       using enum client_main_stage;
 //       switch (main_stage) 
 //       {
	//	case connecting:
	//	case loading:
 //       {
 //           // TODO: send_conn_cancel

 //           main_stage = client_main_stage::disconnected;
 //       }
 //           return true;
 //       default:
 //           break;
 //       }

 //       return false;
 //   }

	bool client_fsm::open(uint64_t nonce)
    {
        if (main_stage != client_main_stage::disconnected)
        {
            error(fsm_error{ fsm_error_type::unknown_signal, protocol_error::remote_violation }, true);
            return false;
        }

        my_nonce        = nonce;
        remote_nonce    = NO_NONCE;

        if (!send_conn_req())
            return false;
        
        arm_timeout();
        main_stage = client_main_stage::connecting;
        return true;
    }
    
    bool client_fsm::on_recv_custom()
    {
        using enum client_main_stage;
        switch (main_stage)
        {
        case connecting:
            return on_recv_connecting();
            break;
        case loading:
            return on_recv_loading();
            break;
        case active:
            return on_recv_active();
            break;
        case disconnecting:
            return on_recv_disconnecting();
            break;
        default:
            error(fsm_error{
                .type = fsm_error_type::unknown_stage,
                .protocol_err = protocol_error::remote_violation
                }, false);
            break;
        }

        return false;
    }
}