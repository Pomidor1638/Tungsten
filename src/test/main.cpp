

#include <ctime>

#include "../common/formats/protocol/cl/cl.h"
#include "../common/formats/protocol/sv/sv.h"


using namespace tungsten;

protocol::client_fsm cl_fsm{};
protocol::server_client_fsm sv_fsm{};

uint64_t last_time;
uint64_t delta_time;
bool running = false;

struct TestConfig
{
    size_t iterations;
} config;

void cl_tick()
{
    cl_fsm.tick(delta_time);
    protocol::event e;
    while (cl_fsm.poll_event(e))
    {
        switch (e.type) 
        {
        case protocol::event_type::send:
            break;
		case protocol::event_type::error:
            break;
        case protocol::event_type::disconnected:
            break;
        case protocol::event_type::connection_accepted:
            break;
        case protocol::event_type::connection_rejected:
            break;
        default:
            break;
        }
    }
}

void sv_tick()
{
    sv_fsm.tick(delta_time);
    protocol::event e;
    while (sv_fsm.poll_event(e))
    {
        switch (e.type) 
        {
        case protocol::event_type::send:
            break;
		case protocol::event_type::error:
            break;
        case protocol::event_type::disconnected:
            break;
        case protocol::event_type::connection_canceled:
            break;
        default:
            break;
        }
    }
}

void test()
{
    uint64_t start = time(nullptr);
    uint64_t last_time = start;
    
    for (size_t i = 0; running && i != config.iterations; i++)
    {
        uint64_t cur_time = time(nullptr);
        delta_time = cur_time - last_time;

        cl_tick();
        sv_tick();
    }

    uint64_t end = time(nullptr);
    printf("Time elapsed: %llu\n", end - start);
}

int main()
{
    config.iterations = 10000;
    test();

    return 0;
}