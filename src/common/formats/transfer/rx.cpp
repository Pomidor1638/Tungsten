
#include "transfer.h"
#include <algorithm>
#include <cstdint>
#include <cstring>

namespace tungsten::transfer
{
    
    
	void rx_fsm::reset_progress()
    {
        next_offset 	    = 0;
        rx_total_size 	    = 0;
        expected_chunk_size = 0;
		total_checksum 	    = 0;
    }

	void rx_fsm::setup_progress(uint64_t total_size, uint16_t chnk_size)
    {
        rx_total_size = total_size;
        expected_chunk_size = chnk_size;
    }

	void rx_fsm::check_progress()
    {
        stage = (next_offset < rx_total_size) ? 
            rx_stage::waiting_chunk : 
            rx_stage::waiting_end   ;
    }

	void rx_fsm::reset()
    {
        stage       = rx_stage::idle;
		action.type = rx_action_type::none;

        reset_progress();
    }

	rx_fsm::rx_fsm()
    {
        reset();
    }


    rx_action rx_fsm::pull_action()
    {
        rx_action act = action;
        action.type = rx_action_type::none;
        return act;
    }

    
    
	void rx_fsm::emit_error(rx_error err)
    {
		action.type = rx_action_type::error;
		action.error.code = err;
    }

	bool rx_fsm::emit_send(pkt_type type, int size, const void* data)
    {
        using enum rx_stage;
        switch (stage) 
        {
        case pending_begin_accept:
        case pending_chunk_commit:
        case pending_end_commit:
            if (size > MAX_PACKET_PAYLOAD_SIZE || size < 0)
            {
                error(rx_error::bad_send_size);
				return false;
            }
            break;
        default:
            error(rx_error::bad_stage);
            return false;
        }

		make_packet(rx_packet, type, size, data);

		action.type = rx_action_type::send;
		rx_action_send& send = action.send;

		send.data = &rx_packet;
		send.size = PACKET_HEADER_SIZE + rx_packet.header.payload_size;

		return true;
    }

    void rx_fsm::emit_offer()
    {
        action.type = rx_action_type::offer;
        rx_action_offer& offer = action.offer;

        offer.total_size = rx_total_size;
        offer.chunk_size = expected_chunk_size;
    }

	void rx_fsm::error(rx_error e)
    {
		emit_error(e);
		stage = rx_stage::idle;
    }

	bool rx_fsm::on_recv_waiting_begin(const packet& p)
    {
        if (p.header.type != pkt_type::begin || p.header.payload_size != sizeof(packet_begin))
        {
            error(rx_error::bad_packet);
            return false;
        }

        auto* beg = reinterpret_cast<const packet_begin*>(p.payload);

        // is this neccessary?
        // why it can't transmit 0 bytes ?
        if (!beg->total_size)
        {
            error(rx_error::bad_total_size);
            return false;
        }

        if (beg->chunk_size < MIN_CHUNK_SIZE || beg->chunk_size > MAX_CHUNK_SIZE)
        {
            error(rx_error::bad_chunk_size);
            return false;
        }

        reset_progress();
        setup_progress(beg->total_size, beg->chunk_size);
        emit_offer();

        stage = rx_stage::pending_begin_accept;
        return true;
    }

    
	uint16_t rx_fsm::get_expected_size() const
	{
		// MAX_PACKET_SIZE always < 64kib
		// MAX_CHUNK_SIZE always < MAX_PACKET_SIZE
		// -> MAX_CHUNK_SIZE always < 64kib 
		// any uint16_t < 64kib
		// it should be ok

		return static_cast<uint16_t>(
			std::min<uint64_t>(
				expected_chunk_size,
				rx_total_size - next_offset
			)
		);
	}
	
	void rx_fsm::emit_end_commit()
    {
        action.type = rx_action_type::end;
        rx_action_end& end = action.end;
        // end.checksum = total_checksum;
    }
    
	void rx_fsm::emit_chunk(uint64_t offset, uint16_t size, const void* data)
    {
        action.type = rx_action_type::chunk;
        rx_action_chunk& chunk = action.chunk;

        
        memcpy(rx_packet.payload, data, size);

        chunk.offset = offset;
        chunk.size = size;
        chunk.data = rx_packet.payload;
    }

    bool rx_fsm::on_recv_waiting_chunk(const packet& p)
    {
        if (p.header.type != pkt_type::chunk 
        ||  p.header.payload_size < sizeof(chunk_header))
        {
            error(rx_error::bad_packet);
            return false;
        }

        auto* chunk = reinterpret_cast<const packet_chunk*>(p.payload);
        const chunk_header& chnk_h = chunk->header;

        if (p.header.payload_size != sizeof(chunk_header) + chnk_h.size)
        {
            error(rx_error::bad_packet);
            return false;
        }

        if (chnk_h.size != get_expected_size())
        {
            error(rx_error::bad_chunk_size);
            return false;
        }
        if (chnk_h.offset != next_offset)
        {
            error(rx_error::bad_chunk_offset);
            return false;
        }
        if (chnk_h.checksum != checksum_bytes(chunk->header.size, chunk->data))
        {
            error(rx_error::bad_chunk_checksum);
            return false;
        }
        
        next_offset += chnk_h.size;
        emit_chunk(chnk_h.offset, chnk_h.size, chunk->data);
        stage = rx_stage::pending_chunk_commit;
        return true;
    }
	
    bool rx_fsm::on_recv_waiting_end(const packet& p)
    {        
        if (p.header.type != pkt_type::end || p.header.payload_size != sizeof(packet_end))
        {
            error(rx_error::bad_packet);
            return false;
        }

        auto* end = reinterpret_cast<const packet_end*>(p.payload); 

        if (end->checksum != total_checksum)
        {
            error(rx_error::bad_total_checksum);
            return false;
        }
        emit_end_commit();
        stage = rx_stage::pending_end_commit;
        return true;
    }

	bool rx_fsm::on_recv(const packet& p)
    {
        if (validate_packet(p))
        {
            switch (stage) 
            {
		    case rx_stage::waiting_begin:
                return on_recv_waiting_begin(p);
		    case rx_stage::waiting_chunk:
                return on_recv_waiting_chunk(p);
		    case rx_stage::waiting_end:
                return on_recv_waiting_end(p);
            default:
                error(rx_error::bad_stage);
                break;
            }
        }
        else
        {
            error(rx_error::bad_packet);
        }
        return false;
    }

	void rx_fsm::start()
    {
        if (stage != rx_stage::idle)
            error(rx_error::bad_stage);
        else
            stage = rx_stage::waiting_begin;
    }

    void rx_fsm::send_ack()
    {
        packet_ack ack 
        {
            .next_offset = next_offset
        };
        emit_send(pkt_type::ack, sizeof(ack), &ack);
    }

	void rx_fsm::offer_accept()
    {
        if (stage != rx_stage::pending_begin_accept)
        {
            error(rx_error::bad_stage);
        }
        else
        {
            send_ack();
            check_progress();
        }
    }

	void rx_fsm::chunk_commit()
    {
        if (stage != rx_stage::pending_chunk_commit)
        {
            error(rx_error::bad_stage);
        }
        else 
        {
            send_ack();
            check_progress();
        }
    }

	void rx_fsm::end_commit()
    {
        if (stage != rx_stage::pending_end_commit)
        {
            error(rx_error::bad_stage);
            return;
        }
        
        send_ack();
        stage = rx_stage::idle;
    }

}