#pragma once
#include <cstddef>
#include <cstdint>

// All protocol structures are packed to 1 byte.
// This prevents compiler padding from changing packet sizes.
#pragma pack(push, 1)

// This file describes packet structures only.
// Transport, reliability, channels and resending are handled elsewhere.

namespace tungsten::protocol
{
	constexpr char 		MAGIC[] = "TUNGSTEN";
	constexpr size_t 	MAGIC_SIZE = sizeof(MAGIC) - 1;

	constexpr uint16_t 	VERSION 					= 1;

	constexpr size_t	MAX_PROTOCOL_REASON_SIZE	= 256;

	constexpr size_t 	MAX_PROTOCOL_FILENAME_SIZE  = 32;
	constexpr size_t 	MAX_PROTOCOL_FILES          = 24;
	
	constexpr size_t 	MAX_SNAPSHOT_ENTITIES       = 32;
	constexpr size_t 	MAX_FILE_FRAGMENT_DATA_SIZE = 1024;
	
	constexpr size_t 	MAX_CONCOLE_COMMAND_SIZE    = 512;

	enum class packet_type : uint8_t
	{
		none = 0,

		// Connection
		conn_req,
		conn_ack,
		conn_cancel,
		conn_error,
		conn_ready,

		// File check / download
		conn_req_files,
		conn_ack_files,
		conn_req_file,
		conn_file_fragment,
		conn_ack_file,

		// Client -> server game traffic
		cl_snapshot,
		cl_event,
		cl_req,
		cl_ack,

		// Server -> client game traffic
		sv_snapshot,
		sv_event,
		sv_req,
		sv_ack,

		// Disconnection
		disconnect_req,
		disconnect_ack,

		// Server browser / ping
		status_req,
		status_ack,
	};

	enum class packet_flags : uint8_t
	{
		none = 0,
	};

	struct packet_header
	{
		uint64_t    timestamp_us;

		char		magic[MAGIC_SIZE];
		uint32_t    sequence;
		uint32_t    tick;

		uint16_t    protocol_version;
		uint16_t    header_size;
		uint16_t    payload_size;

		packet_type type;
		uint8_t     flags;

		uint32_t    checksum;
	};


	constexpr size_t PACKET_HEADER_SIZE 		= sizeof(packet_header);
	
	constexpr size_t MAX_PACKET_SIZE 			= 1400;
    constexpr size_t MAX_PACKET_PAYLOAD_SIZE 	= MAX_PACKET_SIZE - PACKET_HEADER_SIZE;

	struct packet
	{
		packet_header 	header;
		uint8_t 		payload[MAX_PACKET_PAYLOAD_SIZE];
	};

	// Fixed string with explicit size. It does not require null termination.
	template <int N>
	struct fixed_string
	{
		uint16_t 	size;
		char 		data[N];
	};

	using protocol_reason   = fixed_string<MAX_PROTOCOL_REASON_SIZE>;
	using protocol_filename = fixed_string<MAX_PROTOCOL_FILENAME_SIZE>;

	using vec2 = float[2];
	using vec3 = float[3];

	//------------//
	// Connection //
	//------------//

	enum class conn_error_code : uint8_t
	{
		none = 0,
		bad_protocol_version,
		server_full,
		banned,
		timeout,
		files_mismatch,
		internal_error,
	};

	enum class conn_cancel_reason : uint8_t
	{
		none = 0,
		user_cancelled,
		file_download_cancelled,
		timeout,
		protocol_error,
	};

	struct packet_conn_req
	{
		uint32_t client_protocol_version;
		uint64_t client_nonce;
	};


	struct packet_conn_ack
	{
		uint64_t 	server_nonce;
		uint32_t 	server_tick;
		uint32_t 	server_tickrate;

		uint8_t 	need_files_check;
		uint8_t 	reserved[7];
	};

	struct packet_conn_error
	{
		conn_error_code code;
		protocol_reason reason;
	};

	struct packet_conn_cancel
	{
		conn_cancel_reason reason;
	};

	struct packet_conn_ready
	{
		uint32_t client_tick;
	};

	//-----------------------// 
	// File check / download //
	//-----------------------// 

	struct file_info
	{
		uint16_t 			file_id;
		protocol_filename 	filename;

		uint64_t 			size;
		uint64_t 			hash;
	};

	struct packet_conn_req_files
	{
		uint16_t 	files_count;
		file_info 	files[MAX_PROTOCOL_FILES];
	};

	struct packet_conn_ack_files
	{
		uint16_t 	missing_files_count;
		uint16_t 	missing_file_ids[MAX_PROTOCOL_FILES];
	};

	struct packet_conn_req_file
	{
		uint16_t 	file_id;
		uint32_t 	start_offset;
	};

	struct packet_conn_file_fragment
	{
		uint16_t 	file_id;

		uint32_t 	offset;
		uint16_t 	data_size;

		uint64_t 	total_file_size;
		uint64_t 	file_hash;

		uint8_t 	data[MAX_FILE_FRAGMENT_DATA_SIZE];
	};

	enum class file_ack_status : uint8_t
	{
		ok = 0,
		bad_hash,
		bad_size,
		cancelled,
	};

	struct packet_conn_ack_file
	{
		uint16_t 		file_id;
		file_ack_status status;
	};

	//-----------//
	//   Game    //
	//-----------//

	enum class input_buttons : uint64_t
	{
		none   = 			0,
		attack =	1ull << 0,
		jump   = 	1ull << 1,
		duck   = 	1ull << 2,
		use    = 	1ull << 3,
		reload = 	1ull << 4,
	};


	using console_command = fixed_string<MAX_CONCOLE_COMMAND_SIZE>;

	struct usercmd
	{
		vec2 	 wishdir;
		vec3 	 view_angles;
		uint64_t buttons;
	};

	struct packet_cl_snapshot
	{
		usercmd cmd;
		console_command console_cmd;
	};


	enum class geometry_type : uint8_t
	{
		none = 0,
		
		sprite,
		bsp,
		alias
	};

	struct entity_state
	{
		uint16_t 	entity_id;

		vec3 		origin;
		vec3 		velocity;
		vec3 		angles;
	};

	constexpr size_t MAX_CLASS_COUNT    = 32;
	constexpr size_t MAX_CLASSNAME_SIZE = 32;

	struct entity_def
	{
		uint16_t	entity_id;
		uint16_t	class_index;

		vec3 		origin;
		vec3 		angles;
	};

	struct packet_entity_table
	{
		uint16_t	count;
		entity_def	entities[MAX_SNAPSHOT_ENTITIES];
	};

	struct packet_sv_snapshot
	{
		uint32_t        server_tick;
		uint16_t        entity_count;
		entity_state 	entities[MAX_SNAPSHOT_ENTITIES];
	};

	/*

	//-------------//
	// Game events //
	//-------------//

	enum class game_event_type : uint16_t
	{
		none = 0,

		// Add game-specific events here.
		// Example: item_pickup, weapon_fire, chat_message, ability_used.
	};

	struct packet_cl_event
	{
		game_event_type type;
		uint32_t client_tick;
		uint32_t sequence;

		uint16_t data_size;
		uint8_t data[256];
	};

	struct packet_sv_event
	{
		game_event_type type;
		uint32_t server_tick;
		uint32_t sequence;

		uint32_t target_entity_id;

		uint16_t data_size;
		uint8_t data[256];
	};
	*/

	/*

	//-----------------------------//
	// Requests / acknowledgements //
	//-----------------------------//

	enum class request_type : uint8_t
	{
		none = 0,
		full_snapshot,
		resend_snapshot,
		ping,
		pong,
	};

	struct packet_req
	{
		request_type type;
		uint32_t argument;
	};

	struct packet_ack
	{
		uint32_t received_sequence;
		uint32_t received_tick;
	};

	*/

	//---------------//
	// Disconnection //
	//---------------//

	enum class disconnect_reason : uint8_t
	{
		none = 0,
		client_quit,
		timeout,
		kicked,
		protocol_error,
	};

	struct packet_disconnect_req
	{
		disconnect_reason reason;
	};

	struct packet_disconnect_ack
	{
		disconnect_reason reason;
	};

	//--------//
	// Status //
	//--------//

	struct packet_status_req
	{
		uint64_t nonce;
	};

	struct packet_status_ack
	{
		uint64_t nonce;

		uint16_t players;
		uint16_t max_players;

		uint32_t server_tick;
		uint32_t server_tickrate;
	};

	//------------//
	// Client FSM //
	//------------//

	enum class client_state
	{
		disconnected,

		waiting_conn_ack,

		checking_files,
		waiting_files_ack,

		downloading_file,
		waiting_file_fragment,
		waiting_file_ack,

		loading_resources,
		ready,
		in_game,

		disconnecting,
		error,
	};

	// Events that can move the client FSM.
	enum class client_fsm_event
	{
		none = 0,

		start_connect,
		cancel_connection,
		resources_loaded,
		start_disconnect,

		timeout,
	};

	/*
		Client connection sequence:

		disconnected
			start_connect:
				send conn_req
				-> waiting_conn_ack

		waiting_conn_ack
			recv conn_ack:
				if need_files_check -> checking_files
				else -> loading_resources

			recv conn_error / timeout:
				-> error

		checking_files
			send conn_req_files
			-> waiting_files_ack

		waiting_files_ack
			recv conn_ack_files:
				if missing_files_count > 0 -> downloading_file
				else -> loading_resources

		downloading_file
			send conn_req_file
			-> waiting_file_fragment

		waiting_file_fragment
			recv conn_file_fragment:
				if file is not complete -> waiting_file_fragment
				if file is complete -> send conn_ack_file -> waiting_file_ack

		waiting_file_ack
			recv conn_ack_file:
				if more files needed -> downloading_file
				else -> loading_resources

		loading_resources
			resources_loaded:
				send conn_ready
				-> ready

		ready
			recv first sv_snapshot:
				-> in_game

		in_game
			each tick:
				send cl_snapshot
				recv sv_snapshot

		any stage before in_game
			cancel_connection:
				send conn_cancel
				-> disconnected

		any connected stage
			start_disconnect:
				send disconnect_req
				-> disconnecting

		disconnecting
			recv disconnect_ack / timeout:
				-> disconnected
	*/

	//------------//
	// Server FSM //
	//------------//

	enum class server_client_stage : uint8_t
	{
		disconnected,

		waiting_conn_req,
		checking_protocol,

		waiting_files_list,
		sending_file_list,
		waiting_file_request,
		sending_file,
		waiting_file_ack,

		waiting_ready,
		in_game,

		disconnecting,
		error,
	};

	enum class server_client_fsm_event
	{
		none = 0,

		recv_conn_req,
		send_conn_ack,
		recv_conn_cancel,
		send_conn_error,

		recv_files_list,
		send_files_ack,
		recv_file_request,
		send_file_fragment,
		recv_file_ack,

		recv_conn_ready,
		recv_cl_snapshot,

		recv_disconnect_req,
		send_disconnect_ack,
		timeout,
	};

	/*
		Server-side client sequence:

		waiting_conn_req
			recv conn_req:
				validate protocol version
				-> checking_protocol

		checking_protocol
			valid client:
				send conn_ack
				if files must be checked -> waiting_files_list
				else -> waiting_ready

			invalid client:
				send conn_error
				-> disconnected

		waiting_files_list
			recv conn_req_files:
				compare file hashes
				send conn_ack_files
				if missing_files_count > 0 -> waiting_file_request
				else -> waiting_ready

		waiting_file_request
			recv conn_req_file:
				-> sending_file

		sending_file
			send conn_file_fragment:
				if file is not complete -> sending_file
				if file is complete -> waiting_file_ack

		waiting_file_ack
			recv conn_ack_file:
				if file status is ok and more files needed -> waiting_file_request
				if file status is ok and no more files needed -> waiting_ready
				else -> error

		waiting_ready
			recv conn_ready:
				send first sv_snapshot
				-> in_game

		in_game
			each tick:
				recv cl_snapshot
				process usercmd
				send sv_snapshot

		any stage before in_game
			recv conn_cancel:
				stop file transfer if active
				-> disconnected

		any connected stage
			recv disconnect_req:
				send disconnect_ack
				-> disconnected

		any connected stage
			timeout:
				-> disconnected
	*/



	class packet_builder
	{
	public:
		void set_tick     (uint32_t value);
		void set_timestamp(uint64_t value);

		uint32_t get_sequence() const;

		bool build
		(
			packet& out,
			packet_type type,
			const void* payload,
			uint16_t payload_size
		);

		packet_conn_req build_conn_req();
		packet_conn_ack build_conn_ack();

		static bool validate_header(const packet_header& header);
		static bool validate_packet(const packet& packet);

	private:
		packet_header make_header(packet_type type, uint16_t payload_size);

		uint32_t sequence     = 0;
		uint32_t tick         = 0;
		uint64_t timestamp_us = 0;
	};

	static_assert(sizeof(packet_conn_req_files    ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_conn_ack_files    ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_conn_req_file     ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_conn_file_fragment) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_conn_ack_file     ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_entity_table      ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_disconnect_req    ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_disconnect_ack    ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_status_req        ) <= MAX_PACKET_PAYLOAD_SIZE);
	static_assert(sizeof(packet_status_ack		  ) <= MAX_PACKET_PAYLOAD_SIZE);


}

#pragma pack(pop)
