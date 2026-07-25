

#include "base_fsm.h"

#include "../protocol.h"
#include "../packets/packets.h"
#include "../../../platform/memory/memory.h"

namespace tungsten::protocol
{



    base_fsm::base_fsm(void* ctx, base_fsm_callbacks callbacks)
        : context{ ctx }
        , cmn_callbacks{ callbacks }
        , curr_timestamp_us{ 0 }
    {
        pure_reset();
    }

    base_fsm::~base_fsm()
    {
    }

    void base_fsm::call_disconnected(disconnect_type type, const disconnect_reason& reason)
    {
        auto& on_disconnected = cmn_callbacks.on_disconnected;
        if (on_disconnected)
            on_disconnected(context, type, reason);
    }

    void base_fsm::call_error(const fsm_error& err)
    {
        auto& on_error = cmn_callbacks.on_error;
        if (on_error)
            on_error(context, err);
    }

    void base_fsm::call_send(bool reliable, int size, const void* data)
    {
        auto& on_send = cmn_callbacks.on_send;
        if (on_send)
            on_send(context, reliable, size, data);
    }

    void base_fsm::reset_timing()
    {
        curr_delta_us = 0;
    }

    void base_fsm::pure_reset()
    {
        reset_timing();
    }

    void base_fsm::reset()
    {
        pure_reset();
        on_reset();
    }

    void base_fsm::update_timing(uint64_t delta_us)
    {
        curr_delta_us = delta_us;
        curr_timestamp_us += curr_delta_us;
    }

    void base_fsm::tick(uint64_t delta_us)
    {
        update_timing(delta_us);
    }

    void base_fsm::set_context(void* ctx)
    {
        context = ctx;
    }

    void* base_fsm::get_context()
    {
        return context;
    }

    bool base_fsm::send_disconnect(disconnect_type type, const disconnect_reason& reason)
    {
        packet_disconnect d
        {
            .type   = type,
            .reason = reason
        };

        return send_packet_generic(
            true,
            packet_type::disconnect,
            0,
            [&](protocol_writer& w) -> bool
            {
                return write_packet_struct(w, d);
            }
        );
    }

    bool base_fsm::send_protocol_error(protocol_error type)
    {
        packet_error err{ .code = type };

        return send_packet_generic(
            true, 
            packet_type::error, 
            0, 
            [&](protocol_writer& w) 
            {
                return write_packet_struct(w, err);
            }
        );
    }



    void base_fsm::error(fsm_error err)
    {
        call_error(err);
        reset();
    }

    void base_fsm::on_recv_error(const packet_header& header, protocol_reader& reader)
    {
        packet_error err;

        if (!read_packet_struct(reader, err))
        {
            error(fsm_error{ fsm_error_type::corruted_packet });
            return;
        }

        error(fsm_error{ fsm_error_type::remote_violation, err.code });
    }


    void base_fsm::on_recv_disconnect(const packet_header& header, protocol_reader& reader)
    {
        packet_disconnect disconnect_req;
        if (!read_packet_struct(reader, disconnect_req))
        {
            error(fsm_error{ fsm_error_type::corruted_packet, protocol_error::corrupted_packet });
            return;
        }

        on_disconnect(disconnect_req.type, disconnect_req.reason);
    }

    void base_fsm::on_recv(int size, const void* data) // Возвращает void
    {
        protocol_reader reader{0, nullptr};
        packet_header header;

        if (!parse_and_validate_packet(size, data, my_nonce, header, reader))
        {
            error(fsm_error{ fsm_error_type::corruted_packet });
            return;
        }

        using enum packet_type;
        switch (header.type)
        {
        case error:      on_recv_error     (header, reader); break;
        case disconnect: on_recv_disconnect(header, reader); break;
        default:         on_recv_custom    (header, reader); break;
        }
    }


    uint64_t base_fsm::get_timestamp_us() const
    {
        return curr_timestamp_us;
    }
    uint64_t base_fsm::get_delta_us() const
    {
        return curr_delta_us;
    }
    void base_fsm::set_base_callbacks(base_fsm_callbacks callbacks)
    {
        cmn_callbacks = callbacks;
    }
}