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

    // [packet_header][payload]
    using packet_t = uint8_t[MAX_PACKET_SIZE];


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
        protocol_error,

        // connection stage
        conn_req,
        // conn_cancel,
        conn_accept,
        conn_reject,

        // file_sync stage
        // sv_file_manifest,
        // cl_need_manifest,

        // level_sync stage
        // sv_levelstate,
        // sv_snapshot,
        // cl_snapshot_ack,
        // cl_ready,
        // cl_usercmd,
        // sv_usercmd_ack,

        cl_status_req,
        sv_status_ack,
        disconnect_req,
        disconnect_ack,

        // event_req,
        // event_ack,
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

    constexpr size_t PACKET_HEADER_SIZE =
          sizeof(packet_header::timestamp_us)
        + sizeof(packet_header::magic)
        + sizeof(packet_header::protocol_version)
        + sizeof(packet_header::receiver_nonce)
        + sizeof(packet_header::header_size)
        + sizeof(packet_header::payload_size)
        + sizeof(packet_header::type)
        + sizeof(packet_header::flags)
        + sizeof(packet_header::checksum);

    constexpr size_t MAX_PACKET_PAYLOAD_SIZE = MAX_PACKET_SIZE - PACKET_HEADER_SIZE;


    // --- Concrete Packet Payloads ---
    struct packet_error
    {
        protocol_error code;
    };

    struct packet_conn_req
    {
        uint64_t client_nonce;
    };

    struct packet_conn_accept
    {
        uint64_t    server_nonce;
        uint8_t     need_file_sync;
    };

    struct packet_conn_reject
    {
        reject_reason reason;
    };

    struct packet_conn_cancel
    {
        // Reserved/Empty payload stage
    };

    struct packet_disconnect_req
    {
        disconnect_type     type;
        disconnect_reason   reason;
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

    bool make_packet_payload(packet_header& header, uint16_t payload_size, const void* payload);

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
    bool read_packet_struct(protocol_reader& reader, packet_error& value);
    bool write_packet_struct(protocol_writer& writer, const packet_error& value);

    // packet_disconnect_req
    bool read_packet_struct(protocol_reader& reader, packet_disconnect_req& value);
    bool write_packet_struct(protocol_writer& writer, const packet_disconnect_req& value);

    // base validate
    bool validate_header(const packet_header& header, size_t payload_size, const void* payload);
}
