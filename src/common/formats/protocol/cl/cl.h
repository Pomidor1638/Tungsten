
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
        disconnected = 0,
        connecting,
        loading,
        active,
    };

    enum class client_loading_stage
    {
        idle = 0,
        file_sync,
        level_sync,
        snapshot_sync
    };

    enum class client_file_sync_stage
    {
        idle = 0,
    };

    enum class client_level_sync_stage
    {
        idle = 0,
    };


    typedef void(*on_connection_accepted_func)(void* context, bool need_file_sync);
    typedef void(*on_connection_rejected_func)(void* context, const reject_reason& reason);
    typedef bool(*           on_snapshot_func)(void* ctx, int size, const void* data);

    struct client_fsm_callbacks
    {
        on_connection_accepted_func on_conn_accepted = nullptr;
        on_connection_rejected_func on_conn_rejected = nullptr;
        on_snapshot_func            on_snapshot      = nullptr;
    };


    class client_fsm final : public base_fsm
    {
    public:

        client_fsm(void* ctx = nullptr, base_fsm_callbacks cmn_callbacks = {}, client_fsm_callbacks cl_callbacks = {});

        void set_client_callbacks(client_fsm_callbacks callbacks);

        void open(uint64_t nonce);
        void disconnect(disconnect_type type, const disconnect_reason& reason) override;

        void usercmd(int size, const void* data);

    private:

        // virtual funcs override

        void on_recv_custom(const packet_header& header, protocol_reader& reader) override;
        void on_reset() override;
        void on_tick() override;

        void on_disconnect(disconnect_type type, const disconnect_reason& reason) override;

        // redefenition, because 
        void pure_reset();

        // state
        client_fsm_callbacks cl_callbacks = {};

        client_main_stage       main_stage       = client_main_stage      ::disconnected;
        client_loading_stage    loading_stage    = client_loading_stage   ::idle;
        client_file_sync_stage  file_sync_stage  = client_file_sync_stage ::idle;
        client_level_sync_stage level_sync_stage = client_level_sync_stage::idle;

        // stage/type process utils
        void on_recv_connecting     (const packet_header& header, protocol_reader& reader);
            void process_conn_accept(const packet_header& header, protocol_reader& reader);
            void process_conn_reject(const packet_header& header, protocol_reader& reader);
        void on_recv_loading        (const packet_header& header, protocol_reader& reader);
        void on_recv_active         (const packet_header& header, protocol_reader& reader);
            void process_snapshot   (const packet_header& header, protocol_reader& reader);

        // fsm utils
        void to_main_stage_disconnected();
        void to_main_stage_connecting();
        void to_main_stage_loading();
        void to_main_stage_active();

        // send utils
        bool send_conn_req();

        // callbacks utils
        bool call_connection_accepted(bool need_file_sync);
        bool call_connection_rejected(const reject_reason& reason);
        bool call_snapshot(int size, const void* data);
    };
}