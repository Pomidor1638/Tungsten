
#include "cl.h"
#include "../packets/packets.h"

namespace tungsten::protocol
{

    client_fsm::client_fsm(
        void* ctx,
        base_fsm_callbacks cmn_callbacks,
        client_fsm_callbacks cl_callbacks
    )
        : base_fsm{ ctx, cmn_callbacks }
        , cl_callbacks{ cl_callbacks }
    {
        pure_reset();
    }

    void client_fsm::on_reset()
    {
        pure_reset();
    }


    void client_fsm::set_client_callbacks(client_fsm_callbacks callbacks)
    {
        cl_callbacks = callbacks;
    }

    void client_fsm::on_tick()
    {
    }


    void client_fsm::disconnect(disconnect_type type, const disconnect_reason& reason)
    {
        using enum client_main_stage;
        switch (main_stage)
        {
        case connecting:
        case loading:
        case active:
            break;
        case disconnected:
        default:
            error(fsm_error{ fsm_error_type::unexpected_signal });
            // because error() does reset()
            // to_stage_disconnected();
            return;
        }

        to_main_stage_disconnected();
        send_disconnect(type, reason);
    }

    void client_fsm::on_disconnect(disconnect_type type, const disconnect_reason& reason)
    {
        if (main_stage == client_main_stage::disconnected)
        {
            error(fsm_error{ fsm_error_type::unexpected_packet });
            return;
        }

        to_main_stage_disconnected();
        call_disconnected(type, reason);
    }

    void client_fsm::pure_reset()
    {
        main_stage          = client_main_stage      ::disconnected;
        loading_stage       = client_loading_stage   ::idle;
        file_sync_stage     = client_file_sync_stage ::idle;
        level_sync_stage    = client_level_sync_stage::idle;
    }

    bool client_fsm::call_connection_accepted(bool need_file_sync)
    {
        if (cl_callbacks.on_conn_accepted)
            cl_callbacks.on_conn_accepted(get_context(), need_file_sync);
        return true;
    }

    bool client_fsm::call_connection_rejected(const reject_reason& reason)
    {
        if (cl_callbacks.on_conn_rejected)
            cl_callbacks.on_conn_rejected(get_context(), reason);
        return true;
    }

    void client_fsm::process_conn_accept(const packet_header& header, protocol_reader& reader)
    {
        packet_conn_accept conn_accept;
        if (!read_packet_struct(reader, conn_accept))
        {
            error(fsm_error{ .type = fsm_error_type::corruted_packet });
            return;
        }

        remote_nonce = conn_accept.server_nonce;

        to_main_stage_active();
        call_connection_accepted(conn_accept.need_file_sync);
    }

    void client_fsm::process_conn_reject(const packet_header& header, protocol_reader& reader)
    {
        packet_conn_reject conn_reject;
        if (!read_packet_struct(reader, conn_reject))
        {
            error(fsm_error{ .type = fsm_error_type::corruted_packet });
            return;
        }

        to_main_stage_disconnected();
        call_connection_rejected(conn_reject.reason);
    }

    void client_fsm::on_recv_connecting(const packet_header& header, protocol_reader& reader)
    {
        switch (header.type)
        {
        case packet_type::conn_accept: process_conn_accept(header, reader); break;
        case packet_type::conn_reject: process_conn_reject(header, reader); break;
        default:
            send_protocol_error(protocol_error::unexcepted_packet);
            error(fsm_error{ fsm_error_type::unexpected_packet, });
            break;
        }
    }


    void client_fsm::on_recv_loading(const packet_header& header, protocol_reader& reader)
    {
    }

    bool client_fsm::send_conn_req()
    {
        packet_conn_req conn_req{ .client_nonce = my_nonce };

        return send_packet_generic(
            true,
            packet_type::conn_req,
            0,
            [&](protocol_writer& w)
            {
                return write_packet_struct(w, conn_req);
            }
        );
    }

    void client_fsm::on_recv_active(const packet_header& header, protocol_reader& reader)
    {
        switch (header.type)
        {
        default:
            break;
        }
    }


    void client_fsm::to_main_stage_disconnected()
    {
        reset();
        //main_stage = client_main_stage::disconnected;
    }
    void client_fsm::to_main_stage_connecting()
    {
        main_stage = client_main_stage::connecting;
    }
    void client_fsm::to_main_stage_loading()
    {
        main_stage = client_main_stage::loading;
    }
    void client_fsm::to_main_stage_active()
    {
        main_stage = client_main_stage::active;
    }

    void client_fsm::open(uint64_t nonce)
    {
        if (main_stage != client_main_stage::disconnected)
        {
            error(fsm_error{ fsm_error_type::unknown_signal });
            return;
        }

        my_nonce     = nonce;
        remote_nonce = NO_NONCE;

        to_main_stage_connecting();
        send_conn_req();
    }

    void client_fsm::on_recv_custom(const packet_header& header, protocol_reader& reader)
    {
        using enum client_main_stage;
        switch (main_stage)
        {
        case connecting: on_recv_connecting(header, reader); break;
        case loading:    on_recv_loading   (header, reader); break;
        case active:     on_recv_active    (header, reader); break;
        default:
            error(fsm_error{ fsm_error_type::unknown_stage });
            break;
        }
    }
}