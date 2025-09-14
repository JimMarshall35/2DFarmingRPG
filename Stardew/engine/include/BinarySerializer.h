#ifndef BINARY_SERIALIZER_H
#define BINARY_SERIALIZER_H
#ifdef __cplusplus
extern "c" {
#endif
#include <stdbool.h>
#include "IntTypes.h"

	struct BinarySerializer
	{
		bool bSaving;
		char* pData;
		int pDataSize;
		char* pReadPtr;
		char* pPath;
	};

	void BS_CreateForLoad(const char* path, struct BinarySerializer* pOutSerializer);
	void BS_CreateForSave(const char* path, struct BinarySerializer* pOutSerializer);
	void BS_Finish(struct BinarySerializer* pOutSerializer);


	void BS_SerializeI64(i64 val, struct BinarySerializer* pSerializer);
	void BS_SerializeU64(u64 val, struct BinarySerializer* pSerializer);
	void BS_SerializeI32(i32 val, struct BinarySerializer* pSerializer);
	void BS_SerializeU32(u32 val, struct BinarySerializer* pSerializer);
	void BS_SerializeI16(i16 val, struct BinarySerializer* pSerializer);
	void BS_SerializeU16(u16 val, struct BinarySerializer* pSerializer);
	void BS_SerializeI8(i8 val, struct BinarySerializer* pSerializer);
	void BS_SerializeU8(u8 val, struct BinarySerializer* pSerializer);
	void BS_SerializeBool(bool val, struct BinarySerializer* pSerializer);
	void BS_SerializeFloat(float val, struct BinarySerializer* pSerializer);
	void BS_SerializeDouble(double val, struct BinarySerializer* pSerializer);
	void BS_SerializeString(char* val, struct BinarySerializer* pSerializer);
	void BS_SerializeBytes(char* val, u32 len, struct BinarySerializer* pSerializer);


	void BS_DeSerializeI64(i64* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeU64(u64* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeI32(i32* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeU32(u32* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeI16(i16* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeU16(u16* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeI8(i8* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeU8(u8* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeBool(bool* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeFloat(float* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeDouble(double* val, struct BinarySerializer* pSerializer);
	void BS_DeSerializeString(char** val, struct BinarySerializer* pSerializer);
	void BS_BytesRead(struct BinarySerializer* pSerializer, u32 numBytes, char* pDst);

#ifdef __cplusplus
}
#endif
#endif