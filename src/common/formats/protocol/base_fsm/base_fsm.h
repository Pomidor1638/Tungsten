
#pragma once

#include "../protocol.h"

namespace tungsten::protocol
{
    struct      packet_header;
    enum class  packet_type : uint8_t;

    enum class fsm_error_type
    {
        none = 0,

        remote_timeout,

        unknown_signal,
        unknown_packet,
        unknown_stage,

        unexcepted_signal,
        unexpected_packet,

        corruted_packet,
        corruted_signal,

        make_violation,
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

    typedef void(*on_send_func)             (void* context, const data_span& data);
    typedef void(*on_error_func)            (void* context, const fsm_error& err);
    typedef void(*on_protocol_error_func)   (void* context, protocol_error error);
    typedef void(*on_disconnected_func)     (void* context, disconnect_type type, const disconnect_reason& reason);

    struct base_fsm_callbacks
    {
        on_send_func            on_send = nullptr;
        on_error_func           on_error = nullptr;
        on_protocol_error_func  on_protocol_error = nullptr;
        on_disconnected_func    on_disconnected = nullptr;
    };

    struct timeout_config
    {
        uint64_t interval_us = 5'000'000; // 5 s.
    };

    class base_fsm
    {
    public:
        base_fsm(void* ctx = nullptr, timeout_config cfg = {}, base_fsm_callbacks callbacks = {});

        virtual ~base_fsm();

        base_fsm(const base_fsm&) = delete;
        base_fsm(base_fsm&&) = delete;
        void operator=(const base_fsm&) = delete;
        void operator=(base_fsm&&) = delete;

        // configs
        void set_context(void* ctx);
        void set_common_callbacks(base_fsm_callbacks callbacks);
        void set_timeout_config(timeout_config cfg);

        // base interface
        void reset();
        void tick(uint64_t delta_us);
        virtual void disconnect(disconnect_type type, disconnect_reason reason) = 0;
        void on_recv(const data_span& data);

        uint64_t get_timestamp_us() const;
        uint64_t get_delta_us()     const;

    protected:

        // sending/receiving
        uint64_t my_nonce = 0;
        uint64_t remote_nonce = 0;

        // utils
        void error(fsm_error err, bool send_err);

        void arm_timeout();
        void disarm_timeout();
        void touch_timeout();

        bool get_timeout_tracking() const;

        bool make_packet_header_internal(packet_header& out, packet_type type, uint8_t flags);

        virtual bool on_recv_custom(const packet_header& header, protocol_reader& reader) = 0;
        virtual void on_reset() = 0;
        virtual void on_tick() = 0;

        virtual bool on_disconnect_req(disconnect_type type, const disconnect_reason& reason) = 0;
        virtual bool on_disconnect_ack() = 0;

        bool call_disconnected(disconnect_type type, const disconnect_reason& reason);

    private:

        void fatal_error(fsm_error err);

        //  timing:
        uint64_t    curr_timestamp_us = 0;
        uint64_t    curr_delta_us = 0;

        void        update_timing(uint64_t delta_us);
        void        reset_timing();

        //  timeouts:
        timeout_config timeout_cfg{};

        bool timeout_tracking = false;
        uint64_t last_recv_timestamp_us = 0;

        bool check_timeout();
        void reset_timeout();
        void timeout();

        //  callbacks:
        void* context = nullptr;
        base_fsm_callbacks cmn_callbacks{};

        bool call_error(const fsm_error& err);
        bool call_send(const data_span& data);
        bool call_protocol_error(protocol_error protocol_err);

        //  utils:
        bool on_recv_error         (const packet_header& header, protocol_reader& reader);
        bool on_recv_disconnect_ack(const packet_header& header, protocol_reader& reader);
        bool on_recv_disconnect_req(const packet_header& header, protocol_reader& reader);

        bool send_protocol_error(protocol_error code);

        void pure_reset();
    };
}