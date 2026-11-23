#ifndef RAWNETMESSAGE_H
#define RAWNETMESSAGE_H

#include "IntTypes.h"

enum NetRawMessageType
{
    UnreliableDataMessageComplete,
    ReliableDataMessageComplete,
    /// @brief part of a packet that's too big to send all at once
    ReliableDataMessageFragment,
    ReliableDataMessageAck,
};

struct NetReliableMessageHeader
{
    u32 messageIdentifier;
};

struct NetFragmentMessageHeader
{
    u32 fragmentedMsgID;
    u32 fragmentedMsgTotalSize;

    /// @brief how many fragments in total
    u16 numFragments;
    
    /// @brief which message in the sequence is this one
    u16 sequenceNum;
};

void NetMsg_Parse(u8* data, enum NetRawMessageType* pOutType, u8** outBody);

struct NetReliableMessageHeader* NetMsg_GetReliableHeader(u8* data);

struct NetFragmentMessageHeader* NetMsg_GetFragmentHeader(u8* data);

u32 NetMsg_GetAckedIdentifier(u8* data);


/// @brief 
/// @param dataOut 
/// @param dataIn 
/// @param dataSize 
/// @return number of bytes written
int NetMsg_WriteUnreliableCompleteDataPacket(u8* dataOut, u8* dataIn, int dataSize);

/// @brief 
/// @return 
u32 NetMsg_GetReliableMessageIdentifier();

/// @brief 
/// @param dataOut 
/// @param dataIn 
/// @param dataSize 
/// @param messageIdentifier 
/// @return number of bytes written
int NetMsg_WriteReliableCompleteDataPacket(u8* dataOut, u8* dataIn, int dataSize, u32 messageIdentifier);

/// @brief 
/// @param dataOut 
/// @param dataIn 
/// @param dataSize 
/// @param numFragments 
/// @param sequenceNumber 
/// @param messageIdentifier 
/// @param fragmentedMsgID 
/// @param fragmentedMsgTotalSize 
/// @return number of bytes written
int NetMsg_WriteReliableFragmentDataPacket(u8* dataOut, u8* dataIn, int dataSize, u16 numFragments, u16 sequenceNumber, u32 messageIdentifier, u32 fragmentedMsgID, u32 fragmentedMsgTotalSize);

/// @brief 
/// @param dataOut 
/// @param messageIdentifier 
/// @return number of bytes written
int NetMsg_WriteReliableDataAckPacket(u8* dataOut, u32 messageIdentifier);

/// @brief 
/// @param  
/// @return size of headers before message body 
int NetMsg_SizeOfHeaders(enum NetRawMessageType msgType);


#endif
