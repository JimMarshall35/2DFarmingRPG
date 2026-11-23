#include "StringKeyHashMap.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "AssertLib.h"
#include "Log.h"

#define STARDEW_HASHMAP_DEFAULT_LOAD_FACTOR 0.75f

/*

	A hashmap with linear probing, strings as keys, that uses the djb2 hashing algorithm.
	
	Buckets (or KVP) contain the raw hash of the string key.

	Delete implementation moves a bucket ahead of the "hole" who's hashed index refers to a bucket BEHIND the hole
	into the "hole". This is then repeated for the new hole recursively (although not actually recursive).

	An alternative delete would be to mark the deleted bucket as deleted, so that linear probes skip over it.
	This would be faster but use more memory, with the deleted cells only usable when the hash map next resizes, effectively.

	Will resize when load factor reaches a given threshold.

*/

struct KVP
{
	struct KVP* pNext;
	struct KVP* pPrev;
	char* pKey;
	unsigned int keyHash;
};

unsigned int djb2(char* str)
{
	unsigned int hash = 5381;
	int c;

	while (c = *(unsigned char*)str++)
	{
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

static void HashmapInitBase(struct HashMap* pMap, int capacity, int valSize)
{
	memset(pMap, 0, sizeof(struct HashMap));
	pMap->capacity = capacity;
	pMap->size = 0;
	pMap->valueSize = valSize;
	pMap->pData = malloc(capacity * (sizeof(struct KVP) + valSize));
	memset(pMap->pData, 0, capacity * (sizeof(struct KVP) + valSize));
	
}

void HashmapInit(struct HashMap* pMap, int capacity, int valSize)
{
	HashmapInitBase(pMap, capacity, valSize);
	pMap->fLoadFactor = STARDEW_HASHMAP_DEFAULT_LOAD_FACTOR;
}

void HashmapInitWithLoadFactor(struct HashMap* pMap, int capacity, int valSize, float loadFactor)
{
	HashmapInitBase(pMap, capacity, valSize);
	pMap->fLoadFactor = loadFactor;
}

static struct KVP* BucketAtIndex(struct HashMap* pMap, int i)
{
	return (struct KVP*)((char*)pMap->pData + i * (sizeof(struct KVP) + pMap->valueSize));
}

static int NextIndex(struct HashMap* pMap, int i)
{
	if (++i >= pMap->capacity)
	{
		return 0;
	}
	return i;
}

static int GetIndex(struct HashMap* pMap, struct KVP* pKVP)
{
	EASSERT((char*)pMap->pData <= (char*)pKVP);
	int offset = (char*)pKVP - (char*)pMap->pData;
	EASSERT(offset % (pMap->valueSize + sizeof(struct KVP)) == 0);
	return offset / (pMap->valueSize + sizeof(struct KVP));
}

/// <summary>
/// 
/// </summary>
/// <param name="pMap"></param>
/// <param name="key"></param>
/// <param name="bReturnVal">
/// if true, return the value, if false, return the KVP struct
/// </param>
/// <returns></returns>
static void* HashmapSearchBase(struct HashMap* pMap, char* key, bool bReturnVal)
{
	unsigned int hash = djb2(key);
	int index = hash % pMap->capacity;
	/* linear probe */
	struct KVP* pKVP = BucketAtIndex(pMap, index);

	while (true)
	{
		if (pKVP->pKey && pKVP->keyHash == hash)
		{
			return bReturnVal ? pKVP + 1 : pKVP;
		}
		else if (pKVP->pKey) // pKey set == "there's something in this bucket"
		{
			index = NextIndex(pMap, index);
			pKVP = BucketAtIndex(pMap, index);
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}

void* HashmapSearch(struct HashMap* pMap, char* key)
{
	return HashmapSearchBase(pMap, key, true);
}

struct KVP* HashmapSearchKVP(struct HashMap* pMap, char* key)
{
	return (struct KVP*)HashmapSearchBase(pMap, key, false);
}

/// <summary>
/// insert from the old allocation to the new
/// </summary>
/// <param name="pMap">
/// </param>
/// <param name="pKVPsrc">
/// kvp from old allocation
/// </param>
/// <param name="pNewAlloc">
/// New allocation
/// </param>
/// <returns>
/// The KVP inserted into pNewAlloc
/// </returns>
struct KVP* InsertKVPIntoNewAlloc(struct HashMap* pMap, struct KVP* pKVPsrc, char* pNewAlloc)
{
	unsigned int hash = pKVPsrc->keyHash;
	int index = hash % pMap->capacity;
	void* pAt = pNewAlloc + index * (sizeof(struct KVP) + pMap->valueSize);
	struct KVP* pKVP = (struct KVP*)pAt;

	while (true)
	{
		if (pKVP->pKey && pKVP->keyHash != hash)
		{
			// occupied bucket
			index = NextIndex(pMap, index);
			pKVP = (struct KVP*)((char*)pNewAlloc + index * (sizeof(struct KVP) + pMap->valueSize));
		}
		else
		{
			if (pKVP->pKey)
			{
				free(pKVP->pKey);
				EASSERT(false);
			}
			memset(pKVP, 0, sizeof(struct KVP));
			pKVP->keyHash = hash;
			pKVP->pKey = malloc(strlen(pKVPsrc->pKey) + 1);
			strcpy(pKVP->pKey, pKVPsrc->pKey);
			memcpy(pKVP + 1, pKVPsrc + 1, pMap->valueSize);
			return pKVP;
		}
	}
	return pKVP;
}

void HashmapPrintEntries(struct HashMap* pMap, const char* hashMapName)
{
	Log_Info("HASHMAP: '%s' current size: %i current capacity: %i", hashMapName, pMap->size, pMap->capacity);
	struct KVP* pKVP = pMap->pHead;
	while (pKVP)
	{
		EASSERT(pKVP->pKey);
		Log_Info("Key: %s hash: %u", pKVP->pKey, pKVP->keyHash);
		pKVP = pKVP->pNext;
	}
}

void HashmapResize(struct HashMap* pMap)
{
	//printf("RESIZING: current size: %i current capacity: %i", pMap->size, pMap->capacity);
	//PrintEntries(pMap);
	int newAllocSize = (pMap->capacity * (pMap->valueSize + sizeof(struct KVP))) * 2;
	char* pNewAlloc = malloc(newAllocSize);
	memset(pNewAlloc, 0, newAllocSize);
	pMap->capacity *= 2;
	struct KVP* pKVP = pMap->pHead;
	struct KVP* pNewHead = NULL;
	struct KVP* pNewTail = NULL;
	while (pKVP)
	{
		struct KVP* pKVPDst = InsertKVPIntoNewAlloc(pMap, pKVP, pNewAlloc);
		if (pNewHead == NULL)
		{
			pNewHead = pKVPDst;
			pNewTail = pNewHead;
		}
		else
		{
			pNewTail->pNext = pKVPDst;
			pKVPDst->pPrev = pNewTail;
			pNewTail = pKVPDst;
		}
		EASSERT(pKVP->pKey);
		free(pKVP->pKey);
		pKVP = pKVP->pNext;
	}
	if (pNewHead && !pNewTail)
	{
		pNewTail = pNewHead;
	}
	pMap->pHead = pNewHead;
	pMap->pEnd = pNewTail;
	free(pMap->pData);
	pMap->pData = pNewAlloc;
}

void* HashmapInsert(struct HashMap* pMap, char* key, void* pVal)
{
	float newLoad = (float)(pMap->size + 1) / (float)pMap->capacity;
	if(newLoad > pMap->fLoadFactor)
	{
		HashmapResize(pMap);
	}
	// todo: refactor InsertKVPIntoNewAlloc and from here downwards to use a common helper
	unsigned int hash = djb2(key);
	int index = hash % pMap->capacity;
	void* pAt = (char*)pMap->pData + index * (sizeof(struct KVP) + pMap->valueSize);
	struct KVP* pKVP = (struct KVP*)pAt;
	struct KVP* pNewKVP = NULL;
	bool bAdded = true;
	while (true)
	{
		if (pKVP->pKey && pKVP->keyHash != hash)
		{
			// occupied bucket
			index = NextIndex(pMap, index);
			pKVP = BucketAtIndex(pMap, index);
		}
		else
		{
			if (pKVP->pKey)
			{
				bAdded = false;
			}
			else
			{
				memset(pKVP, 0, sizeof(struct KVP));
				pKVP->keyHash = hash;
				pKVP->pKey = malloc(strlen(key) + 1);
				strcpy(pKVP->pKey, key);
			}
			
			memcpy(pKVP + 1, pVal, pMap->valueSize);
			pNewKVP = pKVP;
			break;
		}
	}
	EASSERT(pNewKVP);
	if (!bAdded)
	{
		return NULL;
	}
	if (!pMap->pHead)
	{
		pMap->pHead = pNewKVP;
		pMap->pEnd = pMap->pHead;
	}
	else
	{
		pMap->pEnd->pNext = pNewKVP;
		pNewKVP->pPrev = pMap->pEnd;
		pMap->pEnd = pNewKVP;
	}
	pMap->size++;
	return pNewKVP + 1;
}

struct KVP* FindMoveableKey(struct KVP* pHole, struct HashMap* pMap)
{
	int firstIndex = GetIndex(pMap, pHole);
	int next = NextIndex(pMap, firstIndex);
	struct KVP* pKVP = BucketAtIndex(pMap, next);
	while (pKVP->pKey)
	{
		int hashIndex = pKVP->keyHash % pMap->capacity;
		if (hashIndex <= firstIndex)
		{
			return pKVP;
		}
		next = NextIndex(pMap, next);
		pKVP = BucketAtIndex(pMap, next);
	}
	
	return NULL;
}

bool HashmapDeleteItem(struct HashMap* pMap, char* key)
{
	struct KVP* pFound = HashmapSearchKVP(pMap, key);
	if (pFound)
	{
		if (pFound->pNext)
		{
			pFound->pNext->pPrev = pFound->pPrev;
		}
		if (pFound->pPrev)
		{
			pFound->pPrev->pNext = pFound->pNext;
		}
		if (pMap->pHead == pFound)
		{
			pMap->pHead = pFound->pNext;
		}
		if (pMap->pEnd == pFound)
		{
			pMap->pEnd = pFound->pPrev;
		}
		free(pFound->pKey);
		memset(pFound, 0, sizeof(struct KVP));
		struct KVP* pMoveable = FindMoveableKey(pFound, pMap);
		while (pMoveable)
		{
			if (pMoveable->pNext)
			{
				pMoveable->pNext->pPrev = pFound;
			}
			if (pMoveable->pPrev)
			{
				pMoveable->pPrev->pNext = pFound;
			}
			if (pMap->pEnd == pMoveable)
			{
				pMap->pEnd = pFound;
			}
			if (pMap->pHead == pMoveable)
			{
				pMap->pHead = pFound;
			}
			memcpy(pFound, pMoveable, sizeof(struct KVP) + pMap->valueSize);
			memset(pMoveable, 0, sizeof(struct KVP) + pMap->valueSize);
			pFound = pMoveable;
			pMoveable = FindMoveableKey(pMoveable, pMap);
		}
		pMap->size--;
		return true;
	}
	return false;
}


struct HashmapKeyIterator GetKeyIterator(struct HashMap* pHashMap)
{
	struct HashmapKeyIterator itr = {
		pHashMap,
		pHashMap->pHead
	};
	return itr;
}

char* NextHashmapKey(struct HashmapKeyIterator* itr)
{
	EASSERT(itr->pHashMap);
	if (!itr->pOnKVP)
	{
		return NULL;
	}
	char* r = itr->pOnKVP->pKey;
	itr->pOnKVP = itr->pOnKVP->pNext;
	return r;
}

void HashmapDeInit(struct HashMap* pMap)
{
	struct KVP* pKVP = pMap->pHead;
	while (pKVP)
	{
		EASSERT(pKVP->pKey);
		free(pKVP->pKey);
		pKVP = pKVP->pNext;
	}
	free(pMap->pData);
}
