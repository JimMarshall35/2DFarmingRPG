#ifndef DYNARRAY_H
#define DYNARRAY_H
#ifdef __cplusplus
extern "C"{
#endif 


#include "IntTypes.h"

void* VectorInit(unsigned int itemSize);
void* VectorResize(void* vector, unsigned int size);
void* VectorPush(void* vector, void* item);
void* VectorPop(void* vector);
void* VectorTop(void* vector);
void* VectorClear(void* vector);


//unsigned int VectorSize(void* vector);
void DestoryVector(void* vector);

/// @brief 16 byte aligned
typedef struct
{
	u32 itemSize;
	u32 capacity;
	u32 size;
	u32 _padding;
} VectorData;

#define VectorSize(vector) ((((VectorData*)vector) - 1)->size)
#define VectorCapacity(vector) ((((VectorData*)vector) - 1)->capacity)
#define VectorData_DEBUG(vector)(((VectorData*)vector) - 1)

#define NEW_VECTOR(a) ((a*)VectorInit(sizeof(a)));
#define VECTOR(a) a*

#ifdef __cplusplus
}
#endif

#endif