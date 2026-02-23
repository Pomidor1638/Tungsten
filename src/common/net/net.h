#ifndef _MEGA_GAME_NET_H_ 
#define _MEGA_GAME_NET_H_ 

#include <string>
#include <vector>
#include <cstdint>

struct NetAddress {
    uint8_t ip[4];
    uint16_t port;

    NetAddress() : ip{ 0, 0, 0, 0 }, port(0) {}
    NetAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t p)
        : ip{ a, b, c, d }, port(p) {
    }

    std::string toString() const {
        return std::to_string((int)ip[0]) + "." +
            std::to_string((int)ip[1]) + "." +
            std::to_string((int)ip[2]) + "." +
            std::to_string((int)ip[3]) + ":" +
            std::to_string(port);
    }

    bool operator==(const NetAddress& other) const {
        return ip[0] == other.ip[0] && ip[1] == other.ip[1] &&
            ip[2] == other.ip[2] && ip[3] == other.ip[3] &&
            port == other.port;
    }
};

struct ConnectionStats 
{
    uint64_t bytesSent;      
    uint64_t bytesReceived;  
    uint32_t lastSendTime;   
    uint32_t lastReceiveTime;
};

struct ClientInfo 
{
    NetAddress address;  
    uint32_t connectTime;
    ConnectionStats stats;
};

class ServerBase
{
public:

    ServerBase(int port);
    virtual ~ServerBase();

    bool init();
    void quit();

    void update();

    void broadcast(const void* data, size_t length, bool reliable = true);
    bool sendToClient(const NetAddress& address, const void* data, size_t length, bool reliable = true);

    void disconnectClient(const NetAddress& address);
    void disconnectAll();

    size_t getClientCount() const;
    std::vector<ClientInfo> getClientsInfo() const;
    bool getClientInfo(const NetAddress& address, ClientInfo& outInfo) const;

protected:

    virtual bool canClientConnect(const NetAddress& address) = 0;
    virtual void onClientConnected(const NetAddress& address) = 0;
    virtual void onClientDisconnected(const NetAddress& address) = 0;
    virtual void onDataReceived(const NetAddress& address, const void* data, size_t length) = 0;

private:
    int port;
    void* host;
    void* clientInfoMap;
};

class ClientBase
{
public:
    ClientBase();
    virtual ~ClientBase();

    bool init();
    void quit();

    bool connect(const std::string& hostname, int port);
    void disconnect();
    void update();

    bool send(const void* data, size_t length, bool reliable = true);

    bool isConnected() const { return connected; }
    bool getConnectionStats(ConnectionStats& outStats) const;
    bool getServerAddress(NetAddress& outAddress) const;

protected:
    virtual void onConnected(const NetAddress& serverAddress) = 0;
    virtual void onDisconnected() = 0;
    virtual void onDataReceived(const void* data, size_t length) = 0;

private:
    bool connected;
    void* host;
    void* peer;
    NetAddress serverAddress;
    ConnectionStats stats;
};

#endif