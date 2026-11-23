#include "DynArray.h"
#include <string.h>
#include <stdlib.h> 



void* VectorInit(unsigned int itemSize)
{
	VectorData* data = (VectorData*)malloc(sizeof(VectorData) + itemSize);
	data->itemSize = itemSize;
	data->size = 0;
	data->capacity = 1;
	return data + 1;
}

void* VectorResize(void* vector, unsigned int size)
{
	VectorData* pData = ((VectorData*)vector) - 1;
	if (size <= pData->capacity)
	{
		return vector;
	}
	else if (size > pData->capacity)
	{
		VectorData* pNewAlloc = malloc(sizeof(VectorData) + pData->itemSize * size);
		if (pNewAlloc)
		{
			pNewAlloc->size = pData->size;
			pData->capacity = size;
			memcpy(pNewAlloc, pData, sizeof(VectorData) + pData->size * pData->itemSize);

			free(pData);
		}
		else
		{
			return NULL;
		}
		
		return pNewAlloc + 1;
	}
	return vector;
}

void* VectorPush(void* vector, void* item)
{
	VectorData* pData = ((VectorData*)vector) - 1;
	if (++pData->size > pData->capacity)
	{
		vector = VectorResize(vector, pData->capacity * 2);
		pData = ((VectorData*)vector) - 1;
	}
	void* topSpot = VectorTop(vector);
	memcpy(topSpot, item, pData->itemSize);
	return vector;
}

void* VectorPop(void* vector)
{
	VectorData* pData = ((VectorData*)vector) - 1;
	void* data = VectorTop(vector);
	pData->size--;
	return data;
}

void* VectorTop(void* vector)
{
	VectorData* pData = ((VectorData*)vector) - 1;
	return (char*)vector + ((pData->size - 1) * pData->itemSize);
}

void* VectorClear(void* vector)
{
	VectorData* pData = ((VectorData*)vector) - 1;
	pData->size = 0;
	return vector;
}

//unsigned int VectorSize(void* vector)
//{
//	VectorData* pData = ((VectorData*)vector) - 1;
//	return pData->size;
//}

void DestoryVector(void* vector)
{
	VectorData* pData = ((VectorData*)vector) - 1;
	free(pData);
}
