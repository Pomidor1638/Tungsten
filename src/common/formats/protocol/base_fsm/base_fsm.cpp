

#include "base_fsm.h"

#include "../protocol.h"
#include "../packets/packets.h"
#include "../../../platform/memory/memory.h"

namespace tungsten::protocol 
{

    
    
    base_fsm::base_fsm(void* ctx, timeout_config cfg, base_fsm_callbacks callbacks)
        : context{ctx}
        , timeout_cfg{ cfg }
        , cmn_callbacks{callbacks}
        , curr_timestamp_us{0}
    {
        pure_reset();
    }
    
    bool base_fsm::call_disconnected(disconnect_type type, const disconnect_reason& reason)
    {
        auto& on_disconnected = cmn_callbacks.on_disconnected;
        if (!on_disconnected)
            return false;
        on_disconnected(context, type, reason);
        return true;
    }
    
    bool base_fsm::call_error(const fsm_error& err)
    {
        auto& on_error = cmn_callbacks.on_error;
        if (!on_error)
            return false;
        on_error(context, err);
        return true;
    }

    bool base_fsm::call_send(const data_span& data)
    {
        auto& on_send = cmn_callbacks.on_send;
        if (!on_send)
            return false;
        on_send(context, data);
        return true;
    }
    
	bool base_fsm::check_timeout()
    {
        if (!timeout_tracking)
            return true;

        if (curr_timestamp_us - last_recv_timestamp_us > timeout_cfg.interval_us)
        {
            timeout();
            return false;
        }

        return true;
    }

	void base_fsm::reset_timeout()
    {
        last_recv_timestamp_us  = curr_timestamp_us;
    }

    
    void base_fsm::timeout()
    {
        fatal_error(fsm_error{.type = fsm_error_type::remote_timeout});
    }

    void base_fsm::reset_timing()
    {
        curr_delta_us       = 0; 
    }

    void base_fsm::pure_reset()
    {
        reset_timing();
        reset_timeout();
        disarm_timeout();
    }   

    void base_fsm::reset()
    {
        pure_reset();
        on_reset();
    }

    void base_fsm::update_timing(uint64_t delta_us)
    {
        curr_delta_us      =      delta_us;
        curr_timestamp_us += curr_delta_us;
    }

    void base_fsm::tick(uint64_t delta_us)
    {
        update_timing(delta_us);
		if (check_timeout())
        {
            on_tick();
        }
    }

    void base_fsm::set_context(void* ctx)
    {
        context = ctx;
    }

    bool base_fsm::make_packet_header_internal(packet_header& out, packet_type type, uint8_t flags) 
    {
        if (!make_packet_header(out, get_timestamp_us(), type, flags, remote_nonce))
        {
            fatal_error(fsm_error{fsm_error_type::make_violation});
            return false;
        }

        return true;
    }

    
    void base_fsm::arm_timeout()
    {
        reset_timeout();
        timeout_tracking = true;
    }

    void base_fsm::disarm_timeout()
    {
        timeout_tracking = false;
    }
    
    void base_fsm::touch_timeout()
    {
        reset_timeout();
    }

    bool base_fsm::get_timeout_tracking() const
    {
        return timeout_tracking;
    }

    bool base_fsm::send_protocol_error(protocol_error type)
    {
        packet_error err{ .code = type };
        packet_header header;
        packet_t send_packet;

        protocol_writer writer { sizeof(send_packet), &send_packet };

        if (!writer.seek(PACKET_HEADER_SIZE))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet });
            return false;
        }

        if (!write_packet_struct(writer, err))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet });
            return false;
        }

        size_t payload_size = writer.tell() - PACKET_HEADER_SIZE;
        size_t total_packet_size = writer.tell();

        if (!make_packet_header_internal(header, packet_type::protocol_error, 0))
        {
            return false;
        }

        make_packet_payload(header, static_cast<uint16_t>(payload_size), &send_packet[PACKET_HEADER_SIZE]);
        
        writer.seek(0);
        if (!write_packet_struct(writer, header))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet });
            return false;
        }

        return call_send(data_span{ total_packet_size, send_packet });
    }

    

    void base_fsm::error(fsm_error err, bool send_err)
    {
        if (send_err)
        {
            if (err.protocol_err == protocol_error::none)
            {
                fatal_error(fsm_error{fsm_error_type::internal_violation});
                return;
            }

            if(!send_protocol_error(err.protocol_err))
                return;
        }
        fatal_error(err);
    }

    void base_fsm::fatal_error(fsm_error err)
    {
        call_error(err);
        reset();
    }

    bool base_fsm::on_recv_error(const packet_header& header, protocol_reader& reader)
    {
        packet_error err;

        if (!read_packet_struct(reader, err))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet, protocol_error::corrupted_packet });
            return false;
        }

        fatal_error(fsm_error{fsm_error_type::remote_violation, err.code});
        return true;
    }

    
    bool base_fsm::on_recv_disconnect_ack(const packet_header& header, protocol_reader& reader)
    {
        return on_disconnect_ack();
    }

    bool base_fsm::on_recv_disconnect_req(const packet_header& header, protocol_reader& reader)
    {
        packet_disconnect_req disconnect_req;
        if (!read_packet_struct(reader, disconnect_req))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet, protocol_error::corrupted_packet });
            return false;
        }

        return on_disconnect_req(disconnect_req.type, disconnect_req.reason);
    }

    void base_fsm::on_recv(const data_span& data)
    {
        if (!data.data() || data.size() > MAX_PACKET_SIZE)
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet });
            return;
        }

        protocol_reader reader{ data.size(), data.data() };
        packet_header header;

        if (!read_packet_struct(reader, header))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet });
            return;
        }

        if (!validate_header(header, reader.remaining(), reader.peak()))
        {
            fatal_error(fsm_error{ fsm_error_type::corruted_packet });
            return;
        }

        bool processed = false;
        using enum packet_type;

        switch (header.type)
        {
        case protocol_error:
            processed = on_recv_error(header, reader);
            break;
        case disconnect_ack:
            processed = on_recv_disconnect_ack(header, reader);
            break;
        case disconnect_req:
            processed = on_recv_disconnect_req(header, reader);
            break;
        default:
            processed = on_recv_custom(header, reader);
            break;
        }

        if (!processed)
        {
            fatal_error(fsm_error{ fsm_error_type::unexpected_packet });
            return;
        }

        if (timeout_tracking)
        {
            reset_timeout();
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
    void base_fsm::set_common_callbacks(base_fsm_callbacks callbacks)
    {
        cmn_callbacks = callbacks;
    }
    void base_fsm::set_timeout_config(timeout_config cfg)
    {
        timeout_cfg = cfg;
    }
}