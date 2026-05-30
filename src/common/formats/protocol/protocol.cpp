
#include <cstring>
#include "protocol.h"

namespace tungsten::protocol 
{
    void packet_builder::set_tick(uint32_t value)
    {
        tick = value;
    }

    void packet_builder::set_timestamp(uint64_t value)
    {
        timestamp_us = value;
    }

    uint32_t packet_builder::get_sequence() const
    {
        return sequence;
    }

    packet_header packet_builder::make_header(packet_type type, uint16_t payload_size)
    {
        packet_header header{};

        memcpy(header.magic, MAGIC, MAGIC_SIZE);
        header.timestamp_us     = timestamp_us;
        header.sequence         = sequence++;
        header.tick             = tick;
        header.protocol_version = VERSION;
        header.header_size      = sizeof(packet_header);
        header.payload_size     = payload_size;
        header.type             = type;
        header.flags            = static_cast<uint8_t>(packet_flags::none);
        header.checksum         = 0;

        return header;
    }

    
	packet_conn_req packet_builder::build_conn_req()
    {
        return packet_conn_req 
        {
            .client_protocol_version = VERSION,
            .client_nonce = timestamp_us
        };
    }

    
	packet_conn_ack packet_builder::build_conn_ack()
    {
        return packet_conn_ack{};
    }

    bool packet_builder::build(packet& out, packet_type type, const void* payload, uint16_t payload_size)
    {
        if (payload_size > MAX_PACKET_PAYLOAD_SIZE)
            return false;

        if (!payload && payload_size != 0)
            return false;

        out = {};
        out.header = make_header(type, payload_size);

        if (payload_size != 0)
            memcpy(out.payload, payload, payload_size);

        return true;
    }

    bool packet_builder::validate_header(const packet_header& header)
    {
        return !memcmp(header.magic, MAGIC, MAGIC_SIZE)
            && header.protocol_version == VERSION
            && header.header_size == sizeof(packet_header)
            && header.payload_size <= MAX_PACKET_PAYLOAD_SIZE;
    }

    bool packet_builder::validate_packet(const packet& packet)
    {
        return validate_header(packet.header);
    }
    
}
