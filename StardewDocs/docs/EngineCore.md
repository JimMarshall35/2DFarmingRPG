# Engine Core

## DynamicArray.c/.h

- A dynamic array similar to a C++ vector
- Create new with 
```c 
struct MyStruct* v = VECTOR_NEW(struct MyStruct)
```
- push with 
    - when capacity is reached the vector will double in size
```c
struct MyStruct s;
v = VectorPush(v, &s);

```
- pop with
```c
v = VectorPop(v);
```
- resize with

```c
v = VectorResize(v);
```
- get the last item with
```c
struct MyStruct* pTop = VectorTop(v);
```
- clear the vector with
    - note this doesn't deallocate any memory, that only happens when the vector is destroyed
```c
v = VectorClear(v);
```

- note this doesn't deallocate any memory, that only happens when the vector is destroyed
```c
DestoryVector(v);
```
- Vectors can be accessed as if they were normal arrays:
```c
struct MyStruct s = v[1];
```
- Size of vector can be gotten with 
```c
int sz = VectorSize(v);
```
- they can be resized at will (this will increase their capcity but not size)
```c
v = VectorResize(v)
```

- NOTE that often the functions will return a pointer which needs to be assigned to the vector variable, in case the vector has resized and the old pointer is invalid - don't forget to make the assignment

## ObjectPool.c/.h

This is an array of a specific type of object that you can request an index into, the index then serves as a handle to an instance of the object the pool holds. When the object is done with and needs to be destroyed its index is freed and the pool will give out the index once more. If there is no room in the pool and a new handle is requested it will resize (doubling in size). Even though the pool has resized, the handle (unlike a ptr would be) is still valid.

The memory for the pool consists of two areas, the pool itself and a list of free indices.

- Initialize a new object pool:
```c
/* returns the new object pool*/
void* InitObjectPool(int objectSize, int poolInitialSize)
```
- Get a handle from the pool:
```c
/* must assign return value to object pool variable as it might have resized */ 
void* GetObjectPoolIndex(void* pObjectPool, int* pOutIndex)
```
- access an instance in the pool by using the handle as an index:
```c
int hMyStruct = GetStructHandle();
struct MyStruct* c = &gObjectPool[hMyStruct];
```
- free a pool allocated object:
```c
void FreeObjectPoolIndex(void* pObjectPool, int indexToFree);
```
- destroy an object pool:
```c
void* FreeObjectPool(void* pObjectPool);
```
- NOTE - just like the vector these often return a value which needs to be assigned back to the object pool

## SharedPtr.h

A simple shared pointer, like the object pool and dynamic array it stores a control struct **behind** the pointer that is returned to the user.

Decrement ref count and it is only freed when the ref count is zero. You can also give it a "destructor" function to call

## Bitfield2D.c/.h

A 2D array of booleans implemented as a bit field. Initialize to a specifc size, get and set bits at specific x and y coordinates.

## Log.c/.h

Logging system, filter by severity (info, warning, error), prints time and thread id, can log to file, console or both. Used throughout engine instead of printf

## StringKeyHashMap.c/.h

A generic hash map with strings for keys. Resizes when a given load factor is reached, uses djb2 hash algorithm + linear probing. Can iterate through keys efficiently.

## BinarySerializer.c/.h

A "class" for loading and saving binary data. It can be created to load or save to a file, to load from an in memory array, or to save to the network. First the binary serializer struct is initialized by calling BS_CreateForLoadFromBuffer or BS_CreateForLoad or BS_CreateForSave or BS_CreateForSaveToNetwork. Next data is serialized by calling BS_SerializeU32 or BS_DeserializeU8 etc. Next BS_Finish is called. At this point if saving the data is saved to disk or enqueued to the network.
Used by game2d layer code.

## DataNode.c/.h

An abstraction for parsing and loading text based data, through it code can load from a lua table or xml file. Used by UI code.

## Thread.c/.h

Cross platform thread abstraction using either win32 or pthread.

## ThreadSaveQueue.c/.h

Fixed size circular thread safe queue, uses cross platform Thread.c/.h abstractions. Callback if queue wraps around and data item dropped.

## SharedLib.c/.h

Cross platform shared library / dll handling, wraps linux/win32 functions to explicitly load function pointers from DLLs, not actually used yet.

## FloatingPointLib.c/.h

Compare floats accounting for rounding errors.

## FileHelpers.c/.h

Function to load a file into a buffer

## TimerPool.c/.h

Intended to be used by game layers to implement timers in a uniform way.

## Random.c/.h

RNG functions

## RawNetMessage.c/.h

Parse and create messages that follow the lowest level of networking protocol, goes along with Network.c

## Network.c/.h

Low level networking, networking thread

## Geometry.c/.h

Basic geometry functions: AABB tests etc.

## ImageFieRegstry.c/.h

This is a relic from the past and serves no purpose - it needs to be completely removed TODO

## Atlas.c/.h

Code to create and use a texture atlas. Load a pre-made atlas or create one from xml. Once an atlas is loaded, code to upload texture to gpu, query sprites inside it by name and fetch UVs for them etc.

## Other...

engine/src/input should be considered part of the engines core as well: this contains code for abstracting input so that it is not tied to a given key or input device and remapping it.

It is my hope that one day engine/src/rendering can become a part of the core engine, and in a way it kind of is.

engine/src/scripting seems like it would form part of the core, but the code in there is really specific to the XML UI code, this file needs splitting into a reusable part and a UI code specific part. The code in it is also specifically LUA scripting.
