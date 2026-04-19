#include "BinarySerializer.h"
#include "FileHelpers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "DynArray.h"
#include "AssertLib.h"
#include "Network.h"
#include "Log.h"

void BS_CreateForLoadFromBuffer(void* buf, int size, struct BinarySerializer* pOutSerializer)
{
	memset(pOutSerializer, 0, sizeof(struct BinarySerializer));
	pOutSerializer->bSaving = false;
	pOutSerializer->pData = buf;
	pOutSerializer->pReadPtr = pOutSerializer->pData;
	pOutSerializer->pPath = NULL;
	pOutSerializer->pDataSize = size;
	pOutSerializer->ctx = SCTX_ToNetwork;
}

void BS_CreateForLoad(const char* path, struct BinarySerializer* pOutSerializer)
{
	memset(pOutSerializer, 0, sizeof(struct BinarySerializer));
	pOutSerializer->bSaving = false;
	pOutSerializer->pFile = fopen(path, "rb");
	pOutSerializer->pPath = malloc(strlen(path) + 1);
	pOutSerializer->ctx = SCTX_ToFile;
	strcpy(pOutSerializer->pPath, path);
}

void BS_CreateForSave(const char* path, struct BinarySerializer* pOutSerializer)
{
	memset(pOutSerializer, 0, sizeof(struct BinarySerializer));
	pOutSerializer->bSaving = true;
	pOutSerializer->pFile = fopen(path, "wb");
	pOutSerializer->pPath = malloc(strlen(path) + 1);
	pOutSerializer->ctx = SCTX_ToFile;
	strcpy(pOutSerializer->pPath, path);
}

void BS_CreateForSaveToNetwork(struct BinarySerializer* pOutSerializer, int client)
{
	memset(pOutSerializer, 0, sizeof(struct BinarySerializer));
	pOutSerializer->bSaving = true;
	pOutSerializer->pData = NEW_VECTOR(char);
	pOutSerializer->pPath = NULL;
	pOutSerializer->ctx = SCTX_ToNetwork;
	pOutSerializer->toClient = client;
}

void BS_Finish(struct BinarySerializer* pOutSerializer)
{
	bool bReliable = true;
	switch (pOutSerializer->ctx)
	{
	case SCTX_ToFile:
		fclose(pOutSerializer->pFile);
		EASSERT(pOutSerializer->pPath);
		free(pOutSerializer->pPath);
		break;
	case SCTX_ToNetworkUpdate:
		bReliable = false; /* intentional fallthrough */
	case SCTX_ToNetwork:
		if (pOutSerializer->bSaving)
		{
			struct NetworkQueueItem nci;
			nci.bReliable = bReliable; /* TODO: MAKE OPTIONAL */
			nci.client = pOutSerializer->toClient;
			nci.pData = Sptr_New(VectorSize(pOutSerializer->pData), NULL);
			nci.pDataSize = VectorSize(pOutSerializer->pData);
			
			memcpy(nci.pData, pOutSerializer->pData, VectorSize(pOutSerializer->pData));
			NW_EnqueueData(&nci);
			DestoryVector(pOutSerializer->pData);
		}
		break;
	default:
		break;
	}
	
}

void BS_SerializeI64(i64 val, struct BinarySerializer* pSerializer)
{
	char* pIn = (char*)&val;
	for (int i = 0; i < sizeof(i64); i++)
	{
		pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
	}
}

void BS_SerializeU64(u64 val, struct BinarySerializer* pSerializer)
{
	char* pIn = (char*)&val;
	for (int i = 0; i < sizeof(u64); i++)
	{
		pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
	}
}

void BS_SerializeI32(i32 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(i32), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(i32); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}	
	
}

void BS_SerializeU32(u32 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(u32), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(u32); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}
}

void BS_SerializeI16(i16 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		/* code */
		fwrite(&val, sizeof(i16), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(i16); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}
}

void BS_SerializeU16(u16 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		/* code */
		fwrite(&val, sizeof(u16), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(u16); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}
}

void BS_SerializeI8(i8 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		/* code */
		fwrite(&val, sizeof(i8), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			pSerializer->pData = VectorPush(pSerializer->pData, &val);
		}
		break;
	}
	
}

void BS_SerializeU8(u8 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(u8), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			pSerializer->pData = VectorPush(pSerializer->pData, &val);
		}
		break;
	}
}

void BS_SerializeBool(bool val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(bool), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(bool); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}
	
}

void BS_SerializeFloat(float val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(float), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(float); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}

	
}

void BS_SerializeDouble(double val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(double), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			char* pIn = (char*)&val;
			for (int i = 0; i < sizeof(double); i++)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, &pIn[i]);
			}
		}
		break;
	}
}

void BS_SerializeString(const char* val, struct BinarySerializer* pSerializer)
{
	if (!val)
	{
		BS_SerializeU32(0, pSerializer);
		return;
	}
	int len = strlen(val);
	BS_SerializeU32(len, pSerializer);
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		{
			if(*val)
			{
				fwrite(val, len, 1, pSerializer->pFile);
			}
		}
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			while (*val)
			{
				pSerializer->pData = VectorPush(pSerializer->pData, val++);
			}
		}
		break;
	}
	
}

void BS_SerializeBytesNoLen(const char* val, u32 len, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(val, len, 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			for (int i = 0; i < len; i++)
			{
				BS_SerializeU8((u8)val[i], pSerializer);
			}
		}
		break;
	}
	
}


void BS_SerializeBytes(const char* val, u32 len, struct BinarySerializer* pSerializer)
{
	BS_SerializeU32(len, pSerializer);
	BS_SerializeBytesNoLen(val, len, pSerializer);
	
}

void BS_DeSerializeI64(i64* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(i64), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((i64*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(i64);
		}
		break;
	}
}

void BS_DeSerializeU64(u64* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u64), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((u64*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(u64);
		}
		break;
	}
}

void BS_DeSerializeI32(i32* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(i32), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((i32*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(i32);
		}
		break;
	}
}

void BS_DeSerializeU32(u32* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u32), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((u32*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(u32);
		}
		break;
	}
}

void BS_DeSerializeI16(i16* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(i16), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((i16*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(i16);
		}
		break;
	}
}

void BS_DeSerializeU16(u16* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u16), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((u16*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(u16);
		}
		break;
	}
	
}

void BS_DeSerializeI8(i8* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(i8), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((i8*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(i8);
		}
		break;
	}
}

void BS_DeSerializeU8(u8* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u8), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((u8*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(u8);
		}
		break;
	}
}

void BS_DeSerializeBool(bool* val, struct BinarySerializer* pSerializer)
{
	char r = 0;
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(bool), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((bool*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(bool);
		}
		break;
	}
}

void BS_DeSerializeFloat(float* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(float), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((float*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(float);
		}
		break;
	}	
}

void BS_DeSerializeDouble(double* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(double), 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			*val = *((double*)pSerializer->pReadPtr);
			pSerializer->pReadPtr += sizeof(double);
		}
		break;
	}
}

void BS_DeSerializeStringInto(char* buf, struct BinarySerializer* pSerializer)
{
	u32 len = 0;
	BS_DeSerializeU32(&len, pSerializer);
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(buf, len, 1, pSerializer->pFile);
		buf[len] = '\0';
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			for(int i=0; i<len; i++)
			{
				BS_DeSerializeI8(buf++, pSerializer);
			}
			*buf = '\0';
		}
		break;
	}
}

void BS_DeSerializeString(char** val, struct BinarySerializer* pSerializer)
{
	u32 len = 0;
	BS_DeSerializeU32(&len, pSerializer);
	*val = malloc(len + 1);
	char* buf = *val;
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(buf, len, 1, pSerializer->pFile);
		buf[len] = '\0';
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			for(int i=0; i<len; i++)
			{
				BS_DeSerializeI8(buf++, pSerializer);
			}
			*buf = '\0';
		}
		break;
	}
}


void BS_BytesRead(struct BinarySerializer* pSerializer, u32 numBytes, char* pDst)
{
	u32 len = 0;
	//BS_DeSerializeU32(&len, pSerializer);
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(pDst, numBytes, 1, pSerializer->pFile);
		break;
	case SCTX_ToNetwork:
	case SCTX_ToNetworkUpdate:
		{
			memcpy(pDst, pSerializer->pReadPtr, numBytes);
			pSerializer->pReadPtr += numBytes;
		}
		break;
	}
}