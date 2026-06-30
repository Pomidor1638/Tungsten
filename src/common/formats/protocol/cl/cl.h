
#pragma once

#include "../protocol.h"
#include "container/ring_queue.h"

namespace tungsten::protocol 
{
/*
    CLIENT FSM

    main stages:
        disconnected
        connecting
        loading
        active
        disconnecting

    loading stages:
        file_sync
        level_sync
        snapshot_sync

    rule:
        waiting_*  	= wait for packet from endpoint
        pending_*  	= wait for owner action
        receiving_* = transfer internal receiving
        sending_*   = transfer internal transmitting

    any not disconnected stage:
        timeout:
            emit error(timeout)
            -> disconnected

    disconnected:
        connect_to():
            emit send(conn_req)
            -> connecting

    connecting:
        recv conn_accept:
            if need_file_sync:
                -> loading.file_sync.waiting_file_manifest
            else:
                -> loading.level_sync.waiting_level_info
        recv conn_reject:
            emit connection_rejected
            -> disconnected
        cancel():
            emit send(conn_cancel)
            -> disconnected

    loading:
        cancel():
            emit send(conn_cancel)
            -> disconnected
        file_sync:
        level_sync:
        snapshot_sync:

    active:
        recv sv_disconnect_req:
            emit_send_disconnect_ack()
            emit_disconnected()
            -> disconnected
        disconnect():
            emit_send_disconnect_req()
            -> disconnecting
    disconnecting:
        recv disconnect_ack:
            -> disconnected
*/

	enum class client_main_stage
	{
		none = 0,

		disconnected,
		connecting,
		loading,
		active,
		disconnecting,
	};

	enum class client_loading_stage
	{
		none = 0,

		file_sync,
		level_sync,
		snapshot_sync
	};

    enum class client_file_sync_stage
    {
        none = 0,
    };

    enum class client_level_sync_stage
    {
        none = 0,
    };



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
		void on_recv_packet(const byte_span& data);

		bool connect_to(uint64_t nonce);	
		//bool internal_connect(uint64_t nonce);

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

        // loading
        client_loading_stage loading_stage = client_loading_stage::none;
		// file_sync
		client_file_sync_stage file_sync_stage = client_file_sync_stage::none;
		client_level_sync_stage level_sync_stage = client_level_sync_stage::none;


		bool push_event(      event&& e);
		bool push_event(const event&  e);

		void emit_send(const packet& p, bool reliable);
		void emit_send_error(protocol_error code);
        void emit_send_disconnect_req(disconnect_type type, disconnect_reason reason);
        void emit_send_disconnect_ack();

        void emit_disconnected(disconnect_type type, disconnect_reason reason);
        
        void emit_error(event_error_type type, protocol_error protocol);
		void emit_conn_req();
		void emit_connection_accepted(bool need_file_sync);
		void emit_connection_rejected(reject_reason reason);

		void error(event_error_type type, protocol_error protocol);

        bool on_recv_error                  	(const packet& p);
		bool on_recv_connecting					(const packet& p);
			bool recv_conn_accept				(const packet& p);
			bool recv_conn_reject				(const packet& p);
		bool on_recv_loading					(const packet& p);
			bool on_loading_file_sync			(const packet& p);
			bool on_loading_level_info			(const packet& p);
			bool on_loading_snapshot_sync		(const packet& p);
		bool on_recv_active						(const packet& p);
		    bool on_recv_active_disconnect_req	(const packet& p);
		    bool on_recv_active_sv_status_ack	(const packet& p);
		    bool on_recv_active_sv_snapshot		(const packet& p);
        bool on_recv_disconnecting				(const packet& p);
	};
}