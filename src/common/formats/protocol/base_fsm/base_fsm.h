
#pragma once

#include "../protocol.h"

namespace tungsten::protocol
{
    struct      packet_header;
    enum class  packet_type : uint8_t;

    template <typename F>
    bool pack_packet_to_buffer(
        int         max_size,
        void*       buffer,
        int&        out_total_size,
        uint64_t    timestamp_us,
        packet_type type,
        uint8_t     flags,
        uint64_t    nonce,
        F&&         write_payload_func
    );


    enum class fsm_error_type
    {
        none = 0,

        remote_timeout,

        unknown_signal,
        unknown_packet,
        unknown_stage,

        unexpected_signal,
        unexpected_packet,

        corruted_packet,
        corruted_signal,

        serialize_violation,
        send_violation,
        internal_violation,

        remote_violation,
    };

    struct fsm_error
    {
        fsm_error_type type;
        union
        {
            protocol_error protocol_err;
        };
    };

    typedef void(*on_send_func)        (void* context, bool reliable, int size, const void* data);
    typedef void(*on_error_func)       (void* context, const fsm_error& err);
    typedef void(*on_disconnected_func)(void* context, disconnect_type type, const disconnect_reason& reason);

    struct base_fsm_callbacks
    {
        on_send_func         on_send         = nullptr;
        on_error_func        on_error        = nullptr;
        on_disconnected_func on_disconnected = nullptr;
    };

    class base_fsm
    {
    public:
        base_fsm(void* ctx = nullptr, base_fsm_callbacks callbacks = {});
        virtual ~base_fsm();

        base_fsm(const base_fsm&) = delete;
        base_fsm(base_fsm&&) = delete;
        void operator=(const base_fsm&) = delete;
        void operator=(base_fsm&&) = delete;

        // configs
        void  set_context(void* ctx);
        void* get_context();

        void set_base_callbacks(base_fsm_callbacks callbacks);

        // base interface
        void reset();
        void tick(uint64_t delta_us);
        virtual void disconnect(disconnect_type type, const disconnect_reason& reason) = 0;
        void on_recv(int size, const void* data);

        uint64_t get_timestamp_us() const;
        uint64_t get_delta_us()     const;

    protected:
        // sending/receiving
        uint64_t my_nonce = 0;
        uint64_t remote_nonce = 0;

        // utils
        void error(fsm_error err);

        virtual void on_recv_custom(const packet_header& header, protocol_reader& reader) = 0;
        virtual void on_disconnect(disconnect_type type, const disconnect_reason& reason) = 0;

        virtual void on_reset() = 0;
        virtual void on_tick() = 0;

        // callback utils
        void call_disconnected(disconnect_type type, const disconnect_reason& reason);
        void call_send(bool reliable, int size, const void* data);

        // send utils
        bool send_protocol_error(protocol_error code);
        bool send_disconnect(disconnect_type type, const disconnect_reason& reason);

        template <typename F>
        bool send_packet_generic(bool reliable, packet_type type, uint8_t flags, F&& write_payload_func)
        {
            packet_t packet;
            int total_size = 0;

            if (!pack_packet_to_buffer(
                sizeof(packet),
                &packet,
                total_size,
                get_timestamp_us(),
                type,
                flags,
                remote_nonce,
                std::forward<F>(write_payload_func)
            )) {
                error(fsm_error{ fsm_error_type::serialize_violation });
                return false;
            }

            call_send(reliable, total_size, &packet);
            return true;
        }

    private:
        //  timing:
        uint64_t    curr_timestamp_us = 0;
        uint64_t    curr_delta_us = 0;

        void        update_timing(uint64_t delta_us);
        void        reset_timing();

        //  callbacks:
        void* context = nullptr;
        base_fsm_callbacks cmn_callbacks{};

        void call_error(const fsm_error& err);

        //  on recv utils:
        void on_recv_error     (const packet_header& header, protocol_reader& reader);
        void on_recv_disconnect(const packet_header& header, protocol_reader& reader);

        void pure_reset();
    };
}