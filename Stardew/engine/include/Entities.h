#ifndef ENTITIES_H
#define ENTITIES_H

#include "Game2DLayer.h"
#include "DynArray.h"
#include "DrawContext.h"
#include "InputContext.h"
#include "Physics2D.h"
#include <box2d/box2d.h>
#include <stdbool.h>

struct Entity2D;
struct Entity2DCollection;
struct GameFrameworkLayer;
struct EntityToEntityMessage;

typedef void (*Entity2DOnInitFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx);
typedef void (*Entity2DUpdateFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
typedef void (*Entity2DUpdatePostPhysicsFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
typedef void (*Entity2DDrawFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex);
typedef void (*Entity2DDrawDebugLinesFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(WorldspaceLineVertex)* outVerts);

typedef void (*Entity2DInputFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context);
typedef void (*Entity2DOnDestroyFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer);
typedef void (*Entity2DGetBoundingBoxFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR);

/** @brief lower values drawn first */
typedef float (*Entity2DGetPreDrawSortValueFn)(struct Entity2D* pEnt);


typedef void(*EntityDeserializeFn)(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);
typedef void(*EntitySerializeFn)(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);


typedef void(*RegisterGameEntitiesFn)(void);

typedef void(*OnSensorShapeOverlapBeginFn)(struct GameFrameworkLayer* pLayer, HEntity2D hOverlappingEntity, HEntity2D thisSensorEntity);
typedef void(*OnSensorShapeOverlapEndFn)(struct GameFrameworkLayer* pLayer, HEntity2D hOverlappingEntity, HEntity2D thisSensorEntity);

typedef void(*PrintEntityInfoExtenderFn)(struct Entity2D* pInEnt);

typedef void(*HandleEntityMessagefn)(struct Entity2D* pEnt, struct Entity2D* pSender, struct EntityToEntityMessage* pMsg, struct GameFrameworkLayer* pLayer);

struct EntitySerializerPair
{
    EntityDeserializeFn deserialize;
    EntitySerializeFn serialize;
};

typedef i32 EntityType;

#define MAX_COMPONENTS 8

/** @brief types built into the engine */
enum ComponentType
{
    ETE_Sprite,
    ETE_StaticCollider,
    ETE_DynamicCollider,
    ETE_TextSprite,
    ETE_SpriteAnimator,
    ETE_Tiles, /* represents a collection of tiles in the tilemap */
    ETE_Last
};

struct Sprite
{
    hSprite sprite;
    struct Transform2D transform;
    bool bDraw;
};

struct StaticCollider
{
    H2DBody id;
    struct PhysicsShape2D shape;
    bool bIsSensor;
    /** @brief If this is a non-sensor, does it generate sensor overlap events? Best for performance to only enable this if necessary */
    bool bGenerateSensorEvents; 
    OnSensorShapeOverlapBeginFn onSensorOverlapBegin;
    OnSensorShapeOverlapEndFn onSensorOverlapEnd;
};

struct DynamicCollider
{
    H2DBody id;
    struct PhysicsShape2D shape;
    struct KinematicBodyOptions options;
    bool bIsSensor;
    /** @brief If this is a non-sensor, does it generate sensor overlap events? Best for performance to only enable this if necessary */
    bool bGenerateSensorEvents;
    OnSensorShapeOverlapBeginFn onSensorOverlapBegin;
    OnSensorShapeOverlapEndFn onSensorOverlapEnd;
};

struct TextSprite
{
    char* content;
    HFont font;
    float fSizePts;
    hAtlas atlas;
    float r, g, b, a;
    bool bDraw;
};

struct AnimatedSprite
{
    const char* animationName;
    float timer;
    hSprite* pSprites;
    int numSprites;
    int onSprite;
    float fps;
    bool bRepeat;
    bool bIsAnimating;
    bool bDraw;
    struct Transform2D transform;
};

#define TilesComponentMaxTiles 4
struct TilesComponent
{
    struct EntityTile
    {
        int x, y, layer;
        TileIndex tile;
    }tiles[TilesComponentMaxTiles];
    int numTiles;
};

/// @brief An entity component tagged union - there are a fixed number built into the engine
struct Component2D
{
    enum ComponentType type;
    union
    {
        struct Sprite sprite;

        struct StaticCollider staticCollider;

        struct DynamicCollider dynamicCollider;

        struct TextSprite textSprite;
        
        struct AnimatedSprite spriteAnimator;

        struct TilesComponent tiles;
    }data;
    
};

enum EngineBuiltinEntityType
{
    EBET_StaticColliderRect,
    EBET_StaticColliderCircle,
    EBET_StaticColliderPoly,
    EBET_StaticColliderEllipse,
    EBET_Last
};

struct BinarySerializer;

/// @brief Games using the engine should call this to add types of entity to be serialized
/// @param typeID Entity type
/// @param pair pair of functions to serialize and deserialize
void Et2D_RegisterEntityType(u32 typeID, struct EntitySerializerPair* pair);

void Et2D_Init(RegisterGameEntitiesFn registerGameEntities);

/// @brief Add, but don't initialize, the entity. If you're the server, a new net ID will be assigned
/// @param pCollection - collection to add to
/// @param pEnt - Entity to add
/// @param bOwned - Does this host own the entity or is it owned by another player on the network, no effect if single player
/// @return Entity handle
HEntity2D Et2D_AddEntity(struct Entity2DCollection* pCollection, struct Entity2D* pEnt);

/// @brief Add, but don't initialize, the entity. If you're the server, a new net ID won't be assigned
/// @param pCollection - collection to add to
/// @param pEnt - Entity to add
/// @param bOwned - Does this host own the entity or is it owned by another player on the network, no effect if single player
/// @return Entity handle
HEntity2D Et2D_AddEntityNoNewNetID(struct Entity2DCollection* pCollection, struct Entity2D* pEnt);

/// @brief 
/// @param pLayer 
/// @param pCollection 
/// @param hEnt 
void Et2D_DestroyEntity(struct GameFrameworkLayer* pLayer, struct Entity2DCollection* pCollection, HEntity2D hEnt);


/// @brief 
/// @param pCollection 
/// @param hEnt 
/// @return - entity handle
struct Entity2D* Et2D_GetEntity(struct Entity2DCollection* pCollection, HEntity2D hEnt);


/// @brief iterator function for Et2D_IterateEntities. Return true to continue iterating or false to break. pUser is value passed as pUser to Et2D_IterateEntities
typedef bool(*Entity2DIterator)(struct Entity2D* pEnt, int i, void* pUser);

/// @brief A convenient helper to iterate through the entity collection
/// @param pCollection collection to iterate
/// @param itr iterator function ptr
/// @param pUser user data to pass to iterator fn
void Et2D_IterateEntities(struct Entity2DCollection* pCollection, Entity2DIterator itr, void* pUser);

void Et2D_SerializeEntities(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer);

void Et2D_DeserializeCommon(struct BinarySerializer* bs, struct Entity2D* pOutEnt);
void Et2D_SerializeCommon(struct BinarySerializer* bs, struct Entity2D* pInEnt);

void Et2D_InitCollection(struct Entity2DCollection* pCollection);
void Et2D_DestroyCollection(struct Entity2DCollection* pCollection, struct GameFrameworkLayer* pLayer);

void Et2D_PrintEntitiesInfo(struct Entity2DCollection* pCollection);

struct Entity2D
{
    /* handler functions */
    Entity2DOnInitFn init;
    Entity2DUpdateFn update;
    Entity2DUpdatePostPhysicsFn postPhys;
    Entity2DDrawFn draw;
    Entity2DDrawDebugLinesFn drawDebugLines;
    Entity2DInputFn input;
    Entity2DOnDestroyFn onDestroy;
    Entity2DGetBoundingBoxFn getBB;
    Entity2DGetPreDrawSortValueFn getSortPos;
    PrintEntityInfoExtenderFn printEntityInfo;
    HandleEntityMessagefn handleEntityMsg;

    struct Transform2D transform;
    EntityType type;
    
    union
    {
        /** @brief data */
        void* pData;
        HGeneric hData;
    }user;
    
    
    /** @brief just for convenience - this entities handle */
    HEntity2D thisEntity;

    HEntity2D nextSibling;
    HEntity2D previousSibling;

    int numComponents;
    struct Component2D components[MAX_COMPONENTS];

    bool bKeepInQuadtree;

    /** @brief  reference to the entity in the quad tree needed to remove it */
    HEntity2DQuadtreeEntityRef hQuadTreeRef;

    /** @brief keep in the dynamic list of brute force culled entities */
    bool bKeepInDynamicList;

    /** @brief reference to the entity in the dynamic entities list needed to remove it */
    HDynamicEntityListItem hDynamicListRef;

    /** @brief 
        Which object layer of the scene is it in? 
        Effects the order they are drawn in
    */
    int inDrawLayer;

    /** @brief do we serialize when the level is serialized? */
    bool bSerializeToDisk;

    /** @brief do we serialize when the initial level data is sent to clients */
    bool bSerializeToNetwork;

    /** @brief do we serialize when the next network update is sent */
    bool bSerializeInNetworkUpdate;

    int networkID;
};

/** @brief
    Default base implementations, override with own behavior and then call these at the end
*/
void Entity2DOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx);
void Entity2DUpdate(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void Entity2DUpdatePostPhysics(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void Entity2DDraw(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex);
void Entity2DInput(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context);
void Entity2DOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer);
void Entity2DGetBoundingBox(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR);
float Entity2DGetSortVal(struct Entity2D* pEnt);

void Et2D_PopulateCommonHandlers(struct Entity2D* pEnt);

/// @brief Deserialize an entity from the binary serializer using the version 1 method
/// @param pCollection
/// @param bs 
/// @param pData game layer data
/// @param objectLayer object layer to set the entity to be in
/// @param pOutEnt entity to populate
void Et2D_DeserializeEntityV1Base(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer, struct Entity2D* pOutEnt);

/// @brief Serialize a single entity
/// @param pOn 
/// @param bs 
/// @param pData 
void Et2D_SerializeEntityV1Base(struct Entity2D* pOn, struct BinarySerializer* bs, struct GameLayer2DData* pData);

/// @brief Do the entity messages queue, iterate through the messages, call entities callbacks, and clear the queue
/// @param pCollection 
/// @param pLayer 
void Et2D_DoEntityMessagesQueue(struct Entity2DCollection* pCollection, struct GameFrameworkLayer* pLayer);

/// @brief 
/// @param pCollection 
/// @param pMsg 
void Et2D_SendEntity2EntityMsg(struct Entity2DCollection* pCollection, struct EntityToEntityMessage* pMsg);

#endif
