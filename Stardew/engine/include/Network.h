#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ThreadSafeQueue.h"
#include "SharedPtr.h"
#include "IntTypes.h"
#include <stdlib.h>

/**
    @file Network.h
    @brief
     - game can be started with command line args to configure it as a server or a client (or singleplayer)
    - if server or client is chosen a thread is spawned to recieve date too and from the socket
    - it communicates with the game thread through 3 thread safe queues (named from the game threads perspective):
        - recieve data packets
        - transmit data packets
        - recieve connection events
    - A UDP based custom protocol using the netcode library to handle connecting and disconnecting: 
        - data can be sent reliably or unreliably
        - packets larget than NETCODE_MAX_PACKET_SIZE + (a small header) are transmitted as multiple packets and only 
        presented to the game thread when the full message is assembled
        - reliable packets should only be recieved by recipients game thread once
            - they'll be continually resent until reciept is acknowledged
        
    This layer of the networking code ensures the game thread can transmit and recieve reliable and unreliable packets of any size
    and is informed when a client connects or they as a client is connected to the server or disconnected.

    For the time being the game thread must create a shared pointer of the packet to be sent which the server thread will then free.
    For data received through the queue the game thread must call free() on it when done.

    Some better memory usage arrangement needs to be come up with at some point.
*/

enum MatchmakingProtocolMessageType
{
    /// @brief client sends to matchmaking server to seek a game server
    MM_ClientSeekServer,

    /// @brief game server sends to matchmaking server to begin seeking clients
    MM_ServerSeekClient,

    /// @brief send from the game server to the matchmaking server to indicate a player has joined
    MM_ServerPeerJoined,

    /// @brief send to client with address of server to join
    MM_GameConnection,
};


struct PlayerInfo
{
    char username[64];
};

struct ClientMatchmakingInfo
{
    struct PlayerInfo playerInfo;
};

// ip numbers = (4 * groups of 3 + 3 dots) + ':' + 5 numbers for port + null terminator
#define IP_AND_PORT_BUF_SIZE (15 + 3 + 1 + 5 + 1) 

struct ServerMatchmakingInfo
{
    struct PlayerInfo playerInfo;
    int availableSlots;
};

struct PeerAddress
{
    char username[64];
    char address[IP_AND_PORT_BUF_SIZE];
};

struct MatchMakingMessage
{
    enum MatchmakingProtocolMessageType type;
    union
    {
        struct ClientMatchmakingInfo clientInfo;
        struct ServerMatchmakingInfo serverInfo;
        struct PeerAddress peer;
    }data;
};


struct NetworkQueueItem
{
    /**
        @brief 
        if its a server recieve queue this is the sending client
        if its a server transmit queue then this is what client it should be sent to.

        On the client this is meaningless as communicaiton is always to and from the server
    */
    int client;

    /** @brief shared pointer when sent from game thread, malloced pointer when returned to game thread */
    SHARED_PTR(void) pData; 
    int pDataSize;
    bool bReliable;
    u64 sequenceNumber;
};

enum NetworkConnectionEventType
{
    NCE_ClientConnected,
    NCE_ClientDisconnected,
};

struct NetworkConnectionEvent
{
    enum NetworkConnectionEventType type;
    int client;
};

struct HostInfo
{
    const char* ip;
    unsigned short port;
};

enum GameRole
{
    GR_Singleplayer,
    GR_Client,
    GR_ClientServer
};

void NW_Init();

bool NW_DequeueData(struct NetworkQueueItem* pOut);

bool NW_DequeueConnectionEvent(struct NetworkConnectionEvent* pOut);

void NW_EnqueueData(struct NetworkQueueItem* pIn);

enum GameRole NW_GetRole();

#ifdef __cplusplus
}
#endif

#endif
