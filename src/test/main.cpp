#include <cstring>
#include <iostream>

#include "../common/formats/protocol/protocol.h"

using namespace tungsten;

protocol::client_fsm cl_fsm;
protocol::server_fsm sv_fsm;

void process_cl_events()
{
    protocol::event e;
    while (cl_fsm.poll_event(e))
    {
        
        std::cout << "[client] event: ";
        switch (e.type) 
        {
        case protocol::event_type::send:
            std::cout << "send";
            sv_fsm.on_recv_packet(e.send.p);
            break;
        case protocol::event_type::connection_accepted:
            std::cout << "connection_accepted\n";
            std::cout << "need_filesync: " << e.conn_accepted.need_filesync;
            break;
        case protocol::event_type::connection_rejected:
            std::cout << "connection_rejected:\nreason:";
            std::cout << std::string(e.conn_rejected.reason.data, e.conn_rejected.reason.size);
            break;
        case protocol::event_type::error:
            std::cout << "error";
            break;
        default:
            break;
        }
        std::cout << std::endl;
    }
}

void process_sv_events()
{
    protocol::event e;
    while (sv_fsm.poll_event(e))
    {
        
        std::cout << "[server] event: ";
        switch (e.type) 
        {
        case protocol::event_type::send:
            std::cout << "send";
            cl_fsm.on_recv_packet(e.send.p);
            break;
        case protocol::event_type::connection_canceled:
            std::cout << "connection_canceled";
            break;
        case protocol::event_type::connection_requested:
        {
            std::cout << "connection_requested";
            char s[] = "test accept";
            sv_fsm.accept(-1, false);
        }
            break;
        case protocol::event_type::error:
            std::cout << "error";
            break;
        default:
            break;
        }
        std::cout << std::endl;
    }
}



int main()
{
    cl_fsm.reset();
    sv_fsm.reset();

    cl_fsm.connect_to(0);

    uint64_t tick = 0;

    int iters = 10;

    for (;iters--;tick++)
    {

        if (tick == 0)
        {
            cl_fsm.cancel();
        }

        cl_fsm.tick(tick);
        process_cl_events();
        
        sv_fsm.tick(tick);
        process_sv_events();

        std::cout << "[tick: " << tick << ']' << std::endl; 
    }

    return 0;
}
