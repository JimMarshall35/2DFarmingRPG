#include "Network.h"
#include "netcode.h"
#include "main.h"
#include "Thread.h"
#include "Log.h"
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "main.h"
#include "RawNetMessage.h"
#include "ANSIColourCodes.h"
#include "ThreadSafeQueue.h"
#include "ObjectPool.h"
#include "DynArray.h"
#include "SharedPtr.h"
#include "AssertLib.h"
#include "FileHelpers.h"
#include "cJSON.h"

struct NetworkThreadQueues
{
    /* queue of struct NetworkQueueItem transmit to network */
    struct ThreadSafeQueue tx;
    /* queue of struct NetworkQueueItem recieve from network */
    struct ThreadSafeQueue rx;

    struct ThreadSafeQueue connectionEvents;
};


// resend if unacked after 100ms
#define RESEND_IF_UNACKED_THRESHOLD (100.0 / 1000.0) 


/*
    TODO, URGENT BUT TEDIOUS:
    The FragmentedMessageReciever and ReliableMessageTracker need to use indexes into the pool instead of pointers for
    next and previous, if it resizes, it'll be all fucked up, yo
*/

/*
    A fragmented message that's being reassembled,
    when reassembly is complete it will be pushed to the rx queue
*/
struct FragmentedMessageReciever
{
    u8* data;
    u32 ident;
    u32 totalFragments;
    u32 fragmentsRecieved;
    struct FragmentedMessageReciever* pNext;
    struct FragmentedMessageReciever* pPrev;
};

static OBJECT_POOL(struct FragmentedMessageReciever) gFragmentedMessageRecieverPool = NULL;

static struct FragmentedMessageReciever* pFragmentedMessageRecieverListHead = NULL;

static struct FragmentedMessageReciever* pFragmentedMessageRecieverListTail = NULL;

/* 
    Tracks a reliable message from the POV of the sender.
    The data must be kept around to be resent until it is acked.

    Multiple ReliableMessageTracker objects can refer to the same shared pointer of data
    and represent offsets into it, this is to allow big messages to be sent in fragments
    which are only presented to the message queue when the entire message has been reconstructed.

    When all have been acked the reference count is zero and the data is freed.
*/
struct ReliableMessageTracker
{
    u32 ident;
    SHARED_PTR(void) data;
    u32 dataSize;
    u32 dataOffset;

    i32 client;
        
    struct ReliableMessageTracker* pNext;
    struct ReliableMessageTracker* pPrev;

    struct NetFragmentMessageHeader fragmentHeader;

    double lastSentTime;
};


typedef HGeneric HReliableTracker;

static OBJECT_POOL(struct ReliableMessageTracker) gReliableTrackerPool = NULL;

struct ReliableMessageTracker* pReliableTrackerListHead = NULL;

struct ReliableMessageTracker* pReliableTrackerListTail = NULL;

static enum GameRole gRole;

static struct NetworkThreadQueues* pQueues;

#define TEST_PROTOCOL_ID 0x1122334455667788
#define GAME_PROTOCOL_ID TEST_PROTOCOL_ID
#define CONNECT_TOKEN_EXPIRY 30
#define CONNECT_TOKEN_TIMEOUT 5
#define GAME_MAX_CLIENTS 3


enum GameClientState
{
    GCS_Disconnected,
    GCS_Connected
};

struct GameClient
{
    enum GameClientState state;
};

#define RECENTLY_ACKED_PACKETS_BUF_SIZE 64
static u32 gRecentlyAckedIdentBufferSize = 0;
static u32 gRecentlyAckedPacketIdentifiers[RECENTLY_ACKED_PACKETS_BUF_SIZE];

static void PushAckedPacketIdentifier(u32 ident)
{
    static int nextI = -1;
    gRecentlyAckedPacketIdentifiers[nextI++] = ident;
    if(nextI >= RECENTLY_ACKED_PACKETS_BUF_SIZE)
    {
        nextI = 0;
    }
    gRecentlyAckedIdentBufferSize++;
}

int clamp(int i, int c)
{
    return i <= c ? i : c;
}

static bool HasReliablePacketBeenRecentlyAcknowledged(u32 ident)
{
    for(int i=0; i<clamp(gRecentlyAckedIdentBufferSize, RECENTLY_ACKED_PACKETS_BUF_SIZE); i++)
    {
        if(ident == gRecentlyAckedPacketIdentifiers[i])
        {
            return true;
        }
    }
    return false;
}


static u8 gPacketBuffer[NETCODE_MAX_PACKET_SIZE];

CrossPlatformThread gNetworkThread;

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea, 
                                                  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4, 
                                                  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Client and Server


static struct netcode_network_simulator_t* GetNetworkSimulator()
{
    if(!gCmdArgs.networkSimulatorConfigPath)
    {
        return NULL;
    }
    int sz = 0;
    char* data = LoadFile(gCmdArgs.networkSimulatorConfigPath, &sz);
    Log_Info("Creating network simulator...");
    cJSON* pJSON = cJSON_ParseWithLength(data, sz);
    cJSON* pLatency = cJSON_GetObjectItem(pJSON, "latency_milliseconds");
    cJSON* pJitter = cJSON_GetObjectItem(pJSON, "jitter_milliseconds");
    cJSON* pPacketLossPercent = cJSON_GetObjectItem(pJSON, "packet_loss_percent");
    cJSON* pDuplicatePacketPercent = cJSON_GetObjectItem(pJSON, "duplicate_packet_percent");

    struct netcode_network_simulator_t* pSim = netcode_network_simulator_create(NULL, NULL, NULL);
    pSim->latency_milliseconds = (float)pLatency->valuedouble;
    pSim->jitter_milliseconds = (float)pJitter->valuedouble;
    pSim->packet_loss_percent = (float)pPacketLossPercent->valuedouble;
    pSim->duplicate_packet_percent = (float)pDuplicatePacketPercent->valuedouble;
    Log_Info("Network simulator: latency: %.2fms jitter: %.2f packet loss: %.2f%% packet dup: %.2f%%",
        pSim->latency_milliseconds,
        pSim->jitter_milliseconds,
        pSim->packet_loss_percent,
        pSim->duplicate_packet_percent
    );
    return pSim;
}

static int NetcodeLog(const char* fmt, ...)
{
    static char* netcodeLogTagString = "[Netcode] ";
    static char* netcodeColouredLogTagString = BHBLU"[Netcode]"CRESET" ";
    char buf[512];
    va_list args;
    va_start(args, fmt);
    
    sprintf(buf, gCmdArgs.bLogTextColoured ? netcodeColouredLogTagString : netcodeLogTagString);
    int len = strlen(buf);
    vsnprintf(buf + len, 512 - len, fmt, args);
    len = strlen(buf);
    while(buf[len - 1] == '\n' || buf[len - 1] == '\r')
    {
        buf[--len] = '\0';
    }
    
    Log_Info(buf);
    va_end(args);
}


static void AcknowledgeIdentifier(u32 ident)
{
    struct ReliableMessageTracker* pTracker = pReliableTrackerListHead;
    while(pTracker)
    {
        if(pTracker->ident == ident)
        {
            if(pReliableTrackerListHead == pTracker)
            {
                pReliableTrackerListHead = pTracker->pNext;
            }
            if(pReliableTrackerListTail == pTracker)
            {
                pReliableTrackerListTail = pTracker->pPrev;
            }
            if(pTracker->pNext)
            {
                pTracker->pNext->pPrev = pTracker->pPrev;
            }
            if(pTracker->pPrev)
            {
                pTracker->pPrev->pNext = pTracker->pNext;
            }
            i64 rc = Sptr_GetRefCount(pTracker->data);
            Log_Verbose(BHGRN"[NETWORK]"CRESET" found tracker for msg ID %i, about to decrement refcount, current count %lld", ident, rc);
            Sptr_RemoveRef(pTracker->data);
            
            return;
        }
        pTracker = pTracker->pNext;
    }
    Log_Info("Reliable message ack packet recieved, doesn't correspond to any in list. ID: %i", ident);
}

static struct FragmentedMessageReciever* FindFragmentedMessageReciever(u32 ident)
{
    if(pFragmentedMessageRecieverListHead == NULL || pFragmentedMessageRecieverListTail == NULL)
    {
        /* list empty */
        return NULL;
    }
    struct FragmentedMessageReciever* pReciever = pFragmentedMessageRecieverListHead;
    while(pReciever)
    {
        if(pReciever->ident == ident)
        {
            return pReciever;
        }
        pReciever = pReciever->pNext;
    }
}

static struct FragmentedMessageReciever* CreateNewReciever(struct NetFragmentMessageHeader* pFragmentHeader)
{
    int i = 0;
    gFragmentedMessageRecieverPool = GetObjectPoolIndex(gFragmentedMessageRecieverPool, &i);
    struct FragmentedMessageReciever* pRec = &gFragmentedMessageRecieverPool[i];

    pRec->pNext = NULL;
    pRec->pPrev = NULL;
    pRec->fragmentsRecieved = 0;
    pRec->ident = pFragmentHeader->fragmentedMsgID;
    pRec->totalFragments = pFragmentHeader->numFragments;
    pRec->data = malloc(pFragmentHeader->fragmentedMsgTotalSize);
    if(pFragmentedMessageRecieverListHead == NULL || pFragmentedMessageRecieverListTail == NULL)
    {
        pFragmentedMessageRecieverListHead = pRec;
        pFragmentedMessageRecieverListTail = pRec;
        return pRec;
    }
    pFragmentedMessageRecieverListTail->pNext = pRec;
    pRec->pPrev = pFragmentedMessageRecieverListTail;
    pFragmentedMessageRecieverListTail = pRec;
    return pRec;
}


/*
    returns the completed message or nullptr
*/
static void* RecieveFragmentedMessage(u8* packet, int packetSize, int* outCompletePacketBytes)
{
    *outCompletePacketBytes = 0;
    enum NetRawMessageType type;
    u8* body = NULL;
    NetMsg_Parse(packet, &type, &body);
    EASSERT(type == ReliableDataMessageFragment);

    struct NetFragmentMessageHeader* pFragment = NetMsg_GetFragmentHeader(packet);
    *outCompletePacketBytes = pFragment->fragmentedMsgTotalSize;
    struct FragmentedMessageReciever* pReciever = FindFragmentedMessageReciever(pFragment->fragmentedMsgID);
    Log_Verbose(BHGRN"[NETWORK]"CRESET" Recieving message fragment. package fragment no: %i/%i, msg ID: %i", pFragment->sequenceNum, pFragment->numFragments, pFragment->fragmentedMsgID);
    if(!pReciever)
    {
        pReciever = CreateNewReciever(pFragment);
    }
    int fullPacketSize = NETCODE_MAX_PACKET_SIZE - NetMsg_SizeOfHeaders(ReliableDataMessageFragment);
    u8* pWrite = pReciever->data + pFragment->sequenceNum * fullPacketSize;
    memcpy(pWrite, body, packetSize - NetMsg_SizeOfHeaders(ReliableDataMessageFragment));
    pReciever->fragmentsRecieved++;
    u8* rVal = NULL;
    if(pReciever->fragmentsRecieved == pReciever->totalFragments)
    {
        Log_Verbose(BHGRN"[NETWORK]"CRESET" complete message recieved");
        if(pReciever->pNext)
        {
            pReciever->pNext->pPrev = pReciever->pPrev;
        }
        if(pReciever->pPrev)
        {
            pReciever->pPrev->pNext = pReciever->pNext;
        }
        if(pFragmentedMessageRecieverListHead == pReciever)
        {
            pFragmentedMessageRecieverListHead = pReciever->pNext;
        }
        if(pFragmentedMessageRecieverListTail == pReciever)
        {
            pFragmentedMessageRecieverListTail = pReciever->pPrev;
        }
        rVal = pReciever->data; /*move from data*/
    }
    return rVal;
}


static u32 TrackReliableMessage(u8* data, u32 dataSize, u32 dataOffset, struct NetFragmentMessageHeader* pHeader, double time, int client)
{
    HReliableTracker hT;
    gReliableTrackerPool = GetObjectPoolIndex(gReliableTrackerPool, &hT);
    gReliableTrackerPool[hT].ident = NetMsg_GetReliableMessageIdentifier();
    gReliableTrackerPool[hT].data = data;
    gReliableTrackerPool[hT].dataSize = dataSize;
    gReliableTrackerPool[hT].pNext = NULL;
    gReliableTrackerPool[hT].pPrev = NULL;
    gReliableTrackerPool[hT].lastSentTime = time;
    gReliableTrackerPool[hT].client = client;
    if(pReliableTrackerListHead == NULL)
    {
        pReliableTrackerListHead = &gReliableTrackerPool[hT];
        pReliableTrackerListTail = pReliableTrackerListHead;
        return gReliableTrackerPool[hT].ident;
    }
    pReliableTrackerListTail->pNext = &gReliableTrackerPool[hT];
    gReliableTrackerPool[hT].pPrev = pReliableTrackerListTail;
    pReliableTrackerListTail = &gReliableTrackerPool[hT];
    if(pHeader)
    {
        gReliableTrackerPool[hT].fragmentHeader = *pHeader;
    }
    else
    {
        memset(&gReliableTrackerPool[hT].fragmentHeader, 0, sizeof(struct NetFragmentMessageHeader));
    }
    return gReliableTrackerPool[hT].ident;
}

static void ServiceConnectionEventsBase(struct GameClient* client, int clientIndex, struct NetworkThreadQueues* pQueues, bool(*IsConnected)(void*,int), void* clientOrServer)
{
    if(!IsConnected(clientOrServer, clientIndex))
    {
        if(client->state != GCS_Disconnected)
        {
            struct NetworkConnectionEvent event = 
            {
                .client = clientIndex,
                .type = NCE_ClientDisconnected
            };
            TSQ_Enqueue(&pQueues->connectionEvents, &event);
        }
        client->state = GCS_Disconnected;
    }
    else
    {
        switch(client->state)
        {
        case GCS_Disconnected:
            {
                struct NetworkConnectionEvent event = 
                {
                    .client = clientIndex,
                    .type = NCE_ClientConnected
                };
                Log_Info("Enqueuing connection event...\n");
                TSQ_Enqueue(&pQueues->connectionEvents, &event);
                client->state = GCS_Connected;
            }
            break;
        case GCS_Connected:
            break;
        }
    }
    
}

/*
    These functions wrap netcode_client/server_send/recieve .
    Those that take clientIndex are only relevant if you are the server.
*/
typedef u8*(*RecievePacketFn)(void* serverOrClient, int clientIndex, int* bytesRecieved, uint64_t* packetSequence);
typedef void(*SendPacketFn)(void* serverOrClient, int clientIndex, u8* packetData, int packetSize);
typedef void(*FreePacketFn)(void* serverOrClient, void* packet);

static void RecievePacketsBase(void* serverOrClient, struct NetworkThreadQueues* pQueues, RecievePacketFn recievePacket, SendPacketFn sendPacket, FreePacketFn freePacket)
{
    
    int client_index;
    for ( client_index = 0; client_index < NETCODE_MAX_CLIENTS; ++client_index )
    {
        while ( 1 )             
        {
            int packet_bytes;
            uint64_t packet_sequence;
            void * packet = recievePacket( serverOrClient, client_index, &packet_bytes, &packet_sequence );
            if ( !packet )
                break;

            /* respond to packet here */
            enum NetRawMessageType msgType;
            u8* pBody = NULL;
            NetMsg_Parse(packet, &msgType, &pBody);
            int headersSize = NetMsg_SizeOfHeaders(msgType);
            int payloadSize = packet_bytes - headersSize;
            switch (msgType)
            {
            case UnreliableDataMessageComplete:
                {
                    
                    struct NetworkQueueItem qItem;
                    qItem.sequenceNumber = packet_sequence;
                    qItem.client = client_index;
                    qItem.pData = malloc(payloadSize);
                    qItem.pDataSize = payloadSize;
                    memcpy(qItem.pData, pBody, payloadSize);
                    qItem.bReliable = false;
                    TSQ_Enqueue(&pQueues->rx, &qItem);
                }
                break;
            case ReliableDataMessageComplete:
                {
                    /* acknowledge */
                    /*
                        idea:
                        keep a circular buffer of recently acked packets, search it here before proceeding.
                        A crude filter.
                    */
                    struct NetReliableMessageHeader* pHeader = NetMsg_GetReliableHeader(packet);
                    if(!HasReliablePacketBeenRecentlyAcknowledged(pHeader->messageIdentifier))
                    {
                        int packetSize = NetMsg_WriteReliableDataAckPacket(gPacketBuffer, pHeader->messageIdentifier);
                        Log_Verbose(BHGRN"[NETWORK]"CRESET" Sending ack packet for complete message, msg ID: %i", pHeader->messageIdentifier);
                        sendPacket(serverOrClient, client_index, gPacketBuffer, packetSize);
                        PushAckedPacketIdentifier(pHeader->messageIdentifier);

                        /* only push the data for a reliable packet once to the game thread */
                        struct NetworkQueueItem qItem;
                        qItem.client = client_index;
                        qItem.pData = malloc(payloadSize);
                        qItem.pDataSize = payloadSize;
                        memcpy(qItem.pData, pBody, payloadSize);
                        qItem.bReliable = false;
                        TSQ_Enqueue(&pQueues->rx, &qItem);   
                    }
                    else
                    {
                        Log_Verbose(BHGRN"[NETWORK]"CRESET" resending ack packet for retransmitted message fragment, previous ack must have been lost");
                        /* this same reliable message has been acknowledged before recently, but we're getting it again so ack but don't push to game thread again */
                        int packetSize = NetMsg_WriteReliableDataAckPacket(gPacketBuffer, pHeader->messageIdentifier);
                        sendPacket(serverOrClient, client_index, gPacketBuffer, packetSize);
                    }
                    
                }
                break;
            case ReliableDataMessageFragment:
                {
                    struct NetReliableMessageHeader* pHeader = NetMsg_GetReliableHeader(packet);
                    if(!HasReliablePacketBeenRecentlyAcknowledged(pHeader->messageIdentifier))
                    {
                        int completePacketSize = 0;
                        void* pComplete = RecieveFragmentedMessage(packet, packet_bytes, &completePacketSize);

                        /* send ack */
                        int packetSize = NetMsg_WriteReliableDataAckPacket(gPacketBuffer, pHeader->messageIdentifier);
                        Log_Verbose(BHGRN"[NETWORK]"CRESET" Sending ack packet for fragment message, msg ID: %i", pHeader->messageIdentifier);
                        sendPacket(serverOrClient, client_index, gPacketBuffer, packetSize);
                        PushAckedPacketIdentifier(pHeader->messageIdentifier);

                        if(pComplete)
                        {
                            struct NetworkQueueItem qItem;
                            qItem.client = client_index;
                            qItem.pData = pComplete;
                            qItem.pDataSize = completePacketSize;
                            qItem.bReliable = true;
                            TSQ_Enqueue(&pQueues->rx, &qItem);
                        }
                        
                    }
                    else
                    {
                        /* this same reliable message has been acknowledged before recently, but we're getting it again so ack but don't push to game thread again */
                        Log_Verbose(BHGRN"[NETWORK]"CRESET"Sending ack packet for retransmitted message fragment, previous ack must have been lost");
                        int packetSize = NetMsg_WriteReliableDataAckPacket(gPacketBuffer, pHeader->messageIdentifier);
                        sendPacket(serverOrClient, client_index, gPacketBuffer, packetSize);
                    }
                }
                break;
            case ReliableDataMessageAck:
                {
                    u32 identifier = NetMsg_GetAckedIdentifier(packet);
                    Log_Verbose(BHGRN"[NETWORK]"CRESET" Recieved Ack packet for ID %i", identifier);
                    AcknowledgeIdentifier(identifier);
                }
                break;
            }

            freePacket(serverOrClient, packet);
        }
    }
}


static void SendMessageFragmentsBase(struct NetworkThreadQueues* pQueues, void* serverOrClient, struct NetworkQueueItem* item, double time, SendPacketFn sendPacket)
{
    EASSERT(item->pDataSize > NETCODE_MAX_PACKET_SIZE);
    int sizeOfHeaders = NetMsg_SizeOfHeaders(ReliableDataMessageFragment);
    int maxPayloadPerPacket = NETCODE_MAX_PACKET_SIZE - sizeOfHeaders;
    int off = 0;
    u16 seqNum = 0;
    u32 fragmentedMsgID = NetMsg_GetReliableMessageIdentifier();
    u16 numTotal = item->pDataSize / maxPayloadPerPacket;
    int payloadSize = 0;

    if(item->pDataSize % maxPayloadPerPacket)
    {
        numTotal++;
    }

    Log_Verbose(BHGRN"[NETWORK]"CRESET" Sending as %i packets. Max payload per packet: %i", numTotal, maxPayloadPerPacket);

    do
    {
        struct NetFragmentMessageHeader h = {      /* awkward code alert */
            .fragmentedMsgID = fragmentedMsgID,
            .fragmentedMsgTotalSize = item->pDataSize,
            .numFragments = numTotal,
            .sequenceNum = seqNum
        };
        payloadSize = off + maxPayloadPerPacket < item->pDataSize ? maxPayloadPerPacket : item->pDataSize - off;
        int numBytes = NetMsg_WriteReliableFragmentDataPacket(gPacketBuffer, ((u8*)item->pData) + off, payloadSize, numTotal, seqNum++,
            TrackReliableMessage(item->pData, payloadSize, off, &h, time, item->client),
            fragmentedMsgID, item->pDataSize);
        Sptr_AddRef(item->pData);
        sendPacket(serverOrClient, item->client, gPacketBuffer, numBytes);
        off += payloadSize;
    } while (maxPayloadPerPacket == payloadSize);
}


static void ResendReliablePacketsBase(double time, void * serverOrClient, SendPacketFn sendPacket)
{
    struct ReliableMessageTracker* pTracker = pReliableTrackerListHead;
    while(pTracker)
    {
        if(time - pTracker->lastSentTime > RESEND_IF_UNACKED_THRESHOLD)
        {
            if(pTracker->fragmentHeader.numFragments)
            {
                /* send a fragment packet */
                NetMsg_WriteReliableFragmentDataPacket(
                    gPacketBuffer,
                    (u8*)pTracker->data + pTracker->dataOffset,
                    pTracker->dataSize,
                    pTracker->fragmentHeader.numFragments,
                    pTracker->fragmentHeader.sequenceNum,
                    pTracker->ident,
                    pTracker->fragmentHeader.fragmentedMsgID,
                    pTracker->fragmentHeader.fragmentedMsgTotalSize
                );
                sendPacket(serverOrClient, pTracker->client, gPacketBuffer, pTracker->dataSize);
            }
            else
            {
                /* send a complete reliable packet */
                NetMsg_WriteReliableCompleteDataPacket(gPacketBuffer, (u8*)pTracker->data + pTracker->dataOffset, pTracker->dataSize, pTracker->ident);
                sendPacket(serverOrClient, pTracker->client, gPacketBuffer, pTracker->dataSize);
            }
        }
        pTracker = pTracker->pNext;
    }
}

static void DoTXQueueBase(struct NetworkThreadQueues* pQueues, void* serverOrClient, double time, SendPacketFn sendPacket)
{
    struct NetworkQueueItem item;
    while(TSQ_Dequeue(&pQueues->tx, &item))
    {
        if(item.pDataSize > NETCODE_MAX_PACKET_SIZE - NetMsg_SizeOfHeaders(ReliableDataMessageComplete))
        {
            Log_Verbose(BHGRN"[NETWORK]"CRESET" Message size %i too big to send in one packet, sending fragments.", item.pDataSize);
            /* send fragments here */
            SendMessageFragmentsBase(pQueues, serverOrClient, &item, time, sendPacket);
            Sptr_RemoveRef(item.pData);
        }
        else
        {
            if(item.bReliable)
            {

                int packetSize = NetMsg_WriteReliableCompleteDataPacket(gPacketBuffer, item.pData, item.pDataSize, 
                    TrackReliableMessage(item.pData, item.pDataSize, 0, NULL, time, item.client));
                
                Log_Verbose(BHGRN"[NETWORK]"CRESET" Sending complete reliable packet, size with headers %i.", packetSize);

                sendPacket(serverOrClient, item.client, gPacketBuffer, packetSize);
            }
            else
            {
                int packetSize = NetMsg_WriteUnreliableCompleteDataPacket(gPacketBuffer, item.pData, item.pDataSize);

                Log_Verbose(BHGRN"[NETWORK]"CRESET" Sending complete unreliable packet, size with headers %i.", packetSize);

                sendPacket(serverOrClient, item.client, gPacketBuffer, packetSize);
                Sptr_RemoveRef(item.pData);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Client

static void ClientFreePacket(void* serverOrClient, void* packet)
{
    netcode_client_free_packet(serverOrClient, packet);
}

static u8* ClientRecievePacket(void* serverOrClient, int clientIndex, int* packetBytes, uint64_t* packetSequence)
{
    return netcode_client_receive_packet(serverOrClient, packetBytes, packetSequence);
}

static void ClientSendPacket(void* clientOrServer, int clientIndex, u8* packet, int packetSize)
{
    netcode_client_send_packet(clientOrServer, packet, packetSize);
}

static void ClientRecievePackets(struct netcode_client_t * client, struct NetworkThreadQueues* pQueues)
{
    RecievePacketsBase(client, pQueues, &ClientRecievePacket, &ClientSendPacket, &ClientFreePacket);
}

static void DoTXQueueClient(struct NetworkThreadQueues* pQueues, struct netcode_client_t* server, double time)
{
    DoTXQueueBase(pQueues, server, time, &ClientSendPacket);
}

static void ClientResendReliablePackets(double time, struct netcode_client_t * client)
{
    ResendReliablePacketsBase(time, client, &ClientSendPacket);
}

static bool IsConnectedToServer(void* client, int clientIndex)
{
    return netcode_client_state( client ) == NETCODE_CLIENT_STATE_CONNECTED;
}

static void ServiceClientConnectionEvents(struct GameClient* gameClient, struct netcode_client_t * client, int clientIndex, struct NetworkThreadQueues* pQueues)
{
    ServiceConnectionEventsBase(gameClient, -1, pQueues, &IsConnectedToServer, client);
}


struct MatchMakingMessage gMatchmakingInfo;

/// @brief blocks until a match is found
/// @return game server address to connect to 
static char* MatchmakeClient()
{
    gCmdArgs.serverAddress = NULL;
    gMatchmakingInfo.type = MM_ClientSeekServer;
    strcpy(gMatchmakingInfo.data.clientInfo.playerInfo.username, gCmdArgs.playerName);
    double time = 0.0;
    double delta_time = 1.0 / 60.0;
    struct netcode_client_config_t client_config;
    netcode_default_client_config( &client_config );
    struct netcode_client_t * client = netcode_client_create( gCmdArgs.clientAddress, &client_config, time );
    uint64_t client_id = 0;
    netcode_random_bytes( (uint8_t*) &client_id, 8 );
    Log_Info( "client id is %lu" , client_id );

    NETCODE_CONST char* server_address = gCmdArgs.matchmakingServerAddress;

    client_id = 0;
    uint8_t user_data[NETCODE_USER_DATA_BYTES];

    netcode_random_bytes( (uint8_t*) &client_id, 8 );
    Log_Info( "client id is %lu" , client_id );

    uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

    if ( netcode_generate_connect_token( 1, &server_address, &server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, GAME_PROTOCOL_ID, private_key, user_data, connect_token ) != NETCODE_OK )
    {
        Log_Error( "error: failed to generate connect token" );
        return (void*)1;
    }

    netcode_client_connect( client, connect_token );

    bool quit = false;

    while ( !quit )
    {
        netcode_client_update( client, time );
        netcode_sleep( delta_time );

        
        netcode_client_send_packet(client, (const uint8_t*)&gMatchmakingInfo, sizeof(struct MatchMakingMessage));

        while ( 1 )     
        {
            int packet_bytes;
            uint64_t packet_sequence;
            void * packet = netcode_client_receive_packet( client, &packet_bytes, &packet_sequence );
            if(packet)
            {
                EASSERT(packet_bytes == sizeof(struct MatchMakingMessage));
                struct MatchMakingMessage* pMsg = packet;
                switch(pMsg->type)
                {
                case MM_ServerSeekClient:
                case MM_ClientSeekServer:
                    EASSERT(false);
                    break;
                case MM_GameConnection:
                    gCmdArgs.serverAddress = malloc(strlen(pMsg->data.peer.address) + 1);
                    strcpy(gCmdArgs.serverAddress, pMsg->data.peer.address);
                    break;
                }
                netcode_client_free_packet( client, packet );
                break;
            }
        }
        
        

        time += delta_time;
    }
game_found:
    netcode_client_destroy( client );
    return gCmdArgs.serverAddress;
}

static void MatchmakeClientServerUpdate(struct netcode_client_t * client)
{
    netcode_client_send_packet(client, (const uint8_t*)&gMatchmakingInfo, sizeof(struct MatchMakingMessage));
    while ( 1 )     
    {
        int packet_bytes;
        uint64_t packet_sequence;
        void * packet = netcode_client_receive_packet( client, &packet_bytes, &packet_sequence );
        if(packet)
        {
            EASSERT(packet_bytes == sizeof(struct MatchMakingMessage));
            struct MatchMakingMessage* pMsg = packet;
            switch(pMsg->type)
            {
            case MM_ServerSeekClient:
            case MM_ClientSeekServer:
                EASSERT(false);
                break;
            case MM_GameConnection:
                gCmdArgs.serverAddress = malloc(strlen(pMsg->data.peer.address) + 1);
                strcpy(gCmdArgs.serverAddress, pMsg->data.peer.address);
                break;
            }
            netcode_client_free_packet( client, packet );
            break;
        }
    }
}

DECLARE_THREAD_PROC(ClientThread, arg)
{
    
    struct GameClient gameClient = 
    {
        .state = GCS_Disconnected
    };
    struct NetworkThreadQueues* pQueues = arg; 
    netcode_set_printf_function(&NetcodeLog);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "error: failed to initialize netcode" );
        return (void*)1;
    }

    if(gCmdArgs.matchmakingServerAddress)
    {
        MatchmakeClient();
    }

    netcode_log_level( NETCODE_LOG_LEVEL_DEBUG );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "client" );

    struct netcode_client_config_t client_config;
    netcode_default_client_config( &client_config );
    client_config.network_simulator = NULL;//GetNetworkSimulator();
    struct netcode_client_t * client = netcode_client_create( gCmdArgs.clientAddress, &client_config, time );

    if ( !client )
    {
        Log_Error( "error: failed to create client" );
        return (void*)1;
    }

    NETCODE_CONST char* server_address = gCmdArgs.serverAddress;

    uint64_t client_id = 0;
    netcode_random_bytes( (uint8_t*) &client_id, 8 );
    Log_Info( "client id is %lu" , client_id );

    uint8_t user_data[NETCODE_USER_DATA_BYTES];
    netcode_random_bytes(user_data, NETCODE_USER_DATA_BYTES);

    uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

    if ( netcode_generate_connect_token( 1, &server_address, &server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, GAME_PROTOCOL_ID, private_key, user_data, connect_token ) != NETCODE_OK )
    {
        Log_Error( "error: failed to generate connect token" );
        return (void*)1;
    }

    netcode_client_connect( client, connect_token );

    bool quit = false;

    while ( !quit )
    {
        if(client_config.network_simulator)
        {
            netcode_network_simulator_update(client_config.network_simulator, time);
        }
        netcode_client_update( client, time );

        ServiceClientConnectionEvents(&gameClient, client, -1, pQueues);

        /* resend any reliable packets that haven't been acknowledged after a certain threshold of time */
        ClientResendReliablePackets(time, client);

        /* transmit any data from the game thread to clients */
        DoTXQueueClient(pQueues, client, time);

        /* recieve any packets from clients and push to game thread */
        ClientRecievePackets(client, pQueues);

        netcode_sleep( delta_time );

        time += delta_time;
    }

    if ( quit )
    {
        Log_Info( "shutting netcode thread down" );
    }

    netcode_client_destroy( client );

    if(client_config.network_simulator)
    {
        netcode_network_simulator_destroy(client_config.network_simulator);
    }

    netcode_term();
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Server

static void InitClients(struct GameClient* clients)
{
    for(int i=0; i < GAME_MAX_CLIENTS; i++)
    {
        clients[i].state = GCS_Disconnected;
    }
}

static bool IsServerClientConnected(void* pServerOrClient, int clientIndex)
{
    return netcode_server_client_connected(pServerOrClient, clientIndex);
}

static void ServerFreePacket(void* serverOrClient, void* packet)
{
    netcode_server_free_packet(serverOrClient, packet);
}

static u8* ServerRecievePacket(void* serverOrClient, int clientIndex, int* packetBytes, uint64_t* packetSequence)
{
    return netcode_server_receive_packet(serverOrClient, clientIndex, packetBytes, packetSequence);
}

static void ServerSendPacket(void* clientOrServer, int clientIndex, u8* packet, int packetSize)
{
    netcode_server_send_packet(clientOrServer, clientIndex, packet, packetSize);
}

static void ServerRecievePackets(struct netcode_server_t * server, struct NetworkThreadQueues* pQueues)
{
    RecievePacketsBase(server, pQueues, &ServerRecievePacket, &ServerSendPacket, &ServerFreePacket);
}

static void SendMessageFragmentsServer(struct NetworkThreadQueues* pQueues, struct netcode_server_t* server, struct NetworkQueueItem* item, double time)
{
    SendMessageFragmentsBase(pQueues, server, item, time, &ServerSendPacket);   
}

static void DoTXQueueServer(struct NetworkThreadQueues* pQueues, struct netcode_server_t* server, double time)
{
    DoTXQueueBase(pQueues, server, time, &ServerSendPacket);
}

static void ServerResendReliablePackets(double time, struct netcode_server_t * server)
{
    ResendReliablePacketsBase(time, server, &ServerSendPacket);
}

static void ServiceServerConnectionEvents(struct GameClient* client, struct netcode_server_t * server, int clientIndex, struct NetworkThreadQueues* pQueues)
{
    ServiceConnectionEventsBase(client, clientIndex, pQueues, &IsServerClientConnected, server);
}

/// @brief return a client to the matchmaking server that a game server uses to advertise itself for matchmaking
static struct netcode_client_t* CreateServerMatchmakingClient()
{
    gMatchmakingInfo.type = MM_ClientSeekServer;
    strcpy(gMatchmakingInfo.data.clientInfo.playerInfo.username, gCmdArgs.playerName);
    double time = 0.0;
    double delta_time = 1.0 / 60.0;
    struct netcode_client_config_t client_config;
    netcode_default_client_config( &client_config );
    struct netcode_client_t * client = netcode_client_create( gCmdArgs.clientAddress, &client_config, time );
    uint64_t client_id = 0;

    NETCODE_CONST char* server_address = gCmdArgs.matchmakingServerAddress;

    client_id = 0;
    uint8_t user_data[NETCODE_USER_DATA_BYTES];

    netcode_random_bytes( (uint8_t*) &client_id, 8 );
    Log_Info( "client id is %lu" , client_id );

    uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

    if ( netcode_generate_connect_token( 1, &server_address, &server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, GAME_PROTOCOL_ID, private_key, user_data, connect_token ) != NETCODE_OK )
    {
        Log_Error( "error: failed to generate connect token" );
        return (void*)1;
    }
    Log_Info("Generated connection token OK");
    netcode_client_connect( client, connect_token );
    Log_Info("Connected OK");
    return client;
}


DECLARE_THREAD_PROC(ClientServerThread, arg)
{
    netcode_log_level( NETCODE_LOG_LEVEL_DEBUG );
    netcode_set_printf_function(&NetcodeLog);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "failed to initialize netcode" );
        return (void*)1;
    }

    struct netcode_client_t* pMatchmakingServerClient = NULL;
    if(gCmdArgs.matchmakingServerAddress)
    {
        pMatchmakingServerClient = CreateServerMatchmakingClient();
    }

    struct GameClient gameClients[GAME_MAX_CLIENTS];
    struct NetworkThreadQueues* pQueues = arg; 
    InitClients(&gameClients[0]);
    
    

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "[server]" );

    NETCODE_CONST char* server_address = gCmdArgs.serverAddress;
    struct netcode_server_config_t server_config;
    netcode_default_server_config( &server_config );
    server_config.network_simulator = GetNetworkSimulator();
    server_config.protocol_id = GAME_PROTOCOL_ID;
    memcpy( &server_config.private_key, private_key, NETCODE_KEY_BYTES );

    struct netcode_server_t * server = netcode_server_create( server_address, &server_config, time );

    if ( !server )
    {
        Log_Info( "error: failed to create server" );
        return (void*)1;
    }

    netcode_server_start( server, GAME_MAX_CLIENTS );
    bool quit = false;
    while ( !quit )
    {
        /*NOTE: the network simulator stuff does NOT work. TODO: fix it!*/
        if(server_config.network_simulator)
        {
            netcode_network_simulator_update(server_config.network_simulator, time);
        }

        if(pMatchmakingServerClient)
        {
            netcode_client_update(pMatchmakingServerClient, time);
        }

        if(pMatchmakingServerClient)
        {
            MatchmakeClientServerUpdate(pMatchmakingServerClient);
        }

        netcode_server_update( server, time );
        
        /* pass messages to the game thread about clients connecting and disconnecting */
        for(int i=0; i<GAME_MAX_CLIENTS; i++)
        {
            ServiceServerConnectionEvents(&gameClients[i], server, i, pQueues);
        }

        /* resend any reliable packets that haven't been acknowledged after a certain threshold of time */
        ServerResendReliablePackets(time, server);

        /* transmit any data from the game thread to clients */
        DoTXQueueServer(pQueues, server, time);

        /* recieve any packets from clients and push to game thread */
        ServerRecievePackets(server, pQueues);

        netcode_sleep( delta_time );

        time += delta_time;
    }

    if ( quit )
    {
        Log_Info( "shutting netcode thread down" );
    }

    //netcode_server_destroy( server );

    if(server_config.network_simulator)
    {
        netcode_network_simulator_destroy(server_config.network_simulator);
    }

    netcode_term();
}

void WrapAroundHandlerBase(void* pItemToBeLost, const char* message)
{
    struct NetworkQueueItem* pItem = pItemToBeLost;
    Sptr_RemoveRef(pItem->pData);
    Log_Warning(message);
}


void OnConnectionEventTSQueueWrapAround(void* pItemToBeLost)
{
    WrapAroundHandlerBase(pItemToBeLost, "Network thread Connection event queue wrapped around, packets lost. It must not have been emptied quick enough");
}

void OnTXTSQueueWrapAround(void* pItemToBeLost)
{
    WrapAroundHandlerBase(pItemToBeLost, "Network thread TX queue wrapped around, packets lost. It must not have been emptied quick enough");
}

void OnRXTSQueueWrapAround(void* pItemToBeLost)
{
    WrapAroundHandlerBase(pItemToBeLost, "Network thread RX queue wrapped around, packets lost. It must not have been emptied quick enough");
}

void NW_Init()
{
    gReliableTrackerPool = NEW_OBJECT_POOL(struct ReliableMessageTracker, 128);
    gFragmentedMessageRecieverPool = NEW_OBJECT_POOL(struct FragmentedMessageReciever, 128);
    gRole = gCmdArgs.role;

    switch(gRole)
    {
    case GR_Client:
    case GR_ClientServer:

        pQueues = malloc(sizeof(struct NetworkThreadQueues));
        memset(pQueues, 0, sizeof(struct NetworkThreadQueues));
        TSQ_Init(&pQueues->rx, sizeof(struct NetworkQueueItem), 32, &OnRXTSQueueWrapAround);
        TSQ_Init(&pQueues->tx, sizeof(struct NetworkQueueItem), 32, &OnTXTSQueueWrapAround);
        TSQ_Init(&pQueues->connectionEvents, sizeof(struct NetworkConnectionEvent), 32, OnConnectionEventTSQueueWrapAround);
    }
    switch (gRole)
    {
    case GR_Client:
        gNetworkThread = StartThread(&ClientThread, pQueues);
        break;
    case GR_ClientServer:
        gNetworkThread = StartThread(&ClientServerThread, pQueues);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// API

bool NW_DequeueData(struct NetworkQueueItem* pOut)
{
    return TSQ_Dequeue(&pQueues->rx, pOut);
}

bool NW_DequeueConnectionEvent(struct NetworkConnectionEvent* pOut)
{
    return TSQ_Dequeue(&pQueues->connectionEvents, pOut);
}

void NW_EnqueueData(struct NetworkQueueItem* pIn)
{
    TSQ_Enqueue(&pQueues->tx, pIn);
}

enum GameRole NW_GetRole()
{
    return gRole;
}