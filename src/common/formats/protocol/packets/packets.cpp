
#include "packets.h"

#include <cstring>
#include <type_traits>

namespace tungsten::protocol
{

    bool make_packet_header(packet_header& out, uint64_t timestamp_us, packet_type type, uint8_t flags, uint64_t receiver_nonce)
    {
        memset(&out, 0, sizeof(out));
        memcpy(out.magic, MAGIC, MAGIC_SIZE);

        out.timestamp_us        = timestamp_us;
        out.type                = type;
        out.flags               = flags;
        out.protocol_version    = VERSION;
        out.header_size         = static_cast<uint16_t>(PACKET_HEADER_SIZE);
        out.receiver_nonce      = receiver_nonce;

        return true;
    }

    bool make_packet_payload(packet_header& header, uint16_t payload_size, const void* payload)
    {
        // TODO: need to do checksum funcs

        if (payload_size > MAX_PACKET_PAYLOAD_SIZE)
            return false;

        header.checksum = 0;
        header.payload_size = payload_size;

        return true;
    }

    template <uint32_t N>
    bool read_packet_struct(protocol_reader& reader, protocol_string<N>& str)
    {
        if (!reader.read(str.size))
            return false;

        if (str.size > N)
            return false;

        return reader.read_bytes(str.size, str.data);
    }

    template <uint32_t N>
    bool write_packet_struct(protocol_writer& writer, const protocol_string<N>& str)
    {

        if (str.size > N)
            return false;

        if (!writer.write(str.size))
            return false;

        return writer.write_bytes(str.size, str.data);
    }

    // packet_header
    bool read_packet_struct(protocol_reader& reader, packet_header& header)
    {
        if (!reader.can_advance(sizeof(header)))
            return false;

        reader.read(header.timestamp_us);
        reader.read_bytes(MAGIC_SIZE, header.magic);
        reader.read(header.protocol_version);
        reader.read(header.receiver_nonce);
        reader.read(header.header_size);
        reader.read(header.payload_size);
        reader.read(*reinterpret_cast<uint8_t*>(&header.type));
        reader.read(header.flags);
        reader.read(header.checksum);

        return true;
    }
    bool write_packet_struct(protocol_writer& writer, const packet_header& header)
    {
        if (!writer.can_advance(sizeof(header)))
            return false;

        writer.write(header.timestamp_us);
        writer.write_bytes(MAGIC_SIZE, header.magic);
        writer.write(header.protocol_version);
        writer.write(header.receiver_nonce);
        writer.write(header.header_size);
        writer.write(header.payload_size);
        writer.write(static_cast<uint8_t>(header.type));
        writer.write(header.flags);
        writer.write(header.checksum);

        return true;
    }

    // packet_conn_req
    bool read_packet_struct(protocol_reader& reader, packet_conn_req& conn_req)
    {
        return reader.read(conn_req.client_nonce);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_conn_req& conn_req)
    {
        return writer.write(conn_req.client_nonce);
    }

    // packet_conn_reject
    bool read_packet_struct(protocol_reader& reader, packet_conn_reject& conn_reject)
    {
        return read_packet_struct(reader, conn_reject.reason);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_conn_reject& conn_reject)
    {
        return write_packet_struct(writer, conn_reject.reason);
    }

    // packet_conn_reject
    bool read_packet_struct(protocol_reader& reader, packet_conn_accept& conn_accept)
    {
        if (!reader.read(conn_accept.server_nonce))
            return false;
        return reader.read(conn_accept.need_file_sync);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_conn_accept& conn_accept)
    {
        if (!writer.write(conn_accept.server_nonce))
            return false;
        return writer.write(conn_accept.need_file_sync);
    }


    // packet_error
    bool read_packet_struct(protocol_reader& reader, packet_error& error)
    {
        return reader.read(*reinterpret_cast<uint8_t*>(&error.code));
    }
    bool write_packet_struct(protocol_writer& writer, const packet_error& error)
    {
        return writer.write(static_cast<uint8_t>(error.code));
    }

    // packet_disconnect_req
    bool read_packet_struct(protocol_reader& reader, packet_disconnect_req& disconnect_req)
    {
        if (!reader.read(*reinterpret_cast<uint8_t*>(&disconnect_req.type)))
            return false;

        return read_packet_struct(reader, disconnect_req.reason);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_disconnect_req& disconnect_req)
    {
        if (!writer.write(static_cast<uint8_t>(disconnect_req.type)))
            return false;

        return write_packet_struct(writer, disconnect_req.reason);
    }

    // packet_disconnect_ack
    /*
    template <>
    bool read_packet_struct(protocol_reader& reader)
    {
        return true;
    }
    template <>
    bool write_packet_struct(protocol_writer& writer)
    {
        return true;
    }
    */


    bool validate_header(const packet_header& header, size_t payload_size, const void* payload)
    {

        if (header.payload_size > 0 && !payload)
        {
            return false;
        }

        if (memcmp(header.magic, MAGIC, MAGIC_SIZE) != 0
            || header.header_size != PACKET_HEADER_SIZE
            || header.payload_size > MAX_PACKET_PAYLOAD_SIZE
            || header.payload_size > payload_size) // maybe != ?
        {
            return false;
        }

        return header.checksum == 0;
    }


}
