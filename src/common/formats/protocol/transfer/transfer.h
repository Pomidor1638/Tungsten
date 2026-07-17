
#pragma once

#include <cstdint>
#include <cstddef>

namespace tungsten::transfer
{
    constexpr size_t MAX_PACKET_SIZE = 1024; // same as MAX_PAACKET_PAYLOAD_SIZE from protocol

#pragma pack(push, 1)

    enum class pkt_type : uint8_t
    {
        none = 0,

        begin,
        chunk,
        ack,
        end,
        error,

    };

    struct packet_header
    {
        pkt_type type;
		uint16_t payload_size;
		uint64_t checksum;
    };

    constexpr size_t PACKET_HEADER_SIZE 		= sizeof(packet_header);
    constexpr size_t MAX_PACKET_PAYLOAD_SIZE 	= MAX_PACKET_SIZE - PACKET_HEADER_SIZE;

    struct packet
    {
        packet_header header;
        uint8_t payload[MAX_PACKET_PAYLOAD_SIZE];
    };

	struct packet_begin
    {
        uint64_t total_size;
        uint16_t chunk_size;
    };

	struct chunk_header
	{
		uint64_t checksum;
		uint64_t offset;
		uint16_t size;
	};

	constexpr size_t MAX_CHUNK_SIZE = MAX_PACKET_PAYLOAD_SIZE - sizeof(chunk_header);
	constexpr size_t MIN_CHUNK_SIZE = 1;

	struct packet_chunk
    {
        chunk_header header;
        uint8_t data[MAX_CHUNK_SIZE];
    };

	struct packet_ack
    {
        uint64_t next_offset;
    };

	struct packet_end
    {
        uint64_t checksum;
    };

	static_assert(sizeof(packet      ) == MAX_PACKET_SIZE, 			"transfer packet violation: sizeof(packet      ) == MAX_PACKET_SIZE"		);
	static_assert(sizeof(packet_begin) <= MAX_PACKET_PAYLOAD_SIZE, 	"transfer packet violation: sizeof(packet_begin) >  MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_chunk) <= MAX_PACKET_PAYLOAD_SIZE, 	"transfer packet violation: sizeof(packet_chunk) >  MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_ack  ) <= MAX_PACKET_PAYLOAD_SIZE, 	"transfer packet violation: sizeof(packet_ack  ) >  MAX_PACKET_PAYLOAD_SIZE");
	static_assert(sizeof(packet_end  ) <= MAX_PACKET_PAYLOAD_SIZE, 	"transfer packet violation: sizeof(packet_end  ) >  MAX_PACKET_PAYLOAD_SIZE");

#pragma pack(pop)

	uint64_t checksum_bytes(size_t size, const void* data);
	
	bool make_packet(packet& out, pkt_type type, int size, const void* data);
	bool validate_packet(const packet& p);

	enum class tx_stage
	{
		idle = 0,
		waiting_begin_ack,
		pending_chunk,
		waiting_chunk_ack,
		waiting_end_ack,
	};

	/*
		tx_block sequence

		stages:
			idle
			waiting_begin_ack
			pending_chunk
			waiting_chunk_ack
			waiting_end_ack

		idle:
			begin(total_size, chunk_size):
				// if total_size == 0, maybe should allow this, 
				// or should make upper boud like MAX_TOTAL_SIZE ? 
				if total_size == 0: 
					error(bad_total_size)

				if (chunk_size < MIN_CHUNK_SIZE || chunk_size > MAX_CHUNK_SIZE):
					error(bad_chunk_size) // for avoid ifinity loop

				reset_progress();
				setup_progress(total_size, chunk_size);
				send_begin()

				-> waiting_begin_ack
			else:
				error(bad_stage)

		waiting_begin_ack:
			recv_ack(ack):
				// why not ack->next_offset != next_offset ?
				if (ack.next_offset != 0):
					error(bad_ack)

				check_progress()
			recv else
				error(bad_packet)
			else:
				error(bad_stage)

		pending_chunk:
			chunk_ready(chunk):

				if chunk.offset != next_offset:
					error(bad_chunk_offset)

				expected_size = min(chunk_size, total_size - next_offset)
				if chunk.size != expected_size:
					error(bad_chunk_size)

				emit send_chunk(chunk)
				-> waiting_chunk_ack
			else:
				error(bad_stage)

		waiting_chunk_ack:
			recv_ack(ack):
				expected_next = next_offset + min(chunk_size, total_size - next_offset)

				if ack.next_offset != expected_next:
					error(bad_ack)

				next_offset = ack.next_offset
				check_progress()
			recv else:
				error(bad_packet)
			else:
				error(bad_stage)

		waiting_end_ack:
			recv_ack(ack):
				if ack.next_offset != total_size:
					emit error(bad_ack)
					-> idle

				emit complete
				-> idle

		func check_progress():
			if next_offset < total_size:
				expected_size = min(chunk_size, total_size - next_offset)
				emit need_chunk(next_offset, expected_size)
				-> pending_chunk
			else:
				emit send_end
				-> waiting_end_ack
		
		func error(e):
			emit_error(e)
			-> idle
	*/

	enum class tx_action_type
	{
		none = 0,

		send,
		need_chunk,
		error,
		complete
	};

	struct tx_action_need_chunk
	{
		uint64_t offset = 0;
		uint16_t size 	= 0;
	};

	struct tx_action_send
	{
		int 		size;
		const void* data;
	};

	enum class tx_error
	{
		none = 0,

		bad_total_size,
		bad_chunk_offset,
		bad_chunk_size,
		bad_ack,
		bad_packet,
		bad_stage,
		bad_send_size,
	};

	struct tx_action_error
	{
		tx_error code;
	};

	struct tx_action_complete
	{
	};

	struct tx_action
	{
		tx_action_type type = tx_action_type::none;
		union
		{
			tx_action_send 			send;
			tx_action_error 		error;
			tx_action_need_chunk 	need_chunk;
			tx_action_complete		complete;
		};
	};

	class tx_fsm
	{
	public:

		void reset();
		tx_action pull_action();
		bool on_recv(const packet& p);
		bool begin(uint64_t total_size, uint16_t chunk_size);
		bool chunk(uint64_t offset, int size, const void* data);

		tx_fsm();
		tx_fsm(const tx_fsm&) = delete;
		tx_fsm(tx_fsm&&) = delete;
		void operator=(const tx_fsm&) = delete;
		void operator=(tx_fsm&&) = delete;
		~tx_fsm() = default;

	private:

		void reset_progress();
		void setup_progress(uint64_t total_size, uint16_t chunk_size);
		void check_progress();

		uint16_t get_expected_size() const;

		bool check_ack(const packet& p);

		bool on_recv_waiting_begin_ack(const packet& p);
		bool on_recv_waiting_chunk_ack(const packet& p);
		bool on_recv_waiting_end_ack(const packet& p);

		void emit_error(tx_error err);
		bool emit_send(pkt_type type, int size, const void* data);
		void emit_need_chunk(uint64_t next_offset, uint16_t expected_size);
		void emit_complete();

		void error(tx_error e);
		void complete();

		void send_begin();
		void send_chunk(int size, const void* data);
		void send_end();

		tx_stage stage = tx_stage::idle;

		tx_action action{};
		packet tx_packet;

		uint64_t tx_total_size 			= 0;
		uint64_t next_offset 			= 0;
		uint16_t expected_chunk_size 	= MAX_CHUNK_SIZE;
	};

	
	enum class rx_stage
	{
		idle = 0,
		waiting_begin,
		pending_begin_accept,
		waiting_chunk,
		pending_chunk_commit,
		waiting_end,
		pending_end_commit,
	};

	/*
		stages:
			idle
			waiting_begin
			pending_begin_accept
			waiting_chunk
			pending_chunk_commit
			waiting_end
			pending_end_commit

		idle:
			start():
				-> waiting_begin
			else:
				error(bad_stage)

		waiting_begin:
			recv begin:

				if (begin.total_size == 0)
					error(bad_total_size)
				
				if (begin.chunk_size < MIN_CHUNK_SIZE || begin.chunk_size > MAX_CHUNK_SIZE)
					error(bad_chunk_size)
				
				reset_progress()
				setup_progress(begin.total_size, begin.chunk_size)
				emit offer(begin.total_size, begin.chunk_size)
				-> pending_begin_accept
			recv else:
				error(bad_packet)
			else:
				error(bad_stage)

		pending_begin_accept:
			offer_accept():
				emit send_ack(next_offset)
				check_progress()
			else:
				error(bad_stage)
		
		waiting_chunk:
			recv chunk:
				expected_size = min(total_size - next_offset, exprected_chunk_size)
				if (chunk.size != expected_size)
					error(bad_chunk_size)
				if (chunk.offset != next_offset)
					error(bad_chunk_offset)
				if (chunk.checksum != checksum_bytes(chunk.size, chunk.data))
					error(rx_error::bad_chunk_checksum);
				emit chunk(chunk.offset, chunk.size, chunk.data)
				-> pending_chunk_commit
			recv else:
				error(bad_packet)
			else:
				error(bad_stage)

		pending_chunk_commit:
			chunk_commit():
				emit send_ack(next_offset)
				check_progress()
			else:
				error(bad_stage)

		waiting_end:
			recv end:
				if (end.checksum != total_checksum)
					error(bad_total_checksum)
				emit end_commit()
				-> pending_end_accept
			recv else:
				error(bad_packet)
			else:
				error(bad_stage)
		
		pending_end_commit:
			end_commit():
				emit send_ack(next_offset)
				-> idle	
			else:
				error(bad_stage)
			

		func check_progress():
			if next_offset < total_size:
				-> waiting_chunk
			else:
				-> waiting_end
	*/

	enum class rx_action_type
	{
		none = 0,

		send,
		error,
		offer,
		chunk,
		end,
	};

	struct rx_action_send
	{
		int 		size;
		const void* data;
	};

	enum class rx_error
	{
		none = 0,

		bad_total_size,
		bad_total_checksum,
		bad_chunk_offset,
		bad_chunk_checksum,
		bad_chunk_size,
		bad_ack,
		bad_packet,
		bad_stage,
		bad_send_size,
	};

	struct rx_action_error
	{
		rx_error code;
	};

	struct rx_action_offer
	{
		uint64_t total_size;
		uint16_t chunk_size;
	};

	struct rx_action_chunk
	{
		uint64_t 	offset;
		int 		size;
		const void* data;
	};

	struct rx_action_end
	{
		//uint64_t checksum;
	};

	struct rx_action
	{
		rx_action_type type = rx_action_type::none;
		union
		{
			rx_action_error error;
			rx_action_send 	send;
			rx_action_offer	offer;
			rx_action_chunk chunk;
			rx_action_end	end;
		};
	};

	class rx_fsm
	{
	public:

		rx_action pull_action();

		void reset();

		bool on_recv(const packet& p);

		void start();
		void offer_accept();
		void chunk_commit();
		void end_commit();


		rx_fsm();
		rx_fsm(const rx_fsm&) = delete;
		rx_fsm(rx_fsm&&) = delete;
		void operator=(const rx_fsm&) = delete;
		void operator=(rx_fsm&&) = delete;
		~rx_fsm() = default;

	private:

		bool on_recv_waiting_begin(const packet& p);
		bool on_recv_waiting_chunk(const packet& p);
		bool on_recv_waiting_end  (const packet& p);

		void reset_progress();
		void setup_progress(uint64_t total_size, uint16_t chnk_size);
		void check_progress();

		void emit_error(rx_error err);
		bool emit_send(pkt_type type, int size, const void* data);
        void emit_offer();
		void emit_chunk(uint64_t offset, uint16_t size, const void* data);
		void emit_end_commit();

		void error(rx_error e);
		void send_ack();

		uint16_t get_expected_size() const;
		
		uint64_t next_offset 		 = 0;
		uint64_t rx_total_size 		 = 0;
		uint16_t expected_chunk_size = 0;
		uint64_t total_checksum 	 = 0;

		packet rx_packet{};

		rx_action action{};
		rx_stage stage = rx_stage::idle;
	};
};