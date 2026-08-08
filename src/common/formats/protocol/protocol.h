#pragma once


#include "binary/binary.h"

#include <cstdint>
#include <span>


namespace tungsten::protocol
{
	constexpr int		MAX_PACKET_SIZE = 1024;
	constexpr uint64_t	NO_NONCE		= UINT64_MAX;

	static_assert(MAX_PACKET_SIZE > 0, "MAX_PACKET_SIZE overflows int!");


	// Fixed string with explicit size. It does not require null termination.

	template <uint32_t N>
	struct protocol_string // maybe should use fixed_string?
	{
		uint32_t	size;
		char 		data[N];
	};

	// [packet_header][payload]
	using packet_t = uint8_t[MAX_PACKET_SIZE];

	/*
		TODO:
			need to abstract the entire contents of the packages
			so that interaction via interfaces occurs solely through pointers—that is,
			raw byte arrays.
	*/

	enum class protocol_error_type : uint8_t
	{
		none = 0,

		corrupted_packet, // layout violation or var's values
		unexcepted_packet, 
		unknown_packet, // how actually check this?
		serialize_violation,
		// all other/unknown violation 
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



	constexpr size_t MAX_FILES_COUNT	= 512;
	constexpr size_t MAX_FILE_NAME_SIZE = 64;

	using file_name = protocol_string<MAX_FILE_NAME_SIZE>;

	struct file_info
	{
		uint64_t  checksum; // only for check file collisions (if (file1.name == file2.name))
		uint64_t  size;
		file_name name;

	};
	
	/*
		The maximum payload sizes for gameplay packets (usercmd, snapshot, message)
		are no longer strictly defined at the protocol level.

		Since the protocol layer now treats gameplay data as a generic, untyped raw
		byte stream (raw payload) to ensure complete genre-agnostic versatility,
		all responsibility for data packing, size validation, and layout safety
		is deferred entirely to the owner (the game logic).

		The game should define its own bounds within its local domain and validate
		them against the absolute physical network limit: MAX_PACKET_PAYLOAD_SIZE.
	*/

	constexpr binary::bin_endian_type protocol_endian = binary::bin_endian_type::big;

	using protocol_reader = binary::memory_reader<protocol_endian>;
	using protocol_writer = binary::memory_writer<protocol_endian>;
}

