
#include "protocol.h"
#include <algorithm>
#include <cstring>
#include <cstddef>

namespace tungsten::protocol
{
    bool make_packet(
        packet& out,
        uint64_t timestamp_us,
        packet_type type,
        int payload_size,
        const void* payload,
        uint8_t flags,
        uint64_t receiver_nonce
    ) {
        if (payload_size < 0 || payload_size > MAX_PACKET_PAYLOAD_SIZE)
            return false;

        if (payload_size > 0 && !payload)
            return false;

        memset(&out, 0, sizeof(out));

        packet_header& header = out.header;

        header.timestamp_us = timestamp_us;
        header.type = type;
        header.flags = flags;
        header.protocol_version = VERSION;
        header.header_size = PACKET_HEADER_SIZE;
        header.receiver_nonce = receiver_nonce;
        header.payload_size = static_cast<uint16_t>(payload_size);
        header.checksum = 0;

        memcpy(header.magic, MAGIC, MAGIC_SIZE);

        if (payload_size)
            memcpy(out.payload, payload, payload_size);

        make_checksum(out);

        return true;
    }

    
    bool validate_protocol_version(const packet_header& header)
    {
        return header.protocol_version == VERSION;
    }
    
	bool validate_packet_sizes(const packet_header& header)
    {
        return header.header_size == PACKET_HEADER_SIZE && header.payload_size <= MAX_PACKET_PAYLOAD_SIZE;
    }
    
	bool validate_magic(const packet_header& header)
    {
        return !memcmp(header.magic, MAGIC, MAGIC_SIZE);
    }

	bool validate_nonce(const packet_header& header, uint64_t nonce)
    {
        return header.receiver_nonce == nonce;
    }

    /*
        TODO: checksum
    */

	bool validate_checksum(const packet& p)
    {
        return true;
    }
	bool make_checksum(packet& p)
    {
        return true;
    }

	bool validate_packet(const packet& p, uint64_t nonce)
    {
        return validate_protocol_version(p.header)
            && validate_packet_sizes(p.header)
            && validate_magic(p.header)
            && validate_nonce(p.header, nonce)
            && validate_checksum(p);
    }

    
	bool parse_packet(packet& out, size_t size, const void* data)
    {
        if (!data || size < PACKET_HEADER_SIZE || size > MAX_PACKET_SIZE)
            return false;

        const auto* bytes = static_cast<const uint8_t*>(data);

        packet_header header{};
        memcpy(&header, bytes, PACKET_HEADER_SIZE);

        if (!validate_packet_sizes(header))
            return false;

        const size_t expected_size =
            static_cast<size_t>(header.header_size) +
            static_cast<size_t>(header.payload_size);

        if (expected_size != size)
            return false;

        memset(&out, 0, sizeof(out));

        out.header = header;

        if (header.payload_size)
        {
            memcpy(
                out.payload,
                bytes + header.header_size,
                header.payload_size
            );
        }

        return true;
    }
}