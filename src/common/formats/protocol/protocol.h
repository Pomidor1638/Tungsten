#pragma once

#include <cstdint>
#include "container/static_span.h"
#include "../../utils/binary/binary.h"

namespace tungsten::protocol
{
	constexpr size_t	MAX_PACKET_SIZE = 1024;
	constexpr uint64_t	NO_NONCE		= UINT64_MAX;

	// Fixed string with explicit size. It does not require null termination.

	template <uint32_t N>
	struct protocol_string
	{
		uint32_t	size;
		char 		data[N];
	};

	using data_span  = util::container::static_span<const void>;

	/*
		TODO:
			need to abstract the entire contents of the packages
			so that interaction via interfaces occurs solely through pointers—that is,
			raw byte arrays.
	*/

	enum class protocol_error : uint8_t
	{
		none = 0,

		corrupted_packet,
		unexcepted_packet,
		unknown_packet,
		// all not known violation 
		remote_violation,
	};

	constexpr size_t MAX_REJECT_REASON_SIZE = 256;
	using reject_reason = protocol_string<MAX_REJECT_REASON_SIZE>;

	constexpr size_t MAX_DISCONNECT_REASON_SIZE = 512;
	using disconnect_reason = protocol_string<MAX_DISCONNECT_REASON_SIZE>;

	enum class disconnect_type : uint8_t
	{
		none = 0,

		kicked,
		banned,
		shutting_down,
		closed,

		other,
	};

	using protocol_reader = util::binary::memory_reader<util::binary::bin_endian_type::big>;
	using protocol_writer = util::binary::memory_writer<util::binary::bin_endian_type::big>;
}

