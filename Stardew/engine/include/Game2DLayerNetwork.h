
/**
    @file Game2DLayerNetwork.h
    @brief
    Networking layer specific to game 2d layer 
    
*/
#ifndef GAME2DLAYER_NETWORK_H
#define GAME2DLAYER_NETWORK_H

#include <stdbool.h>
#include "IntTypes.h"
struct GameFrameworkLayer;
struct BinarySerializer;
struct GameLayer2DData;
struct Entity2D;
struct Entity2DCollection;

enum G2DPacketType
{
    G2DPacket_RequestLevelData,
    G2DPacket_LevelDataResponseData,
    G2DPacket_RPC,
    G2DPacket_WorldState
};


struct TilemapEdit
{
    u16 x, y;
    u16 layer;
    TileIndex newVal;
};

enum Game2DRPCType
{
	G2DRPC_CreateEntity,
	G2DRPC_DestroyEntity,
    G2DRPC_AdjustNetworkID,
    G2DRPC_LastEngineRPC /* leave at end */
};

/// @brief callback to construct an rpc message of a given type, pRPCStruct is a struct containing relevant data to construct your rpc message
typedef void(*RPCMessageConstructorFn)(struct BinarySerializer* pBS, void* pRPCStruct);

/// @brief callback to handle an RPC message, to be read from the binary serializer, passed Game2DLayerData* so the RPC can do stuff
typedef void(*RPCMessageHandlerFn)(struct BinarySerializer* pBS, struct GameLayer2DData* pData, int client);

/// @brief An RPC implemented by a users game
struct UserGame2DRPC
{
    /// @brief RPC type - implement an RPC enum type for your game with values starting at G2DRPC_LastEngineRPC, register RPCs in order
    int rpcType; 

    /// @brief callback to construct an rpc message of a given type, pRPCStruct is a struct containing relevant data to construct your rpc message
    RPCMessageConstructorFn ctor;

    /// @brief callback to handle an RPC message, to be read from the binary serializer, passed Game2DLayerData* so the RPC can do stuff
    RPCMessageHandlerFn handler;
};

/// @brief Register a user RPC, call once at startup, call in order your games rpc types appear in the enum
/// @param rpc 
void G2D_RegisterUserRPC(struct UserGame2DRPC* rpc);

typedef void(*PacketExtensionNoArgsFn)(struct BinarySerializer*);

/// @brief extend the packet that the client sends to request level data from the server. By default tile maps 
/// and serialized entities are included, this callback is called before this Game2DLayer managed level data is serialized - see WfNetwork.c
/// @param fn callback to call
void G2D_Extend_RequestLevelDataMessage(PacketExtensionNoArgsFn fn);

/// @brief TODO: should really be static to this file, probably 
void G2D_Enqueue_RequestLevelData();

enum G2DPacketType G2D_ParsePacket(u8* pPacket, u8** pOutBody, int* outHeaderSize);

/// @brief Enqueue a packet that will cause all entities with bSerializeInNetworkUpdate == true to be saved into a packet, the serializer context will be SCTX_NetworkUpdate
/// @param pData The Game2dLayer
/// @param clientI The client index to send to - only relevant if you're the server
void G2D_Enqueue_Worldstate_Packet(struct GameLayer2DData* pData, int clientI);

/// @brief 
/// @param packet
/// @param pOutBody
enum Game2DRPCType G2D_ParseRPCPacket(u8* packet, u8** pOutBody);

/// @brief do RPCs, call once a packet has been confirmed as a G2DPacket_RPC, call on both client and server
/// @param pData 
/// @param pRPCData 
/// @param client the number of the client who has sent the RPC, or -1 if the server has sent it and you are a client
void G2D_DoRPC(struct GameFrameworkLayer* pLayer, struct GameLayer2DData* pData, u8* pRPCData, int client);

void G2D_PollNetworkQueueClient(struct GameFrameworkLayer* pLayer, float deltaT);

void G2D_PollNetworkQueueServer(struct GameFrameworkLayer* pLayer, float deltaT);

/// @brief Send an RPC
/// @param client client to send to - send -1 if you're a client yourself
/// @param type type of rpc to send
/// @param pRPCData a struct appropriate to construct the rpc message - see src 
void G2D_SendRPC(int client, enum Game2DRPCType type, void* pRPCData);


/// @brief try to find the entity with a given net ID
/// @param pCollection collection to search
/// @param netID netID to search for
/// @return NULL if not found
struct Entity2D* G2D_FindEntityWithNetID(struct Entity2DCollection* pCollection, int netID);

/// @brief call from the game thread if you're the server to determine if a client is connected
/// @param i 
/// @return 
bool G2D_IsClientConnected(int i);

/////////////////////////////////////////////////////////////////////////////////////////////////// RPC data structs

struct AdjustNetID_RPC
{
    int oldNetID;
    int newNetID;
};

struct CreateEntity_RPC
{
    struct Entity2D* pEnt;
    struct GameLayer2DData* pData;
};

struct DeleteEntity_RPC
{
    int netID;
};


#endif
