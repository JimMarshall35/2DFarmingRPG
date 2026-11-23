#ifndef STRING_KEY_HASH_MAP_H
#define STRING_KEY_HASH_MAP_H

#ifdef __cplusplus
extern "C" {
#endif 


#include <stdbool.h>

/// @brief A hash map with strings for keys and any type for values
/// doubles in size when the load factor is met, keeps KVPs as a linked list
struct HashMap
{
	int capacity;
	int size;
	int valueSize;
	void* pData;
	struct KVP* pHead;
	struct KVP* pEnd;
	float fLoadFactor;
};

/// @brief initializes the hashmap struct to a decfault value. you can change fLoadFactor afterwards but it sets it to a default value
/// @param pMap 
/// @param capacity 
/// @param valSize size of individual values
void HashmapInit(struct HashMap* pMap, int capacity, int valSize);

/// @brief same as HashmapInit but sets load factor as well
/// @param pMap 
/// @param capacity 
/// @param valSize 
/// @param loadFactor size of individual values
void HashmapInitWithLoadFactor(struct HashMap* pMap, int capacity, int valSize, float loadFactor);

/// @brief Search for a key in the hash map, returns a pointer to the value or NULL if not present
/// @param pMap 
/// @param key 
/// @return 
void* HashmapSearch(struct HashMap* pMap, char* key);


/// @brief 
/// @param pMap 
/// @param key 
/// @param pVal 
/// @return true if a new key and inserted, false if an existing key and value overwritten
void* HashmapInsert(struct HashMap* pMap, char* key, void* pVal);

/// @brief 
/// @param pMap 
/// @param key 
/// @return 
bool HashmapDeleteItem(struct HashMap* pMap, char* key);

/// @brief 
/// @param pMap 
void HashmapDeInit(struct HashMap* pMap);

/// @brief 
/// @param pMap 
/// @param hashMapName 
void HashmapPrintEntries(struct HashMap* pMap, const char* hashMapName);

struct HashmapKeyIterator
{
	struct HashMap* pHashMap;
	struct KVP* pOnKVP;
};

/// @brief Get an iterator object that you can call "NextHashmapKey" with to iterate through the hashmaps keys
struct HashmapKeyIterator GetKeyIterator(struct HashMap* pHashMap);

/// @brief 
/// @param  
/// @return 
char* NextHashmapKey(struct HashmapKeyIterator*);

#ifdef __cplusplus
}
#endif

#endif