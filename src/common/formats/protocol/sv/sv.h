
#pragma once

#include "../protocol.h"
#include "container/ring_queue.h"
#include "container/static_span.h"

namespace tungsten::protocol 
{
	/*
    SERVER CLIENT SLOT FSM

    main stages:
        empty
        connecting
        loading
        active
        disconnecting

    loading stages:
        
        level_sync
        snapshot_sync

    any not empty stage:
        on timeout:
            emit_error(timeout)
            -> empty

    empty:
        on open(cl_nonce, sv_nonce, need_file_sync):
            -> active
        on else:
            emit_error(bad_stage);
            -> empty

    loading:
        on cancel():
            -> empty

        level_sync:
            on else:
                emit_error(bad_stage);
                -> empty
        snapshot_sync:
            on else:
                emit_error(bad_stage);
                -> empty        

    active:
        on close(type, reason):
            emit_send_disconnect(type, reason)
            -> disconnecting
        recv cl_disconnect_req:
            emit_send_disconnect_ack
            -> empty
        recv else
            emit_error(bad_packet)
            -> empty
        on else:
            emit_error(bad_stage)
            -> empty

    disconnecting:
        waiting_disconnect_ack:
            recv disconnect_ack:
                -> empty
            recv else:
                emit_error(bad_packet)
                -> empty
*/
  
    enum class server_main_stage
    {
        empty = 0,

        connecting,
        loading,
        active,
        disconnecting
    };

    enum class server_loading_stage
    {
        none = 0,

        file_sync,
        level_sync,
        snapshot_sync
    };

    enum class server_file_sync_stage
    {
        none = 0,
    };

    enum class server_level_sync_stage
    {
        none = 0,
    };

    enum class server_snapshot_sync_stage
    {
        none = 0,
    };

	class server_client_fsm
	{
	public:
		 server_client_fsm() = default;
		~server_client_fsm() = default;

		server_client_fsm(const server_client_fsm& ) = delete;
		server_client_fsm(      server_client_fsm&&) = delete;
		void operator=   (const server_client_fsm& ) = delete;
		void operator=   (      server_client_fsm&&) = delete;

		void tick(uint64_t delta_time_us);
		void reset();

		bool poll_event(event& e);

		void on_recv_packet(const byte_span& data);
        
		bool open(uint64_t cl_nonce, uint64_t sv_nonce, bool need_file_sync);
        void close(disconnect_type type, disconnect_reason reason);

		static bool is_conn_req  (const byte_span& data, uint64_t& client_nonce);
		static bool is_status_req(const byte_span& data);

        static bool reject(event_send& out, uint64_t timestamp_us, uint64_t cl_nonce, reject_reason reason);

	private:

		bool push_event(      event&& e);
		bool push_event(const event&  e);

        // events emitting
		void emit_send(const packet& p, bool reliable);
		void emit_error(event_error_type code, protocol_error protocol);
		void emit_connection_canceled();

        
        void emit_send_error(protocol_error code);
        
        // void emit_recv_usercmd();
        // void emit_recv_cl_status_req();

        void emit_send_disconnect_req(disconnect_type type, disconnect_reason reason);
        void emit_send_disconnect_ack();
        void emit_disconnected(disconnect_type type, disconnect_reason reason);

		util::container::ring_queue<event, 8> events;
		
		// timing
		uint64_t    last_timestamp_us       = 0;
		uint64_t    curr_timestamp_us       = 0;
		uint64_t    delta_us		        = 0; 

		// timeouts
		uint64_t    last_recv_timestamp_us  = 0;
		uint64_t    retry_interval_us       = 5;
		int         retry_count             = 5;
		int         tries_count             = 0;

		void check_timeouts();
		void reset_timeouts();

		// connecting
		uint64_t    server_nonce            = 0;
		uint64_t    client_nonce            = 0;

        // utils
        void error(event_error_type code, protocol_error protocol);

        bool on_recv_error                  (const packet& p);
		bool on_recv_loading                (const packet& p);

		bool on_recv_loading_file_sync      (const packet& p);
		bool on_recv_loading_level_sync     (const packet& p);
		bool on_recv_loading_snapshot_sync  (const packet& p);
		
		bool on_recv_active                 (const packet& p);

        bool on_recv_active_disconnect_req  (const packet& p);
        bool on_recv_active_cl_status_req   (const packet& p);
		bool on_recv_active_cl_usercmd      (const packet& p);
		
        bool on_recv_disconnecting(const packet& p);

		server_main_stage 			main_stage			= server_main_stage			::empty;
		server_loading_stage 		loading_stage		= server_loading_stage		::none;
		server_file_sync_stage 		file_sync_stage		= server_file_sync_stage	::none;
		server_level_sync_stage 	gamesync_stage		= server_level_sync_stage	::none;
		server_snapshot_sync_stage 	snapshot_sync_stage = server_snapshot_sync_stage::none;
	};
}