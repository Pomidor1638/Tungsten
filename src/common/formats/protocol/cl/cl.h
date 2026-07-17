
#pragma once

#include "../base_fsm/base_fsm.h"
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


    typedef void(*on_connection_accepted_func)(bool need_file_sync);
    typedef void(*on_connection_rejected_func)(const reject_reason& reason);
    typedef void(*on_disconnect_req_func)(disconnect_type type, const disconnect_reason& reason);
    typedef void(*on_disconnect_ack_func)();

    struct client_callbacks
    {
        on_connection_accepted_func on_conn_accepted = nullptr;
        on_connection_rejected_func on_conn_rejected = nullptr;
        on_disconnect_req_func      on_disconnect_req = nullptr;
        on_disconnect_ack_func      on_disconnect_ack = nullptr;
    };


	class client_fsm final : public base_fsm
	{
	public:

        client_fsm(void* ctx = nullptr, timeout_config cfg = {}, base_fsm_callbacks cmn_callbacks = {}, client_callbacks cl_callbacks = {});

		bool open(uint64_t nonce);	

        void disconnect(disconnect_type type, disconnect_reason reason) override;
        void cancel();

	private:

        bool on_recv_custom() override;
        void on_reset() override;
        void on_tick() override;

        bool on_disconnect_req(disconnect_type type, const disconnect_reason& reason) override;
        bool on_disconnect_ack() override;

        // redefenition
        void pure_reset();

        client_callbacks cl_callbacks = {};

        client_main_stage       main_stage       = client_main_stage      ::none;
        client_loading_stage    loading_stage    = client_loading_stage   ::none;
		client_file_sync_stage  file_sync_stage  = client_file_sync_stage ::none;
		client_level_sync_stage level_sync_stage = client_level_sync_stage::none;

        bool send_conn_req();
        bool send_disconnect_ack();

        bool call_disconnect_ack();
        bool call_disconnect_req(disconnect_type type, const disconnect_reason& reason);
        bool call_connection_accepted(bool need_file_sync);
        bool call_connection_rejected(const reject_reason& reason);

		bool on_recv_connecting                 ();
			bool process_conn_accept            ();
			bool process_conn_reject            ();
		bool on_recv_loading                    ();
			bool on_loading_file_sync           ();
			bool on_loading_level_info          ();
			bool on_loading_snapshot_sync       ();
		bool on_recv_active                     ();
		    bool on_recv_active_sv_status_ack   ();
		    bool on_recv_active_sv_snapshot     ();
        bool on_recv_disconnecting              ();
	};
}