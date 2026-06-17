
#include "transfer.h"
#include <cstring>

namespace tungsten::transfer
{
	bool make_packet(packet& out, pkt_type type, int size, const void* data)
    {
        packet_header& header = out.header;
        header.type = type;
		if (size > MAX_PACKET_PAYLOAD_SIZE || size < 0)
        {
            return false;
        }
        header.payload_size = size;
        if (data && size)
        {
            memcpy(out.payload, data, header.payload_size);
        }
        header.checksum = checksum_bytes(header.payload_size, out.payload);
        return true;
    }

    uint64_t checksum_bytes(size_t size, const void* data)
    {
        return 0;
    }

	bool validate_packet(const packet& p)
    {
        if (p.header.payload_size > MAX_PACKET_PAYLOAD_SIZE)
            return false;
        return p.header.checksum == checksum_bytes(p.header.payload_size, p.payload);    
    }

}