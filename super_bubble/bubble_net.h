#ifndef NET_H
#define NET_H

enum NetMessageType
{
    NO_MESSAGE,
    CONNECTED,
    DISCONNECTED,
    NUM_BUBBLES
};

struct NetMessage
{
    NetMessageType type;
    // Only valid if type is NUM_BUBBLES, otherwise should be zero.
    uint8_t numBubbles;
};

bool createServer();
bool createClient();
bool clientConnect(const char* hostName);
NetMessage updateNetwork();
bool sendBubbles(const uint8_t numBubbles);
bool networkIsConnected();
bool isServer();
void shutdownNetwork();

#endif
