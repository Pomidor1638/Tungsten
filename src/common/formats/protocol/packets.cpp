
#include "packets.h"
#include <cstring>

#define LITTLE_ENDIAN
//#define BIG_ENDIAN

namespace tungsten::protocol
{    
    // swap to native endian
    uint16_t swap16(uint16_t value)
    {
        return
            ((value & 0x00ffu) << 8) |
            ((value & 0xff00u) >> 8);
    }
        
    uint32_t swap32(uint32_t value)
    {
        return 
            ((value & 0x000000ffu) << 24) | 
            ((value & 0x0000ff00u) <<  8) |
            ((value & 0x00ff0000u) >>  8) |
            ((value & 0xff000000u) >> 24);
    }
    
    uint64_t swap64(uint64_t value)
    {
        return 
            ((value & 0x00000000000000ffull) << 56) |
            ((value & 0x000000000000ff00ull) << 40) |
            ((value & 0x0000000000ff0000ull) << 24) |
            ((value & 0x00000000ff000000ull) <<  8) |
            ((value & 0x000000ff00000000ull) >>  8) |
            ((value & 0x0000ff0000000000ull) >> 24) |
            ((value & 0x00ff000000000000ull) >> 40) |
            ((value & 0xff00000000000000ull) >> 56);
    }
   
#if !defined(LITTLE_ENDIAN) && defined (BIG_ENDIAN)
    
    uint16_t to_native16(uint16_t protocol)
    {
        return protocol;
    }
    uint32_t to_native32(uint32_t protocol)
    {
        return protocol;
    }
    uint64_t to_native64(uint64_t protocol)
    {
        return protocol;
    }
    
    uint16_t to_protocol16(uint16_t native)
    {
        return native;
    }
    uint32_t to_protocol32(uint32_t native)
    {
        return native;
    }
    uint64_t to_protocol64(uint64_t native)
    {
        return native;
    }

#elif defined(LITTLE_ENDIAN) && !defined (BIG_ENDIAN)

    uint16_t to_native16(uint16_t protocol)
    {
        return swap16(protocol);
    }
    uint32_t to_native32(uint32_t protocol)
    {
        return swap32(protocol);
    }
    uint64_t to_native64(uint64_t protocol)
    {
        return swap64(protocol);
    }

    uint16_t to_protocol16(uint16_t native)
    {
        return swap16(native);
    }
    uint32_t to_protocol32(uint32_t native)
    {
        return swap32(native);
    }
    uint64_t to_protocol64(uint64_t native)
    {
        return swap64(native);
    }
    
#else 
    static_assert(false, "Unknown Target Machine Endianess");
#endif


    void packet_header_to_native(packet_header& protocol)
    {
        protocol.timestamp_us      = to_native64(protocol.timestamp_us);
        protocol.protocol_version  = to_native16(protocol.protocol_version);
        protocol.receiver_nonce    = to_native64(protocol.receiver_nonce);
        protocol.header_size       = to_native16(protocol.header_size);
        protocol.payload_size      = to_native16(protocol.payload_size);
        protocol.checksum          = to_native64(protocol.checksum);
    }

    void packet_header_to_protocol(packet_header& native)
    {
        native.timestamp_us      = to_protocol64(native.timestamp_us);
        native.protocol_version  = to_protocol16(native.protocol_version);
        native.receiver_nonce    = to_protocol64(native.receiver_nonce);
        native.header_size       = to_protocol16(native.header_size);
        native.payload_size      = to_protocol16(native.payload_size);
        native.checksum          = to_protocol64(native.checksum);
    }


    packet make_packet(
        uint64_t    timestamp_us,
        packet_type type,
        uint8_t     flags,
        uint64_t    receiver_nonce,
        uint16_t    payload_size,
        const void* payload
    ) {
        
        packet out;
        memset(&out, 0, sizeof(out));
        
        if (/*payload_size >= 0 && */payload_size <= MAX_PACKET_PAYLOAD_SIZE)
        {
            packet_header& header = out.header;

            header.timestamp_us     = timestamp_us;
            header.type             = type;
            header.flags            = flags;
            header.protocol_version = VERSION;
            header.header_size      = PACKET_HEADER_SIZE;
            header.receiver_nonce   = receiver_nonce;
            header.payload_size     = payload_size;
            //header.checksum         = 0;

            memcpy(header.magic, MAGIC, MAGIC_SIZE);

            if (payload_size && payload)
                memcpy(out.payload, payload, payload_size);

            make_checksum(out);
        }
        
        return out;
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

	bool validate_checksum(uint64_t checksum, int size, const void* data)
    {
        return true;
    }

	bool make_checksum(packet& p)
    {
        return true;
    }

	bool validate_all_packet(const packet& p, uint64_t nonce)
    {
        return validate_protocol_version(p.header)
            // because parse_packet do it
            //&& validate_packet_sizes(p.header)
            //&& validate_magic(p.header)
            && validate_nonce(p.header, nonce)
            && validate_checksum(p.header.checksum, p.header.payload_size, p.payload);
    }

    
	bool parse_packet(packet& out, int size, const void* data)
    {
        if (!data || size < PACKET_HEADER_SIZE || size > MAX_PACKET_SIZE)
            return false;

        memcpy(&out, data, size);
        packet_header_to_native(out.header);

        if (!validate_magic(out.header) || !validate_packet_sizes(out.header))
            return false;

        int expected_size = static_cast<int>(out.header.header_size) + static_cast<int>(out.header.payload_size);

        return (expected_size == size);
    }

    
	bool check_range(int total, int min, int max)
    {
        return total >= min && total <= max;
    }

}

