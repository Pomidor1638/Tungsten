
#include "sv.h"
#include "protocol/base_fsm.h"
#include "protocol/packets.h"
#include "protocol/protocol.h"
#include <cstring>


/*
    TODO:
        It would be worth creating a method for generating packets 
        so you don't have to specify `timestamp_us` every time.
*/

namespace tungsten::protocol 
{
    /*
        All connection requests must be processed by the server itself, 
        and only after acceptance, open it.
    */


    server_client_fsm::server_client_fsm(
        void* ctx,
        timeout_config cfg,
        base_fsm_callbacks cmn_callbacks,
        server_callbacks sv_callbacks
    ) 
        : base_fsm{ ctx, cfg, cmn_callbacks }
        , sv_callbacks{sv_callbacks}
    {
        pure_reset();
    }

    bool server_client_fsm::on_recv_connecting(const packet& p)
    {
        error(fsm_error{
            .type = fsm_error_type::unexpected_packet, 
            //.protocol_err = protocol_error::unexcepted_packet
            }, false);
        return false;
    }
    
    bool server_client_fsm::on_disconnect_req(disconnect_type type, const disconnect_reason& reason)
    {
        switch (main_stage) 
        {
        case server_main_stage::active:
            main_stage = server_main_stage::empty;
            if (!send_disconnect_ack())
            {
                return false;
            }
            return call_disconnected(type, reason);
        default:
            break;
        }
        return false;
    }
    
    bool server_client_fsm::on_disconnect_ack()
    {
        switch (main_stage) 
        {
        case server_main_stage::disconnecting:
            main_stage = server_main_stage::empty;
            return true;
        default:
            break;
        }
        return false;
    }

    bool server_client_fsm::on_recv_custom()
    {
        using enum server_main_stage;
        switch(main_stage)
        {
        case connecting:
            return on_recv_connecting();
        case loading:
            return on_recv_loading();
        case active:
            return on_recv_active();
        case disconnecting:
            return on_recv_disconnecting();
        default:
            error(fsm_error{
                .type = fsm_error_type::unknown_stage, 
                .protocol_err = protocol_error::remote_violation
            }, false);
            break;
        }
        
        return false;
    }   

    void server_client_fsm::on_reset()
    {
        pure_reset();
    }

    void server_client_fsm::on_tick()
    {

    }
    
    /*
        TODO:
            All substages of the loading stage need to be implemented.
    */
    
	bool server_client_fsm::on_recv_loading_file_sync()
    {
        return false;
    }

	bool server_client_fsm::on_recv_loading_level_sync()
    {
        return false;
    }

    /*
        TODO:
            Need to find a way to make `snapshot_sync` usable during the active stage as well.
    */

	bool server_client_fsm::on_recv_loading_snapshot_sync()
    {
        return false;
    }

    
	bool server_client_fsm::on_recv_loading()
    {
        using enum server_loading_stage;
        switch (loading_stage) 
        {
		case file_sync:
            return on_recv_loading_file_sync();
		case level_sync:
            return on_recv_loading_level_sync(); 
		case snapshot_sync:
            return on_recv_loading_snapshot_sync();
        default:
            break;
        }
        return false;
    }
    
    /*
        TODO:
            need to implement the processing of status_req and usercmd packets.
    */

	bool server_client_fsm::on_recv_active_cl_status_req()
    {
        return false;
    }

    bool server_client_fsm::on_recv_active_cl_usercmd()
    {
        return false;
    }

	bool server_client_fsm::on_recv_active()
    {
        using enum packet_type;
        switch (p.header.type) 
        {
        //case cl_status_req:     return on_recv_active_cl_status_req();
        //case cl_usercmd:        return on_recv_active_cl_usercmd();
        default:
            error(fsm_error{fsm_error_type::unexpected_packet, protocol_error::unexcepted_packet}, true);
            break;
        }
        return false;
    }
    
    bool server_client_fsm::send_conn_accept(bool need_file_sync)
    {
        packet p;
        packet_conn_accept conn_accept
        {
            .server_nonce = my_nonce,
            .need_file_sync = need_file_sync
        };

        if (!make_packet_header_internal(
            p,
            packet_type::conn_accept,
            0, 
            sizeof(conn_accept), 
            &conn_accept
        )) {
            return false;
        }
        
        return send(p);
    }

    bool server_client_fsm::open(uint64_t cl_nonce, uint64_t sv_nonce, bool need_file_sync)
    {
        if (main_stage != server_main_stage::empty)
        {
            error(fsm_error{fsm_error_type::unexcepted_signal}, false);
            return false;
        }

        remote_nonce = cl_nonce;
        my_nonce     = sv_nonce;

        // TODO: need to fix, 
        // now it's only for `active` stage test
        // loading stage is too hard now

        if (!send_conn_accept(need_file_sync))
            return false;
        
        arm_timeout();
        main_stage = server_main_stage::active;

        return true;
    }

    
    bool server_client_fsm::reject(uint64_t timestamp_us, uint64_t cl_nonce, reject_reason reason, int max_size, int& out_size, void* out_data)
    {
        out_size = 0;

        if (!out_data)
            return false;

        if (max_size < PACKET_HEADER_SIZE)
            return false;

        packet_conn_reject payload;
        payload.reason = reason;
        
        swap_protocol_string(payload.reason);

        packet p;

        bool ok = make_packet(
            p,
            timestamp_us,
            packet_type::conn_reject,
            0,
            cl_nonce,
            sizeof(payload),
            &payload
        );

        if (!ok)
            return false;

        int packet_size = p.header.header_size + p.header.payload_size;

        if (packet_size > max_size)
            return false;

        if (!parse_to_protocol_packet(packet_size, p))
            return false;

        memcpy(out_data, &p, packet_size);

        out_size = packet_size;
        return true;
    }
    
	bool server_client_fsm::is_conn_req(const data_span& data, uint64_t& client_nonce, bool& bad_version)
    {
        packet p;
        bad_version = false;

        if (!parse_packet(p, data.size(), data.data()))
            return false;

        packet_header& header = p.header;

        if (header.payload_size == sizeof(packet_conn_req)
            && validate_magic(header) 
            && validate_checksum(p) 
            && header.type == packet_type::conn_req
        ) {
            auto* conn_req = reinterpret_cast<const packet_conn_req*>(p.payload);
            bad_version  = !validate_protocol_version(header);
            return true;
        }

        return false;
    }

    
	bool server_client_fsm::is_status_req(const data_span& data)
    {
        // if (data.size() < PACKET_HEADER_SIZE + sizeof(packet_cl_status_req))
        // {
        //     return false;
        // }

        // auto* p = reinterpret_cast<const packet*>(data.data());

        // packet_header header = p->header;

        return false;
    }
    
    bool server_client_fsm::send_disconnect_ack()
    {
        packet p;
        if (!make_packet_header_internal(p, packet_type::disconnect_ack, 0, 0, nullptr))
            return false;

        return send(p);
    }

    
    void server_client_fsm::disconnect(disconnect_type type, disconnect_reason reason)
    {
        if (main_stage != server_main_stage::active)
        {
            error(fsm_error{fsm_error_type::unexcepted_signal, protocol_error::remote_violation}, true);
            return;
        }

        if (!send_disconnect_req(type, reason))
            return;

        main_stage = server_main_stage::disconnecting;
    }

    bool server_client_fsm::send_disconnect_req(disconnect_type type, disconnect_reason reason)
    {
        packet p;
        packet_disconnect_req req
        {
            .type = type,
            .reason = reason
        };

        if (!make_packet_header_internal(p, packet_type::disconnect_req, 0, sizeof(req), &req))
            return false;

        return send(p);
    }


    void server_client_fsm::pure_reset()
    {
        main_stage         = server_main_stage          ::empty;
        loading_stage      = server_loading_stage       ::none;
        file_sync_stage    = server_file_sync_stage     ::none;
        gamesync_stage     = server_level_sync_stage    ::none;
        snapshot_sync_stage= server_snapshot_sync_stage ::none;
    }

	void server_client_fsm::close()
    {
        using enum server_main_stage;
        switch (main_stage) 
        {
        case active:
            disconnect(disconnect_type::closed, disconnect_reason{});
            //send_disconnect_req(type, reason);
            main_stage = disconnecting;
            break;
        case connecting:
        case loading:
            // TODO: need to send cancel
            //main_stage = empty;
            //break;
        default:
            error(fsm_error{fsm_error_type::unexcepted_signal}, false);
            break;
        }
    }
}