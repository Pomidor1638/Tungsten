#include "transfer.h"
#include <algorithm>
#include <cstdint>
#include <cstring>


namespace tungsten::transfer
{
	void tx_fsm::reset_progress()
	{
		tx_total_size		= 0;
		next_offset			= 0;
		expected_chunk_size	= MAX_CHUNK_SIZE;
	}

	void tx_fsm::reset()
	{
		stage		= tx_stage		::idle;
		action.type	= tx_action_type::none;
		
		reset_progress();
	}

	tx_action tx_fsm::pull_action()
	{
		tx_action act = action;
		action.type = tx_action_type::none;
		return act;
	};

	bool tx_fsm::on_recv(const packet& p)
	{
		if (validate_packet(p))
		{
			switch (stage) 
			{
			case tx_stage::waiting_begin_ack:
				return on_recv_waiting_begin_ack(p);
			case tx_stage::waiting_chunk_ack:
				return on_recv_waiting_chunk_ack(p);
			case tx_stage::waiting_end_ack:
				return on_recv_waiting_end_ack(p);
			default:
				error(tx_error::bad_stage);
				break;
			}
		}
		// Is this necessary?
		// Maybe invalid packets should be ignored?
		// Let it be so as long as it doesn't interfere
		else
		{
			error(tx_error::bad_packet);
		}
		return false;
	}

	
	void tx_fsm::send_begin()
	{
		packet_begin beg
		{
			.total_size = tx_total_size,
			.chunk_size = expected_chunk_size
		};

		emit_send(
			pkt_type::begin,
			sizeof(beg), 
			&beg
		);
	}
	


	void tx_fsm::setup_progress(uint64_t total_size, uint16_t chunk_size)
	{
		tx_total_size 		= total_size;
		expected_chunk_size = chunk_size;
	}

	bool tx_fsm::begin(uint64_t total_size, uint16_t chunk_size)
	{
		// check stage
		if (stage != tx_stage::idle)
		{
			error(tx_error::bad_stage);
			return false;
		}

		// if total_size == 0, maybe should allow this, 
		// or should make upper boud like MAX_TOTAL_SIZE ? 
		if (!total_size)
		{
			error(tx_error::bad_total_size);
			return false;
		}
		
		// for avoid ifinity loop
		if (chunk_size < MIN_CHUNK_SIZE || chunk_size > MAX_CHUNK_SIZE)
		{
			error(tx_error::bad_chunk_size);
			return false;
		}
		// reset counters
		reset_progress();
		// setup counters and expected_size
		setup_progress(total_size, chunk_size);
		send_begin();

		stage = tx_stage::waiting_begin_ack;
		return true;
	}

	
	void tx_fsm::send_chunk(int size, const void* data)
	{
		packet_chunk ch
		{
			.header 
			{
				.offset = next_offset,
				.size = static_cast<uint16_t>(size)
			},
		};

		memcpy(ch.data, data, size);

		if (!emit_send(
			pkt_type::chunk,
			sizeof(chunk_header) + size,//sizeof(ch), 
			&ch
		)){
			// maybe need emit error
			stage = tx_stage::idle;
		}
	}

	bool tx_fsm::chunk(uint64_t offset, int size, const void* data)
	{
		if (stage != tx_stage::pending_chunk)
		{
			error(tx_error::bad_stage);
			return false;
		}

		if (offset != next_offset)
		{
			error(tx_error::bad_chunk_offset);
			return false;
		}

		if (size != get_expected_size())
		{
			error(tx_error::bad_chunk_size);
			return false;
		}

		send_chunk(size, data);
		stage = tx_stage::waiting_chunk_ack;
		return true;
	}

	tx_fsm::tx_fsm()
	{
		reset();
	}

	
	bool tx_fsm::check_ack(const packet& p)
	{
		if (p.header.type != pkt_type::ack || p.header.payload_size != sizeof(packet_ack))
		{
			error(tx_error::bad_packet);
			return false;
		}

		auto* ack = reinterpret_cast<const packet_ack*>(p.payload);
		if (ack->next_offset != next_offset) 
		{
			error(tx_error::bad_ack);
			return false;
		}
		return true;
	}

	bool tx_fsm::on_recv_waiting_begin_ack(const packet& p)
	{
		if (!check_ack(p))
			return false;
		check_progress();
		return true;
	}

	bool tx_fsm::on_recv_waiting_chunk_ack(const packet& p)
	{
		if (p.header.type != pkt_type::ack || p.header.payload_size != sizeof(packet_ack))
		{
			error(tx_error::bad_packet);
			return false;
		}

		auto* ack = reinterpret_cast<const packet_ack*>(p.payload);
		if (ack->next_offset != next_offset + get_expected_size()) 
		{
			error(tx_error::bad_ack);
			return false;
		}

		next_offset = ack->next_offset;		
		check_progress();
		return true;
	}

	
	void tx_fsm::emit_complete()
	{
		action.type = tx_action_type::complete;
	}

	
	void tx_fsm::complete()
	{
		emit_complete();
		stage = tx_stage::idle;
	}

	bool tx_fsm::on_recv_waiting_end_ack(const packet& p)
	{
		if (p.header.type != pkt_type::ack || p.header.payload_size != sizeof(packet_ack))
		{
			error(tx_error::bad_packet);
			return false;
		}

		auto* ack = reinterpret_cast<const packet_ack*>(p.payload);
		if (ack->next_offset != tx_total_size) 
		{
			error(tx_error::bad_ack);
			return false;
		}

		complete();
		return true;
	}

	void tx_fsm::send_end()
	{
		packet_end end
		{
			.checksum = 0 // need to fix this thing
		};

		emit_send
		(
			pkt_type::end,
			sizeof(end),
			&end
		);
	}

	
	uint16_t tx_fsm::get_expected_size() const
	{
		// MAX_PACKET_SIZE always < 64kib
		// MAX_CHUNK_SIZE always < MAX_PACKET_SIZE
		// -> MAX_CHUNK_SIZE always < 64kib 
		// any uint16_t < 64kib
		// it should be ok
		return static_cast<uint16_t>(
			std::min<uint64_t>(
				expected_chunk_size,
				tx_total_size - next_offset
			)
		);
	}

	void tx_fsm::check_progress()
	{
		if (next_offset < tx_total_size)
		{
			emit_need_chunk(next_offset, get_expected_size());
			stage = tx_stage::pending_chunk;
			return;
		}

		send_end();
		stage = tx_stage::waiting_end_ack;
	}

	void tx_fsm::emit_error(tx_error err)
	{
		action.type = tx_action_type::error;
		action.error.code = err;
    }

	
	void tx_fsm::error(tx_error e)
	{
		emit_error(e);
		stage = tx_stage::idle;
	}

	
	void tx_fsm::emit_need_chunk(uint64_t next_offset, uint16_t expected_size)
	{
		action.type = tx_action_type::need_chunk;
		tx_action_need_chunk& need_chunk = action.need_chunk;

		need_chunk.offset 	= next_offset;
		need_chunk.size 	= expected_size;
	}

	bool tx_fsm::emit_send(pkt_type type, int size, const void* data)
	{
		using enum tx_stage;
        switch (stage) 
        {
        case idle:
        case pending_chunk:
        case waiting_chunk_ack:
            if (size > MAX_PACKET_PAYLOAD_SIZE || size < 0)
			{
            	error(tx_error::bad_send_size);
				return false;
			}
			break;
        default:
            error(tx_error::bad_stage);
            return false;
        }

		make_packet(tx_packet, type, size, data);

		action.type = tx_action_type::send;
		tx_action_send& send = action.send;

		send.data = &tx_packet;
		send.size = PACKET_HEADER_SIZE + tx_packet.header.payload_size;

		return true;
	}
}