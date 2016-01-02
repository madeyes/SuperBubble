#include <iostream>
#include <stdint.h>
#include "enet/enet.h"
#include "bubble_net.h"

const enet_uint16 PORT = 2468;
const size_t NUM_CLIENTS = 1;
const size_t NUM_CHANNELS = 1;
const enet_uint32 CHANNEL_ID = 0;

static ENetAddress address;
static ENetHost *server = nullptr;
static ENetHost *client = nullptr;
static ENetPeer *peer = nullptr;
static bool connected = false;

static void disconnect();
static bool sendHello();

const uint8_t TYPE_HELLO = 0;
const uint8_t TYPE_NUM_BUBBLES = 1;
const uint8_t TYPE_REMOTE_GAME_OVER = 2;

static struct BubbleInfo
{
    uint8_t type;
    uint8_t value;    
} info;

/*
Returns true for success.
*/
bool createServer()
{
    if (server == nullptr)
    {
        address.host = ENET_HOST_ANY;
        address.port = PORT;
        server = enet_host_create(&address, NUM_CLIENTS, NUM_CHANNELS, 0, 0);
        // Peer will get set when we get a connection.
        peer = nullptr;
    }
    return (server != nullptr);
}

/*
Returns true for success.
*/
bool createClient()
{
    if (client == nullptr)
    {
        client = enet_host_create(nullptr, NUM_CLIENTS, NUM_CHANNELS, 0, 0);
    }
    return (client != nullptr);
}

/*
Returns true for success.
*/
bool clientConnect(const char* hostName)
{
    if (client == nullptr)
    {
        return false;
    }

    if (strcmp(hostName, "localhost") == 0)
    {
        enet_address_set_host(&address, "127.0.0.1");
    }
    else
    {
        enet_address_set_host(&address, hostName);
    }
    address.port = PORT;
    
    peer = enet_host_connect(client, &address, NUM_CHANNELS, CHANNEL_ID);

    return (peer != nullptr);
}

/*
Returns true for success.
*/
bool sendBubbles(const uint8_t numBubbles)
{
    if (peer == nullptr)
    {
        return false;
    }    
 
    info.type = TYPE_NUM_BUBBLES;
    info.value = numBubbles;
    // ENet will handle packet deallocation.
    ENetPacket *packet = enet_packet_create(&info, sizeof(info), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, CHANNEL_ID, packet);
    return false;
}

/*
Returns true for success.
*/
bool sendGameOver()
{
    if (peer == nullptr)
    {
        return false;
    }

    info.type = TYPE_REMOTE_GAME_OVER;
    info.value = 0;
    // ENet will handle packet deallocation.
    ENetPacket *packet = enet_packet_create(&info, sizeof(info), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, CHANNEL_ID, packet);
    return false;
}

NetMessage updateNetwork()
{
    ENetHost *host = client == nullptr ? server : client;
    NetMessage result;
    result.type = NO_MESSAGE;
    result.numBubbles = 0;

    if (host != nullptr)
    {
        ENetEvent event;
        if (enet_host_service(host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Connected" << std::endl;
                connected = true;
                result.type = NetMessageType::CONNECTED;
                if (peer == nullptr)
                {
                    // Server.
                    peer = event.peer;                    
                }
                else
                {
                    // Client.
                    sendHello();                    
                }
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                info.type = *(event.packet->data);
                info.value = *(event.packet->data + 1);
                enet_packet_destroy(event.packet);
                switch (info.type)
                {
                case TYPE_HELLO:
                    break;
                case TYPE_NUM_BUBBLES:
                    result.type = NetMessageType::NUM_BUBBLES;
                    result.numBubbles = info.value;
                    break;
                case TYPE_REMOTE_GAME_OVER:
                    result.type = NetMessageType::REMOTE_GAME_OVER;
                    break;
                }
                break;
            case ENET_EVENT_TYPE_DISCONNECT:                
                peer = nullptr;
                connected = false;
                result.type = NetMessageType::DISCONNECT_REQ;
                break;
            }
        }
    }
    return result;
}

bool networkIsConnected()
{
    return connected;
}

bool isServer()
{
    return (server != nullptr);
}

void shutdownNetwork()
{   
    ENetHost *host = client == nullptr ? server : client;
    
    if (host != nullptr)
    {        
        disconnect();
        enet_host_destroy(host);
    }
    client = nullptr;
    server = nullptr;
    peer = nullptr;
    connected = false;
}

/*
Returns true for success.
*/
static bool sendHello()
{
    if (peer == nullptr)
    {
        return false;
    }
    info.type = TYPE_HELLO;
    info.value = 0;
    // ENet will handle packet deallocation.
    ENetPacket *packet = enet_packet_create(&info, sizeof(info), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, CHANNEL_ID, packet);
}

/*
timeoutMs : the time before forcing the connection down.
*/
static void disconnect()
{
    ENetHost *host = client == nullptr ? server : client;
    if (peer != nullptr && host != nullptr)
    {
        ENetEvent event;
        enet_peer_disconnect(peer, 0);
        // Allow up to 3 seconds for the disconnect to succeed and drop any packets received packets.
        while (enet_host_service(host, &event, 3000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                return;
            }
        }

        // Force connection down.
        enet_peer_reset(peer);
    }
}