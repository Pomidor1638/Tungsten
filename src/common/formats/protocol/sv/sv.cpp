
#include "sv.h"
#include "../packets/packets.h"
#include <cstring>


/*
    TODO:
        It would be worth creating a method for generating packets 
        so you don't have to specify `timestamp_us` every time.
*/

namespace tungsten::protocol 
{


    void server_client_fsm::to_main_stage_empty()
    {
        main_stage = server_main_stage::empty;
    }

    void server_client_fsm::to_main_stage_loading()
    {
        main_stage = server_main_stage::loading;
    }

    void server_client_fsm::to_main_stage_active()
    {
        main_stage = server_main_stage::active;
    }

    /*
        All connection requests must be processed by the server itself, 
        and only after acceptance, open it.
    */


    server_client_fsm::server_client_fsm(
        void* ctx,
        base_fsm_callbacks cmn_callbacks,
        server_fsm_callbacks sv_callbacks
    ) 
        : base_fsm{ ctx, cmn_callbacks }
        , sv_callbacks{sv_callbacks}
    {
        pure_reset();
    }

    
    void server_client_fsm::on_disconnect(disconnect_type type, const disconnect_reason& reason)
    {
        if (main_stage == server_main_stage::empty)
        {
            error(fsm_error{ fsm_error_type::unexpected_packet });
            return;
        }

        to_main_stage_empty();
        call_disconnected(type, reason);
    }


    void server_client_fsm::on_reset()
    {
        pure_reset();
    }

    void server_client_fsm::on_tick(){}
    

	void server_client_fsm::on_recv_loading(const packet_header& header, protocol_reader& reader){}
    
	void server_client_fsm::on_recv_active(const packet_header& header, protocol_reader& reader){}
    
    bool server_client_fsm::send_conn_accept(bool need_file_sync)
    {
        packet_conn_accept conn_accept
        {
            .server_nonce = my_nonce,
            .need_file_sync = need_file_sync
        };

        return send_packet_generic(
            true, 
            packet_type::conn_accept, 
            0, 
            [&](protocol_writer& w) -> bool
            {
                return write_packet_struct(w, conn_accept);
            }
        );
    }

    void server_client_fsm::open(uint64_t cl_nonce, uint64_t sv_nonce, bool need_file_sync)
    {
        if (main_stage != server_main_stage::empty)
        {
            error(fsm_error{fsm_error_type::unexpected_signal});
            return;
        }

        remote_nonce = cl_nonce;
        my_nonce     = sv_nonce;

        // TODO: need to fix, 
        // now it's only for `active` stage test
        // loading stage is too hard now
        to_main_stage_active();
        send_conn_accept(need_file_sync);
    }

    
    bool server_client_fsm::reject(uint64_t timestamp_us, uint64_t cl_nonce, const reject_reason& reason, int max_size, int& out_size, void* out_data)
    {
        packet_conn_reject conn_reject{ .reason = reason };

        return pack_packet_to_buffer(
            max_size,
            out_data,
            out_size,
            timestamp_us,
            packet_type::conn_reject,
            0,
            cl_nonce,
            [&](protocol_writer& w) {
                return write_packet_struct(w, conn_reject);
            }
        );
    }

    
	bool server_client_fsm::is_conn_req(bool& bad_version, uint64_t& cl_nonce, int size, const void* data)
    {
        protocol_reader reader{ 0, nullptr };
        packet_header header;

        if (!parse_and_validate_packet(size, data, NO_NONCE, header, reader))
            return false;

        bad_version = header.protocol_version != VERSION;

        packet_conn_req conn_req;
        if (!read_packet_struct(reader, conn_req))
            return false;

        cl_nonce = conn_req.client_nonce;

        return true;
    }
    
    void server_client_fsm::disconnect(disconnect_type type, const disconnect_reason& reason)
    {
        if (main_stage == server_main_stage::empty)
        {
            error(fsm_error{fsm_error_type::unexpected_signal});
            return;
        }

        to_main_stage_empty();
        send_disconnect(type, reason);
    }


    void server_client_fsm::pure_reset()
    {
        main_stage         = server_main_stage          ::empty;
        loading_stage      = server_loading_stage       ::idle;
        file_sync_stage    = server_file_sync_stage     ::idle;
        gamesync_stage     = server_level_sync_stage    ::idle;
        snapshot_sync_stage= server_snapshot_sync_stage ::idle;
    }


    void server_client_fsm::on_recv_custom(const packet_header& header, protocol_reader& reader)
    {
        using enum server_main_stage;
        switch (main_stage)
        {
        case loading:       on_recv_loading(header, reader); break;
        case active:        on_recv_active (header, reader); break;
        default:
            error(fsm_error{ fsm_error_type::unknown_stage });
            break;
        }
    }
}