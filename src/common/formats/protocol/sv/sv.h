
#pragma once

#include "../base_fsm/base_fsm.h"
#include "../protocol.h"

namespace tungsten::protocol
{
    enum class server_main_stage
    {
        empty = 0,

        loading,
        active,
    };

    enum class server_loading_stage
    {
        idle = 0,
    };

    enum class server_file_sync_stage
    {
        idle = 0,
    };

    enum class server_level_sync_stage
    {
        idle = 0,
    };

    enum class server_snapshot_sync_stage
    {
        idle = 0,
    };


    typedef bool(*on_usercmd_func)(void* ctx, int size, const void* data);
    //typedef bool(*on_recv_message_func)(void* ctx, int size, const void* data);

    struct server_fsm_callbacks
    {
        on_usercmd_func on_recv_usercmd = nullptr;
        //on_recv_message_func on_recv_message = nullptr;
    };


    class server_client_fsm final : public base_fsm
    {
    public:

        server_client_fsm(void* ctx = nullptr, base_fsm_callbacks cmn_callbacks = {}, server_fsm_callbacks sv_callbacks = {});

        void set_server_callbacks(server_fsm_callbacks callbacks);

        void open(uint64_t cl_nonce, uint64_t sv_nonce, bool need_file_sync);
        void disconnect(disconnect_type type, const disconnect_reason& reason) override;

        static bool is_conn_req(bool& bad_version, uint64_t& cl_nonce, int size, const void* data);
        static bool reject(uint64_t timestamp_us, uint64_t cl_nonce, const reject_reason& reason, int max_size, int& out_size, void* out_data);

        void snapshot(int size, const void* data);
        //void message(int size, const void* data);

    private:

        void pure_reset();

        // utils
        bool send_conn_accept(bool need_filesync);

        // virtual funcs overrides
        void on_reset() override;
        void on_tick()  override;

        void on_disconnect(disconnect_type type, const disconnect_reason& reason) override;

        void on_recv_custom       (const packet_header& header, protocol_reader& reader) override;
        // on_recv_* funcs        
        void on_recv_loading      (const packet_header& header, protocol_reader& reader);
                                  
        void on_recv_active       (const packet_header& header, protocol_reader& reader);
            void process_usercmd  (const packet_header& header, protocol_reader& reader);
        //  void process_message  (const packet_header& header, protocol_reader& reader);
        
        void on_recv_disconnecting(const packet_header& header, protocol_reader& reader);

        server_fsm_callbacks sv_callbacks{};

        server_main_stage 			main_stage          = server_main_stage         ::empty;
        server_loading_stage 		loading_stage       = server_loading_stage      ::idle;
        server_file_sync_stage 		file_sync_stage     = server_file_sync_stage    ::idle;
        server_level_sync_stage 	gamesync_stage      = server_level_sync_stage   ::idle;
        server_snapshot_sync_stage 	snapshot_sync_stage = server_snapshot_sync_stage::idle;

        bool call_on_recv_usercmd(int size, const void* data);
        //bool call_on_recv_message(int size, const void* data);

        void to_main_stage_empty();
        void to_main_stage_loading();
        void to_main_stage_active();

    };
}