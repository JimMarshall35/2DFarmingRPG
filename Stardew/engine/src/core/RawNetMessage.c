#include "RawNetMessage.h"
#include "Log.h"
#include "netcode.h"
#include <string.h>

void NetMsg_Parse(u8* data, enum NetRawMessageType* pOutType, u8** outBody)
{
    *pOutType = *((u32*)data);
    switch(*pOutType)
    {
    case ReliableDataMessageAck:
        *outBody = NULL;
        break;
    case UnreliableDataMessageComplete:
        *outBody = data + sizeof(u32);
        break;
    case ReliableDataMessageComplete:
        *outBody = data + sizeof(u32) + sizeof(struct NetReliableMessageHeader);
        break;
    case ReliableDataMessageFragment: /* part of a packet that's too big to send all at once */
        *outBody = data + sizeof(u32) + sizeof(struct NetReliableMessageHeader) + sizeof(struct NetFragmentMessageHeader);
        break;
    }
}

struct NetReliableMessageHeader* NetMsg_GetReliableHeader(u8* data)
{
    return (struct NetReliableMessageHeader*)(data + sizeof(u32));
}

struct NetFragmentMessageHeader* NetMsg_GetFragmentHeader(u8* data)
{
    return (struct NetFragmentMessageHeader*)(data + sizeof(u32) + sizeof(struct NetReliableMessageHeader));
}

u32 NetMsg_GetReliableMessageIdentifier()
{
    static u32 i = 0;
    return ++i;
}

int NetMsg_WriteUnreliableCompleteDataPacket(u8* dataOut, u8* dataIn, int dataSize)
{
    int totalBytes2Write = dataSize + sizeof(u32);
    if(totalBytes2Write <= NETCODE_MAX_PACKET_SIZE)
    {
        *((u32*)dataOut) = UnreliableDataMessageComplete;
        dataOut += sizeof(u32);
        memcpy(dataOut, dataIn, dataSize);
        return totalBytes2Write;
    }
    else
    {
        Log_Error("Trying to write too much data in a single unreliable packet");
    }
    return 0;
}

int NetMsg_WriteReliableCompleteDataPacket(u8* dataOut, u8* dataIn, int dataSize, u32 messageIdentifier)
{
    int totalBytes2Write = dataSize + sizeof(u32) + sizeof(struct NetReliableMessageHeader);
    if(totalBytes2Write <= NETCODE_MAX_PACKET_SIZE)
    {
        *((u32*)dataOut) = ReliableDataMessageComplete;
        dataOut += sizeof(u32);
        struct NetReliableMessageHeader reliable =
        {
            .messageIdentifier = messageIdentifier
        };
        *((struct NetReliableMessageHeader*)dataOut) = reliable;
        dataOut += sizeof(struct NetReliableMessageHeader);
        memcpy(dataOut, dataIn, dataSize);
        return totalBytes2Write;
    }
    else
    {
        Log_Error("Trying to write too much data in a single reliable packet - you need to send it as fragments");
    }   
    return 0;
}

int NetMsg_WriteReliableFragmentDataPacket(u8* dataOut, u8* dataIn, int dataSize, u16 numFragments, u16 sequenceNumber, u32 messageIdentifier, u32 fragmentedMsgID, u32 fragmentedMsgTotalSize)
{
    int totalBytes2Write = dataSize + sizeof(u32) + sizeof(struct NetReliableMessageHeader) + sizeof(struct NetFragmentMessageHeader);
    if(totalBytes2Write <= NETCODE_MAX_PACKET_SIZE)
    {
        *((u32*)dataOut) = ReliableDataMessageFragment;
        dataOut += sizeof(u32);
        struct NetReliableMessageHeader reliable =
        {
            .messageIdentifier = messageIdentifier
        };
        *((struct NetReliableMessageHeader*)dataOut) = reliable;
        dataOut += sizeof(struct NetReliableMessageHeader);
        struct NetFragmentMessageHeader fragment = 
        {
            .numFragments = numFragments,
            .sequenceNum = sequenceNumber,
            .fragmentedMsgID = fragmentedMsgID,
            .fragmentedMsgTotalSize = fragmentedMsgTotalSize
        };
        *((struct NetFragmentMessageHeader*)dataOut) = fragment;
        dataOut += sizeof(struct NetFragmentMessageHeader);
        memcpy(dataOut, dataIn, dataSize);
        return totalBytes2Write;
    }
    else
    {
        Log_Error("Trying to write too much data in a single reliable packet - you need to send it as fragments");

    }
    return 0;
}

int NetMsg_WriteReliableDataAckPacket(u8* dataOut, u32 messageIdentifier)
{
    int totalBytes2Write = sizeof(u32) * 2;
    u32* p = (u32*)dataOut;
    *p++ = ReliableDataMessageAck;
    *p = messageIdentifier;
    return totalBytes2Write;
}

int NetMsg_SizeOfHeaders(enum NetRawMessageType msgType)
{
    switch (msgType)
    {
    case UnreliableDataMessageComplete: return sizeof(u32);
    case ReliableDataMessageComplete:   return sizeof(u32) + sizeof(struct NetReliableMessageHeader);
    case ReliableDataMessageFragment:   return sizeof(u32) + sizeof(struct NetReliableMessageHeader) + sizeof(struct NetFragmentMessageHeader);
    case ReliableDataMessageAck:        return sizeof(u32);
    }
    return 0;
}

u32 NetMsg_GetAckedIdentifier(u8* data)
{
    return *(((u32*)data) + 1);
}
