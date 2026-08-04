#include <iostream>
#include <cassert>
#include <cstring>
#include <string_view>

#include "protocol/base_fsm/base_fsm.h"
#include "protocol/cl/cl.h"
#include "protocol/sv/sv.h"

using namespace tungsten::protocol;

uint64_t client_nonce = 0x1111222233334444ULL;
uint64_t server_nonce = 0xAAAA6666BBBB7777ULL;

struct test_loopback_context
{
    client_fsm* client;
    server_client_fsm* server;
};

int main()
{
    std::cout << "[TEST] Starting Tungsten FSM Handshake Test..." << std::endl;

    client_fsm client{};
    server_client_fsm server_slot{};

    test_loopback_context loopback_ctx{ &client, &server_slot };

    auto server_send_to_client = [](void* ctx, bool reliable, int size, const void* data)
        {
            auto* loop = static_cast<test_loopback_context*>(ctx);
            std::cout << "  [NET] SV -> CL | Size: " << size << " | Reliable: " << reliable << std::endl;
            loop->client->on_recv(size, data);
        };

    auto client_send_to_server = [](void* ctx, bool reliable, int size, const void* data)
        {
            auto* loop = static_cast<test_loopback_context*>(ctx);
            auto* cl = loop->client;
            auto* sv = loop->server;

            std::cout << "  [NET] CL -> SV | Size: " << size << " | Reliable: " << reliable << std::endl;

            bool bad_version = false;
            uint64_t cl_nonce = NO_NONCE;

            if (server_client_fsm::is_conn_req(bad_version, cl_nonce, size, data))
            {
                // --- (branch A) ---
                
                
                std::cout << "  [SERVER PLATFORM] Free slot found! Opening server slot..." << std::endl;
                sv->open(cl_nonce, server_nonce, false);
                

                // --- (branch B) ---
                //
                //std::cout << "  [SERVER PLATFORM] Slots full! Generating reject..." << std::endl;
                //reject_reason reason{};
                //const char* text = "there are no place";
                //reason.size = static_cast<uint32_t>(std::strlen(text));
                //std::memcpy(reason.data, text, reason.size);

                //packet_t packet;
                //int total_size = 0;
                //server_client_fsm::reject(sv->get_timestamp_us(), cl_nonce, reason, sizeof(packet), total_size, &packet);
                //std::cout << "  [NET] SV -> CL (REJECT) | Size: " << total_size << std::endl;
                //cl->on_recv(total_size, &packet);
                //
            }
            else
            {
                sv->on_recv(size, data);
            }
        };

    base_fsm_callbacks client_cmn_cb;
    client_cmn_cb.on_send = client_send_to_server;
    client_cmn_cb.on_error = [](void*, const fsm_error& err)
        {
            std::cout << "  [ERR] Client FSM error type: " << static_cast<int>(err.type) << std::endl;
        };
    client_cmn_cb.on_disconnected = [](void*, disconnect_type type, const disconnect_reason& reason)
        {
            std::string_view s{ reason.data, static_cast<size_t>(reason.size) };
            std::cout << "  [CL] Server disconnected. Type: " << static_cast<int>(type) << " | Reason: " << s << std::endl;
        };

    base_fsm_callbacks server_cmn_cb;
    server_cmn_cb.on_send = server_send_to_client;
    server_cmn_cb.on_error = [](void*, const fsm_error& err)
        {
            std::cout << "  [ERR] Server FSM error type: " << static_cast<int>(err.type) << std::endl;
        };
    server_cmn_cb.on_disconnected = [](void*, disconnect_type type, const disconnect_reason& reason)
        {
            std::string_view s{ reason.data, static_cast<size_t>(reason.size) };
            std::cout << "  [SV] Client disconnected. Type: " << static_cast<int>(type) << " | Reason: " << s << std::endl;
        };

    client.set_context(&loopback_ctx);
    client.set_base_callbacks(client_cmn_cb);

    server_slot.set_context(&loopback_ctx);
    server_slot.set_base_callbacks(server_cmn_cb);

    client_fsm_callbacks cl_callbacks;

    cl_callbacks.on_conn_accepted = [](void* ctx, bool need_file_sync)
        {
            std::cout << "  [CL] Connection accepted, need_file_sync: " << need_file_sync << std::endl;
        };

    cl_callbacks.on_conn_rejected = [](void* ctx, const reject_reason& reason)
        {
            std::string_view s{ reason.data, static_cast<size_t>(reason.size) };
            std::cout << "  [CL] Connection rejected, reason: " << s << std::endl;
        };

    cl_callbacks.on_snapshot = [](void* ctx, int size, const void* data) -> bool
        {
            std::cout << "[CL] Snapshot received, size: " << size << std::endl;
            return true;
        };

    client.set_client_callbacks(cl_callbacks);

    server_fsm_callbacks sv_callbacks;

    sv_callbacks.on_recv_usercmd = [](void* ctx, int size, const void* data) -> bool
        {
            std::cout << "[SV] UserCmd received, size: " << size << std::endl;
            return true;
        };

    server_slot.set_server_callbacks(sv_callbacks);

    std::cout << "[TEST] Step 1: Client calls open()..., Server checks this and calls open()" << std::endl;
    client.open(client_nonce);

    std::cout << "[TEST] Step 2: Clients sends usercmd" << std::endl;
    client.usercmd(0, nullptr);

    std::cout << "[TEST] Step 3: Server initiates disconnect()..." << std::endl;
    server_slot.snapshot(0, nullptr);

    std::cout << "[TEST] Step 4: Server initiates disconnect()..." << std::endl;
    disconnect_reason reason{};
    const char* kick_text = "kicked for being dumbass";
    reason.size = static_cast<uint32_t>(std::strlen(kick_text));
    std::memcpy(reason.data, kick_text, reason.size);

    // --- (branch A) ---
    server_slot.disconnect(disconnect_type::kicked, reason);

    std::cout << "[TEST] Test finished successfully!" << std::endl;
    return 0;
}
