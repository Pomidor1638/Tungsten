#pragma once

#include <cstdint>
#include "container/static_span.h"

namespace tungsten::protocol 
{
	constexpr size_t MAX_PACKET_SIZE 			= 1200;
	
	// Fixed string with explicit size. It does not require null termination.
	template <int N>
	struct fixed_string
	{
		uint16_t	size;
		char 		data[N];
	};

    struct packet;
    using byte_span = util::container::static_span<uint8_t>;

	/*
		TODO:
			need to abstract the entire contents of the packages 
			so that interaction via interfaces occurs solely through pointers—that is,
			raw byte arrays.
	*/

	/*
		I’d like to create two different event types—one 
		for the server-side client slot and one for the 
		client itself—but that would result in a lot of 
		duplicate shared events, similar to what happened 
		with the transfer FSMs.
	*/
	enum class event_type
	{
		none = 0,
    // common events
		send,
		error,
        disconnected, /*it is very strange that the client can state the reason for the disconnection.*/
	// server events
		connection_canceled,
	// client events
		connection_accepted,
		connection_rejected,
	};

	struct event_send
	{
		bool 	reliable;
		int 	size;
		uint8_t data[MAX_PACKET_SIZE];
	};

	enum class event_error_type
	{
		none = 0,

		bad_stage,
		bad_packet,

		protocol_error,
	};

	enum class protocol_error : uint8_t
	{
		none = 0,
		timeout,

		// bad packet contents
		packet_violation,

		// wrong packet
		unexcepted_packet,
		// all not known violation 
		remote_violation,
	};

	struct event_error
	{
		event_error_type type;
		protocol_error protocol;
	};

	struct event_conn_requested
	{
		/*
			maybe we should add something here?s
			the client's name or something like that?
		*/
	};

	struct event_conn_accepted
	{
		/*
			It feels like something should be added here, 
			but for now, let's leave it as is
		*/
		bool need_file_sync;
	};
	
	constexpr size_t MAX_REJECT_REASON_SIZE = 256;
	using reject_reason = fixed_string<MAX_REJECT_REASON_SIZE>;
	struct event_conn_rejected
	{
		reject_reason reason;
	};

	struct event_conn_cancel 
	{
		/*
			So, the client can state the reason for the disconnection 
			but cannot state the reason for the cancellation?
		*/
    };

	constexpr size_t MAX_DISCONNECT_REASON_SIZE = 512;
	using disconnect_reason = fixed_string<MAX_DISCONNECT_REASON_SIZE>;

    enum class disconnect_type : uint8_t
    {
        none = 0,

        kicked,
        banned,
        shutting_down,
    };

    struct event_disconnected
    {
        disconnect_type     type;
        disconnect_reason   reason;
    };

	struct event
	{
		event_type type;
		// It seems a bit dangerous, but it’s actually very convenient, isn’t it?
		union
		{
			event_send 				send;
			event_error 			error;
			event_conn_requested 	conn_requested;
			event_conn_cancel		conn_cancel;
			event_conn_accepted 	conn_accepted;
			event_conn_rejected 	conn_rejected;
            event_disconnected      disconnected;
		};
	};

}

