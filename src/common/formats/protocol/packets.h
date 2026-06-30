
#pragma once

#include <cstdint>
#include "protocol.h"

namespace tungsten::protocol
{
#pragma pack(push, 1)

	constexpr char 		MAGIC[] = "TUNGSTEN 1.0b";
	constexpr size_t 	MAGIC_SIZE = sizeof(MAGIC) - 1;
	constexpr uint16_t 	VERSION = 1;

	enum class PACKET_FLAGS : uint8_t
	{
		PACKET_NONE_FLAG    = 	   0,
		PACKET_ENCRYPTED 	= 1 << 0,
		PACKET_COMPRESSED 	= 1 << 1,
		PACKET_RELIABLE 	= 1 << 2,
	};

	enum class packet_type : uint8_t
	{
		none = 0,

		error,

		conn_req,
		conn_cancel,
		conn_accept,
		conn_reject,

		sv_levelstate,
		sv_snapshot,
		cl_snapshot_ack,

		cl_ready,

		cl_status_req,
		sv_status_ack,

		cl_usercmd,
		sv_usercmd_ack,

		disconnect_req,
		disconnect_ack,

		transfer,
	};

	struct packet_header
	{
		uint64_t	timestamp_us;
		
		char		magic[MAGIC_SIZE];
		uint16_t	protocol_version;
		uint64_t	receiver_nonce;

		uint16_t	header_size;
		uint16_t	payload_size;

		packet_type	type;
		uint8_t		flags;

		uint64_t	checksum;
	};

	constexpr size_t PACKET_HEADER_SIZE 		= sizeof(packet_header);	
    constexpr size_t MAX_PACKET_PAYLOAD_SIZE 	= MAX_PACKET_SIZE - PACKET_HEADER_SIZE;

	struct packet
	{
		packet_header 	header;
		uint8_t 		payload[MAX_PACKET_PAYLOAD_SIZE];
	};

	struct packet_error
	{
		protocol_error code;
	};

    // Connection stage packets
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

    // Loading stage packets
	struct packet_conn_cancel
	{
		uint64_t client_nonce;
	};

    /*
    // file_sync stage
    constexpr size_t MAX_FILENAME_SIZE = 32;
    using file_name = fixed_string<MAX_FILENAME_SIZE>;

    struct file_info
    {
        uint64_t total_size;
        uint64_t checksum;
        file_name name;
    };

	struct file_manifest_header
	{
        uint64_t total_size;
        uint16_t files_count;
        uint64_t checksum;
    };

    constexpr size_t MAX_MANIFEST_FILES_COUNT = 16;

    struct packet_sv_file_manifest
    {   
        file_manifest_header    header;
        file_info               files[MAX_MANIFEST_FILES_COUNT];
    };

	constexpr size_t MAX_CLASSNAME_SIZE = 64;
	using sv_class_name = fixed_string<MAX_CLASSNAME_SIZE>;

	constexpr size_t MAX_CLASSES_COUNT = 16;

    // level_sync stage
	struct packet_sv_levelstate
	{
		uint64_t        tickrate;                     // ms = 1.0/tickrate
		uint16_t        class_count;
		sv_class_name   class_names[MAX_CLASSES_COUNT];
	};

    // snashot_sync stage
	struct packet_sv_snapshot_header
	{
		uint64_t    server_tick;
		int64_t     delta_from;   // -1 is full
		uint16_t    entity_count; // max edicts ? for good must be less than 200, but..., is want more 
	};
    */

    struct packet_disconnect_req
    {
        disconnect_type     type;
        disconnect_reason   reason;
    };

	/*
    struct packet_disconnect_ack
    {
    };
	*/
#pragma pack(pop)

    // !!! MAKE PACKETS ONLY FROM HERE !!!
	packet make_packet
    (
        uint64_t    timestamp_us,
        packet_type type,
        uint8_t     flags,
        uint64_t    receiver_nonce,
        uint16_t    payload_size,
        const void* payload
    );

    // packet_header swap
    void packet_header_to_native   (packet_header& protocol);
    void packet_header_to_protocol (packet_header& native);

    uint16_t swap16(uint16_t value);
    uint32_t swap32(uint32_t value);
    uint64_t swap64(uint64_t value);

    // swap to native endian
    uint16_t to_native16(uint16_t protocol);
    uint32_t to_native32(uint32_t protocol);
    uint64_t to_native64(uint64_t protocol);
    
    // swap to protocol endian
    uint16_t to_protocol16(uint16_t native);
    uint32_t to_protocol32(uint32_t native);
    uint64_t to_protocol64(uint64_t native);

    // Compression
    bool    compress(size_t src_size, const void* src, size_t& dst_size, void* dst);
    bool  decompress(size_t src_size, const void* src, size_t& dst_size, void* dst);
	
    // En/De-cryption

    // bool encrypt();
    // bool decrypt();

    // Validation
	bool validate_protocol_version  (const packet_header& header);
	bool validate_magic             (const packet_header& header);
	bool validate_packet_sizes      (const packet_header& header);
	bool validate_nonce             (const packet_header& header, uint64_t nonce);
	bool validate_checksum          (uint64_t checksum, int size, const void* data);


	enum VALIDATE_FLAGS : uint8_t
	{
		VALIDATE_NONE 		= 	   0,
		VALIDATE_MAGIC    	= 1 << 0,
		VALIDATE_CHECKSUM 	= 1 << 1,
		VALIDATE_VERSION  	= 1 << 2,
		VALIDATE_NONCE    	= 1 << 3,
		VALIDATE_SIZES    	= 1 << 4,
	};

	bool validate_packet(uint64_t nonce, uint8_t flags, int size, const void* data);
	
    bool validate_all_packet(const packet& p, uint64_t nonce);

    // Checksum, compress and encryption
    bool make_checksum(packet& p);
    
    bool encrypt_packet(packet& p);
    bool decrypt_packet(packet& p);
    
    bool compress_packet(packet& p);
    bool decompress_packet(packet& p);

	bool parse_packet(packet& out, int size, const void* data);

	bool check_range(int total, int min, int max);
	
    // assertions
	static_assert(sizeof(packet			   ) <= MAX_PACKET_SIZE		   , "sizeof(packet) > MAX_PACKET_SIZE");
	static_assert(sizeof(packet_conn_req   ) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_req) > MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_conn_accept) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_accept) > MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_conn_reject) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_reject) > MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_conn_cancel) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_cancel) > MAX_PACKET_PAYLOAD_SIZE");
}