
#include "protocol.h"
#include <algorithm>
#include <cstring>
#include <cstddef>

namespace tungsten::protocol
{
    packet make_packet
    (
        uint64_t timestamp_us,
        packet_type type,
        int payload_size,
        void* payload,
        uint8_t flags,
        uint64_t nonce
    ) {
        packet p;
        packet_header& header = p.header;

        header.timestamp_us = timestamp_us;
        header.type = type;
        header.flags = flags;
        header.protocol_version = VERSION;
        header.header_size = PACKET_HEADER_SIZE;
        header.nonce = nonce;
        header.checksum = 0;

        memcpy(header.magic, MAGIC, MAGIC_SIZE);
        header.payload_size = std::clamp<uint16_t>(payload_size, 0, MAX_PACKET_PAYLOAD_SIZE);
        
        if (payload)
            memcpy(p.payload, payload,  header.payload_size);

        memset(p.payload, 0, MAX_PACKET_PAYLOAD_SIZE - header.payload_size);
        
        return p;
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
        return header.nonce == nonce;
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


        if (!validate_packet_sizes(out.header))
            return false;

        size_t expected_size = out.header.header_size + out.header.payload_size;

        if (expected_size != size)
            return false;
        
        // memset(&out, 0, sizeof(out));
        memcpy(&out, data, size);

        return true;
    }
}