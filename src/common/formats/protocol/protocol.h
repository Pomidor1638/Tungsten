#pragma once

#include "container/ring_queue.h"
#include <cstddef>
#include <cstdint>

// All protocol structures are packed to 1 byte.
// This prevents compiler padding from changing packet sizes.

// This file describes packet structures only and protocol fsm.
// Transport, reliability, channels and resending are handled elsewhere.

namespace tungsten::protocol
{

#pragma pack(push, 1)
	constexpr char 		MAGIC[] = "TUNGSTEN";
	constexpr size_t 	MAGIC_SIZE = sizeof(MAGIC) - 1;
	constexpr uint16_t 	VERSION = 1;

	enum class packet_flags : uint8_t
	{
		none 		= 	   0,
		encrypted 	= 1 << 0,
		compressed 	= 1 << 1,
	};

	enum class packet_type : uint8_t
	{
		none = 0,

		conn_req,
		conn_cancel,
		conn_accept,
		conn_reject,

		cl_status_req,
		sv_status_ack,

		sv_gamestate,
		sv_snapshot,
		
		cl_ready,
		cl_usercmd,

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
	constexpr size_t MAX_PACKET_SIZE 			= 1200;
    constexpr size_t MAX_PACKET_PAYLOAD_SIZE 	= MAX_PACKET_SIZE - PACKET_HEADER_SIZE;

	struct packet
	{
		packet_header 	header;
		uint8_t 		payload[MAX_PACKET_PAYLOAD_SIZE];
	};

	bool parse_packet(packet& out, size_t size, const void* data);

	// Fixed string with explicit size. It does not require null termination.
	template <int N>
	struct fixed_string
	{
		uint16_t	size;
		char 		data[N];
	};

	using vec2 = float[2];
	using vec3 = float[3];


	struct packet_conn_req
	{
		uint64_t client_nonce;
	};

	struct packet_conn_cancel
	{
		uint64_t client_nonce;
	};

	struct packet_conn_accept
	{
		uint64_t server_nonce;
		uint8_t need_filesync;
	};

	constexpr size_t MAX_REJECT_REASON_SIZE = 256;

	using reject_reason = fixed_string<MAX_REJECT_REASON_SIZE>;

	struct packet_conn_reject
	{	
		reject_reason reason;
	};


	struct packet_sv_file_manifest
	{};


	constexpr size_t MAX_CLASSNAME_SIZE = 64;
	using sv_classname = fixed_string<MAX_CLASSNAME_SIZE>;

	constexpr size_t MAX_CLASSES_COUNT = 16;

	struct packet_sv_gamestate
	{
		uint64_t server_tick;
		uint16_t class_count;
		sv_classname classnames[MAX_CLASSES_COUNT];
	};

	struct packet_sv_snapshot_header
	{
		uint32_t server_tick;
		int32_t delta_from;
		uint16_t entity_count;
	};

	
#pragma pack(pop)

	enum class event_type
	{
		none = 0,

		send,
		error,

	// server events
		connection_canceled,

	// client events
		connection_accepted,
		connection_rejected,

	};

	struct event_send
	{
		bool reliable;
		packet p;
	};

	enum class event_error_type
	{
		none = 0,
		timeout,
	};

	struct event_error
	{
		event_error_type type;
	};

	struct event_conn_requested
	{};

	struct event_conn_accepted
	{
		bool need_filesync;
	};

	struct event_conn_rejected
	{
		reject_reason reason;
	};

	struct event_conn_cancel 
	{};

	struct event
	{
		event_type type;
		union
		{
			event_send 				send;
			event_error 			error;
			event_conn_requested 	conn_requested;
			event_conn_cancel		conn_cancel;
			event_conn_accepted 	conn_accepted;
			event_conn_rejected 	conn_rejected;
		};
	};

	enum class client_main_stage
	{
		none = 0,

		disconnected,
		connecting,
		loading,
		active,
		disconnecting,
	};

	/*

		Client main stage sequence

		disconnected:
			connect_to():
				emit send conn_req
				-> connecting
		
		connecting:
			recv conn_accept:
				emit conn_accepted
				-> loading
			recv conn_reject:
				emit conn_rejected
				-> disconnected
			timeout 
				-> disconnected
		
		loading:
			recv sv_snapshot_end:
				-> active
			timeout:
				-> disconnect

		active:

		disconnecting:

	*/

	/*

	*/

	/*
		Client filesync stage sequence
	*/

	enum class client_loading_stage
	{
		none = 0,

		filesync,
		gamesync,
		snapshot_sync
	};

	/*
		Client snapshot_sync stage sequence
	*/

	enum class client_snapshot_stage
	{
		none = 0,
	};

	packet make_packet
    (
        uint64_t timestamp_us,
        packet_type type,
        int payload_size,
        const void* payload,
        uint8_t flags,
        uint64_t nonce
    );
	
	bool validate_protocol_version(const packet_header& header);
	bool validate_magic(const packet_header& header);
	bool validate_packet_sizes(const packet_header& header);
	bool validate_nonce(const packet_header& header, uint64_t nonce);
	bool validate_checksum(const packet& p);
	
	bool make_checksum(packet& p);
	
	bool validate_packet(const packet& p, uint64_t nonce);

	class client_fsm
	{
	public:
		client_fsm();
		~client_fsm() = default;

		client_fsm(const client_fsm& )     = delete;
		client_fsm(      client_fsm&&)     = delete;
		void operator=(const client_fsm& ) = delete;
		void operator=(      client_fsm&&) = delete;

		void tick(uint64_t delta_time_us);
		void reset();

		bool poll_event(event& e);
		
		void on_recv_packet(const packet& p);		
		bool connect_to(uint64_t nonce);

		// TODO: need to fix cancel conqurency
		bool cancel();

	private:

		util::container::ring_queue<event, 8> events;
		client_main_stage main_stage = client_main_stage::none;

		// timing
		uint64_t last_timestamp_us = 0;
		uint64_t curr_timestamp_us = 0;
		uint64_t delta_us		   = 0; 

		// timeouts
		uint64_t last_recv_timestamp_us = 0;
		uint64_t retry_interval_us  	= 5;
		int retry_count = 5;
		int tries_count = 0;

		void check_timeouts();
		void reset_timeouts();

		// connecting
		uint64_t client_nonce = 0;
		uint64_t server_nonce = 0;

		// filesync
		client_loading_stage loading_stage = client_loading_stage::none;

		bool push_event(const event& e);

		void emit_send(const packet& p, bool reliable);
		void emit_error(event_error_type type);
		void emit_connection_accepted(bool need_filesync);
		void emit_connection_rejected(reject_reason reason);
		
		bool on_recv_connecting(const packet& p);
			bool recv_conn_accept(const packet& p);
			bool recv_conn_reject(const packet& p);

		bool on_recv_loading(const packet& p);
			bool on_loading_filesync(const packet& p);
			bool on_loading_level_info(const packet& p);
			bool on_loading_snapshot_sync(const packet& p);

		bool on_recv_active(const packet& p);
		bool on_recv_disconnecting(const packet& p);
	};


	/*
		Server main stage sequence
	*/

	enum class server_main_stage
	{
		none = 0,

		empty,
		loading,
		active,
		disconnecting
	};

	/*
		Server loading stage sequence
	*/

	enum class server_loading_stage
	{
		none = 0,

		filesync,
		gamesync,
		snapshot_sync,
		ready_sync,
	};

	/*
		Server filesync stage sequence
	*/

	enum class server_filesync_stage
	{
		none = 0,
		pending_file_manifest,
		waiting_needed_file_manifest,
		checking_missing_files,
		pending_file,
		sending_file,
	};

	enum class server_file_stage
	{
		none = 0,

		checking_file_fragments,

	};

	/*
		Server gamesync stage sequence
	*/

	enum class server_gamesync_stage
	{
		none = 0,
		pending_gamestate, // only diffs from .tbsp
		waiting_ack,
	};

	/*
		Server snapshot_sync stage sequence
	*/

	enum class server_snapshot_sync_stage
	{
		none = 0,
	};
	

	class server_client_fsm
	{
	public:
		server_client_fsm()  = default;
		~server_client_fsm() = default;

		server_client_fsm(const server_client_fsm& ) = delete;
		server_client_fsm(      server_client_fsm&&) = delete;
		void operator=(const server_client_fsm& ) = delete;
		void operator=(      server_client_fsm&&) = delete;

		void tick(uint64_t delta_time_us);
		void reset();

		bool poll_event(event& e);
		void on_recv_packet(const packet& p);

		bool accept(uint64_t cl_nonce, uint64_t sv_nonce, bool need_filesync);
		
		bool load_gamestate(const packet_sv_gamestate& gs);

		static bool is_conn_req(const packet& p);
		static bool reject(packet& out, uint64_t timestamp_us, uint64_t cl_nonce, reject_reason reason);

		void disconnect();

	private:

		bool push_event(const event& e);

		void emit_send(const packet& p, bool reliable);
		void emit_error(event_error_type type);

		void emit_connection_canceled();

		util::container::ring_queue<event, 8> events;
		
		// timing
		uint64_t last_timestamp_us = 0;
		uint64_t curr_timestamp_us = 0;
		uint64_t delta_us		   = 0; 

		// timeouts
		uint64_t last_recv_timestamp_us = 0;
		uint64_t retry_interval_us  = 5;
		int retry_count = 5;
		int tries_count = 0;

		void check_timeouts();
		void reset_timeouts();

		// connecting
		uint64_t server_nonce = 0;
		uint64_t client_nonce = 0;

		//bool on_recv_empty(const packet& p);
		bool on_recv_loading(const packet& p);

		bool on_recv_loading_filesync(const packet& p);
		bool on_recv_loading_gamesync(const packet& p); // waiting_ack
		bool on_recv_loading_snapshot_sync(const packet& p);
		
		bool on_recv_active(const packet& p);
		bool on_recv_disconnecting(const packet& p);

		server_main_stage 			main_stage			= server_main_stage			::none;
		server_loading_stage 		loading_stage		= server_loading_stage		::none;
		server_filesync_stage 		filesync_stage		= server_filesync_stage		::none;
		server_gamesync_stage 		gamesync_stage		= server_gamesync_stage	::none;
		server_snapshot_sync_stage 	snapshot_sync_stage = server_snapshot_sync_stage::none;
		
	};


	static_assert(sizeof(packet			   ) <= MAX_PACKET_SIZE		   , "sizeof(packet			   ) > MAX_PACKET_SIZE");
	static_assert(sizeof(packet_conn_req   ) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_req   ) > MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_conn_accept) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_accept) > MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_conn_reject) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_reject) > MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_conn_cancel) <= MAX_PACKET_PAYLOAD_SIZE, "sizeof(packet_conn_cancel) > MAX_PACKET_PAYLOAD_SIZE");

}

