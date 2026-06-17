#include <cstdio>
#include <cstring>

#include "../common/formats/transfer/transfer.h"

namespace tr = tungsten::transfer;


tr::tx_fsm tx{};
tr::rx_fsm rx{};

constexpr size_t TEST_SIZE = 32ull * 1024 * 1024 * 1024;

char* tx_data = nullptr;
char* rx_data = nullptr;

bool running = true;

void process_tx_action()
{
    tr::tx_action action = tx.pull_action();
    tr::packet p;
    

    using enum tr::tx_action_type;
    switch (action.type) 
    {
    case send:
        //printf("[send]: %i bytes", action.send.size);
        memcpy(&p, action.send.data, action.send.size);    
        rx.on_recv(p);
        break;
    case need_chunk:
        //printf("[need_chunk]: offset: %llu, size: %i", action.need_chunk.offset, action.need_chunk.size);
        tx.chunk(action.need_chunk.offset, action.need_chunk.size, &tx_data[action.need_chunk.offset]);
        break;
    case error:
        printf("[tx]: ");
        printf("[error]: code: %i", (int)action.error.code);
        tx.reset();
        rx.reset();
        running = false;
        printf("\n");
        break;
    case complete:
        printf("[tx]: ");
        printf("[complete]");
        printf("\n");
        running = false;
        break;
    default:
        break;
    }
}

void process_rx_action()
{
    tr::rx_action action = rx.pull_action();
    tr::packet p;
    
    using enum tr::rx_action_type;
    switch (action.type) 
    {
    case send:
        //printf("[send]: %i bytes", action.send.size);
        //printf("[rx]: ");
        memcpy(&p, action.send.data, action.send.size);    
        tx.on_recv(p);
        //printf("\n");
        break;
	case error:
        printf("[rx]: ");
        printf("[error]: code: %i", (int)action.error.code);
        tx.reset();
        rx.reset();
        running = false;
        printf("\n");
        break;
	case offer:
    {
        printf("[rx]: ");
        auto& offer = action.offer;
        printf("[offer]: size: %llu, chunk_size: %i", offer.total_size, offer.chunk_size);
        rx.offer_accept();
        printf("\n");
    }
        break;
	case chunk:
    {
        //printf("[rx]: ");
        auto& chunk = action.chunk;
        //printf("[chunk]: offset: %llu, size: %i", chunk.offset, chunk.size);
        memcpy(&rx_data[chunk.offset], chunk.data, chunk.size);
        rx.chunk_commit();
        //printf("\n");
    }
        break;
	case end:
        printf("[rx]: ");
        printf("[end]");
        rx.end_commit();
        printf("\n");
        //running = false;
        break;
    default:
        break;
    }

}

int main()
{

    tx_data = new char[TEST_SIZE];
    rx_data = new char[TEST_SIZE];

    for (size_t i = 0; i < TEST_SIZE; ++i)
        tx_data[i] = static_cast<char>((i * 13 + 7) & 0xFF);

    tx_data[TEST_SIZE-1] = '\0';

    memset(rx_data, 0, TEST_SIZE);

    printf("start testing...\n");
    tx.reset();
    rx.reset();

    tx.begin(TEST_SIZE, tr::MAX_CHUNK_SIZE);
    rx.start();

    uint64_t ticks = 0;

    while (running)
    {
        process_tx_action();
        process_rx_action();
        ticks++;
    }

    printf("transmitted : %s\n", tx_data);
    printf("received    : %s\n", rx_data);
    printf("ticks       : %llu\n", ticks);
    printf("total size  : %llu bytes\n", TEST_SIZE);
    printf("memcmp: %s\n", memcmp(tx_data, rx_data, TEST_SIZE) ? "not ok" : "ok");

    delete[] tx_data;
    delete[] rx_data;

    return 0;
}
