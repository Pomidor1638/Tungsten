#pragma once

#include <cstdint>
#include <cstddef>

#include "../protocol.h"

namespace tungsten::protocol
{
    // --- Protocol Constants ---
    constexpr char      MAGIC[] = "TUNGSTEN 1.0b";
    constexpr size_t    MAGIC_SIZE = sizeof(MAGIC) - 1;
    constexpr uint16_t  VERSION = 0x0001;

    // --- Generic Packet Structure ---

    /*

        TODO:
            Need to determine layout:

            - struct packet gurantee 1 alignment
            - packet is alias to uint8_t[MAX_PACKET_SIZE]

    */



    // --- Protocol Enums ---
    enum PACKET_FLAGS : uint8_t
    {
        PACKET_NONE_FLAG = 0,
        PACKET_ENCRYPTED = 1 << 0,
        PACKET_COMPRESSED = 1 << 1,
        //PACKET_RELIABLE     = 1 << 2,
    };

    enum class packet_type : uint8_t
    {
        none = 0,

        transfer,
        error,

        // connection stage
        conn_req,
        conn_accept,
        conn_reject,

        // file_sync stage
        sv_file_manifest,
        cl_need_manifest,

        // level_sync stage
        // sv_levelstate,
        // cl_snapshot_ack,
        // cl_ready,
        
        // active stage
        sv_snapshot,
        cl_usercmd,

        //message_req,
        //message_ack,

        cl_status_req,
        sv_status_ack,

        /*
            TODO:
                define of disconnect req/ack:
                1. disconnect without acknowledge           P.S. <- this one
                2. disconnect always with ack.
                3. disconnect particially with ack. (like cancel oro smthng)
        */

        disconnect,
    };



    // --- Packet Network Header ---
    struct packet_header
    {
        uint64_t    timestamp_us;
        char        magic[MAGIC_SIZE];
        uint16_t    protocol_version;
        uint64_t    receiver_nonce;
        uint16_t    header_size;
        uint16_t    payload_size;
        packet_type type;
        uint8_t     flags;
        uint64_t    checksum;
    };

    constexpr int PACKET_HEADER_SIZE =
        sizeof(packet_header::timestamp_us)
        + sizeof(packet_header::magic)
        + sizeof(packet_header::protocol_version)
        + sizeof(packet_header::receiver_nonce)
        + sizeof(packet_header::header_size)
        + sizeof(packet_header::payload_size)
        + sizeof(packet_header::type)
        + sizeof(packet_header::flags)
        + sizeof(packet_header::checksum);

    constexpr int MAX_PACKET_PAYLOAD_SIZE = MAX_PACKET_SIZE - PACKET_HEADER_SIZE;


    // --- Concrete Packet Payloads ---

    // --- base packets ---
    struct packet_error_header
    {
        protocol_error_type code;
    };

    struct packet_disconnect
    {
        disconnect_type   type;
        disconnect_reason reason;
    };

    // --- connection stage packets ---
    struct packet_conn_req
    {
        uint64_t client_nonce;
    };

    struct packet_conn_accept
    {
        uint64_t server_nonce;
        uint8_t  need_file_sync;
    };

    struct packet_conn_reject
    {
        reject_reason reason;
    };

    // --- loading stage packets ---

    // ------ file sync stage ------


    /*
        [packet_sv_file_manifest_header]
        [file_info] for (packet_sv_file_manifest_header.files_count)

        there is no data, only names and checksums
    */

    struct packet_sv_file_manifest_header
    {
        uint64_t total_size;
        uint64_t files_count;
    };

    /*
        [cl_file_manifest_header]
        [flag bits], for (total_count)
    */

    struct packet_cl_file_manifest_header
    {
        uint64_t  total_count;
        uint64_t missed_count;
    };

    // --- active stage packets ---

    // cl_usercmd and sv_snapshot shares the same struct, 
    // usercmd - from clients, snapshot - from server

    // [packet_snapshot_header][snapshot's payload]
    
    struct packet_snapshot_header 
    {
        uint64_t total_size;    
        uint64_t last_snapshot; // or last usercmd
    };


    // --- Public API Functions ---

    // Factory method to build network packets safely

    bool make_packet_header
    (
        packet_header& out,
        uint64_t       timestamp_us,
        packet_type    type,
        uint8_t        flags,
        uint64_t       receiver_nonce
    );

    bool update_header_payload(packet_header& header, int payload_size, const void* payload);


    // packet_header
    bool read_packet_struct(protocol_reader& reader, packet_header& header);
    bool write_packet_struct(protocol_writer& writer, const packet_header& header);

    // packet_conn_req
    bool read_packet_struct(protocol_reader& reader, packet_conn_req& value);
    bool write_packet_struct(protocol_writer& writer, const packet_conn_req& value);

    // packet_conn_accept
    bool read_packet_struct(protocol_reader& reader, packet_conn_accept& value);
    bool write_packet_struct(protocol_writer& writer, const packet_conn_accept& value);

    // packet_conn_reject
    bool read_packet_struct(protocol_reader& reader, packet_conn_reject& value);
    bool write_packet_struct(protocol_writer& writer, const packet_conn_reject& value);

    // packet_error
    bool read_packet_struct(protocol_reader& reader, packet_error_header& value);
    bool write_packet_struct(protocol_writer& writer, const packet_error_header& value);

    // packet_disconnect_req
    bool read_packet_struct(protocol_reader& reader, packet_disconnect& value);
    bool write_packet_struct(protocol_writer& writer, const packet_disconnect& value);

    // file_info
    bool read_packet_struct(protocol_reader& reader, file_info& value);
    bool write_packet_struct(protocol_writer& writer, const file_info& value);

    // sv_file_manifest_header
    bool read_packet_struct(protocol_reader& reader, packet_sv_file_manifest_header& value);
    bool write_packet_struct(protocol_writer& writer, const packet_sv_file_manifest_header& value);

    // cl_file_manifest_header
    bool read_packet_struct(protocol_reader& reader, packet_cl_file_manifest_header& value);
    bool write_packet_struct(protocol_writer& writer, const packet_cl_file_manifest_header& value);



    // base validate
    bool validate_header(const packet_header& header, uint64_t nonce, int payload_size, const void* payload);

    template <typename F>
    bool pack_packet_to_buffer(
        int         max_size,
        void* buffer,
        int& out_total_size,
        uint64_t    timestamp_us,
        packet_type type,
        uint8_t     flags,
        uint64_t    nonce,
        F&& write_payload_func
    ) {
        if (!buffer || max_size < PACKET_HEADER_SIZE)
            return false;

        protocol_writer writer{ static_cast<size_t>(max_size), buffer };
        if (!writer.seek(PACKET_HEADER_SIZE))
            return false;

        const void* payload = writer.peek();
        if (!write_payload_func(writer))
            return false;

        out_total_size = static_cast<int>(writer.tell());
        int payload_size = out_total_size - PACKET_HEADER_SIZE;

        packet_header header;
        if (!make_packet_header(header, timestamp_us, type, flags, nonce))
            return false;

        if (!update_header_payload(header, payload_size, payload))
            return false;

        writer.seek(0);
        return write_packet_struct(writer, header);
    }

    bool parse_and_validate_packet(int size, const void* data, uint64_t expected_nonce, packet_header& out_header, protocol_reader& out_reader);
}
