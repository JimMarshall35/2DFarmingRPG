#include "BinarySerializer.h"
#include "FileHelpers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "DynArray.h"
#include "AssertLib.h"
#include "Log.h"

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
	}	
	
}

void BS_SerializeU32(u32 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(u32), 1, pSerializer->pFile);
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
	}
	
}

void BS_SerializeU8(u8 val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(u8), 1, pSerializer->pFile);
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
	}
	
}

void BS_SerializeFloat(float val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(&val, sizeof(float), 1, pSerializer->pFile);
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
	}
	
}

void BS_SerializeBytesNoLen(const char* val, u32 len, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fwrite(val, len, 1, pSerializer->pFile);
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
	}
}

void BS_DeSerializeU64(u64* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u64), 1, pSerializer->pFile);
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
	}
}

void BS_DeSerializeU32(u32* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u32), 1, pSerializer->pFile);
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
	}
}

void BS_DeSerializeU16(u16* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u16), 1, pSerializer->pFile);
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
	}
}

void BS_DeSerializeU8(u8* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(u8), 1, pSerializer->pFile);
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
	}
}

void BS_DeSerializeFloat(float* val, struct BinarySerializer* pSerializer)
{
	switch (pSerializer->ctx)
	{
	case SCTX_ToFile:
		fread(val, sizeof(float), 1, pSerializer->pFile);
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
	}
}