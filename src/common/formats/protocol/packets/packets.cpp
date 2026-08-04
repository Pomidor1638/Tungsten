
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

    bool update_header_payload(packet_header& header, int payload_size, const void* payload)
    {
        if (payload_size < 0 || payload_size > MAX_PACKET_PAYLOAD_SIZE)
            return false;

        header.checksum = 0; // TODO: need to do checksum funcs

        header.payload_size = static_cast<uint16_t>(payload_size);
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
        if (!reader.can_advance(PACKET_HEADER_SIZE))
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
    bool read_packet_struct(protocol_reader& reader, packet_error_header& error)
    {
        return reader.read(*reinterpret_cast<uint8_t*>(&error.code));
    }
    bool write_packet_struct(protocol_writer& writer, const packet_error_header& error)
    {
        return writer.write(static_cast<uint8_t>(error.code));
    }

    // packet_disconnect_req
    bool read_packet_struct(protocol_reader& reader, packet_disconnect& disconnect_req)
    {
        if (!reader.read(*reinterpret_cast<uint8_t*>(&disconnect_req.type)))
            return false;

        return read_packet_struct(reader, disconnect_req.reason);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_disconnect& disconnect_req)
    {
        if (!writer.write(static_cast<uint8_t>(disconnect_req.type)))
            return false;

        return write_packet_struct(writer, disconnect_req.reason);
    }

    // file_info
    bool read_packet_struct(protocol_reader& reader, file_info& value)
    {
        if (!reader.read(value.checksum))
            return false;
        if (!reader.read(value.size))
            return false;
        return read_packet_struct(reader, value.name);
    }
    bool write_packet_struct(protocol_writer& writer, const file_info& value)
    {
        if (!writer.write(value.checksum))
            return false;
        if (!writer.write(value.size))
            return false;
        return write_packet_struct(writer, value.name);
    }

    // sv_file_manifest_header
    bool read_packet_struct(protocol_reader& reader, packet_sv_file_manifest_header& value)
    {
        if (!reader.read(value.total_size))
            return false;
        return reader.read(value.files_count);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_sv_file_manifest_header& value)
    {
        if (!writer.write(value.total_size))
            return false;
        return writer.write(value.files_count);
    }

    // cl_file_manifest_header
    bool read_packet_struct(protocol_reader& reader, packet_cl_file_manifest_header& value)
    {
        if (!reader.read(value.total_count))
            return false;
        return reader.read(value.missed_count);
    }
    bool write_packet_struct(protocol_writer& writer, const packet_cl_file_manifest_header& value)
    {
        if (!writer.write(value.total_count))
            return false;
        return writer.write(value.missed_count);
    }


    bool validate_header(const packet_header& header, uint64_t nonce, int payload_size, const void* payload)
    {
        if (header.payload_size > 0 && !payload)
            return false;   

        if (
            memcmp(header.magic, MAGIC, MAGIC_SIZE) != 0
            || header.header_size != PACKET_HEADER_SIZE
            || header.payload_size > MAX_PACKET_PAYLOAD_SIZE
            || header.receiver_nonce != nonce
            || static_cast<int>(header.payload_size) > payload_size
        ) {
            return false;
        }

        return header.checksum == 0;
    }

    bool parse_and_validate_packet(int size, const void* data, uint64_t expected_nonce, packet_header& out_header, protocol_reader& out_reader)
    {
        if (!data || size < PACKET_HEADER_SIZE || size > MAX_PACKET_SIZE)
            return false;

        out_reader = protocol_reader{ static_cast<size_t>(size), data };

        if (!read_packet_struct(out_reader, out_header))
            return false;

        if (!validate_header(out_header, expected_nonce, out_reader.remaining(), out_reader.peek()))
            return false;

        return true;
    }
}
