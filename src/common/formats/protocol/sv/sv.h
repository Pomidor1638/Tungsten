
#pragma once

#include "../base_fsm/base_fsm.h"
#include "protocol/protocol.h"

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


    //typedef bool (*on_status_req_func)(void);
    //typedef void (*on_usercmd_func)(void);

    struct server_callbacks
    {
        //on_status_req_func on_status_req = nullptr;
    };


    class server_client_fsm final : public base_fsm
	{
	public:

        server_client_fsm(void* ctx = nullptr, timeout_config cfg = {}, base_fsm_callbacks cmn_callbacks = {}, server_callbacks sv_callbacks = {});

        void set_server_callbacks(server_callbacks callbacks);

		bool open(uint64_t cl_nonce, uint64_t sv_nonce, bool need_file_sync);
        void close();

        void disconnect(disconnect_type type, disconnect_reason reason) override;

		static bool is_conn_req  (const data_span& data, uint64_t& client_nonce, bool& bad_version);
		static bool is_status_req(const data_span& data);
        static bool reject(uint64_t timestamp_us, uint64_t cl_nonce, reject_reason reason, int max_size, int& out_size, void* out_data);
        
	private:

        void pure_reset();

        // utils
        bool send_conn_accept(bool need_filesync);
        bool send_disconnect_ack();
        bool send_disconnect_req(disconnect_type type, disconnect_reason reason);

        // overrides
        void on_reset() override;
        void on_tick()  override;
        bool on_recv_custom() override;
        bool on_disconnect_req(disconnect_type type, const disconnect_reason& reason) override;
        bool on_disconnect_ack() override;

        // on_recv_* funcs
        bool on_recv_connecting();
		bool on_recv_loading();
		    bool on_recv_loading_file_sync();
		    bool on_recv_loading_level_sync();
		    bool on_recv_loading_snapshot_sync();
		bool on_recv_active();
            bool on_recv_active_cl_status_req();
		    bool on_recv_active_cl_usercmd();
        bool on_recv_disconnecting();

        server_callbacks sv_callbacks{};

		server_main_stage 			main_stage			= server_main_stage			::empty;
		server_loading_stage 		loading_stage		= server_loading_stage		::none;
		server_file_sync_stage 		file_sync_stage		= server_file_sync_stage	::none;
		server_level_sync_stage 	gamesync_stage		= server_level_sync_stage	::none;
		server_snapshot_sync_stage 	snapshot_sync_stage = server_snapshot_sync_stage::none;
	};
}