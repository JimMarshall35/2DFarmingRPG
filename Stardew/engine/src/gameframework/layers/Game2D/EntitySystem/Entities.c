#include "Entities.h"
#include "AssertLib.h"
#include "BinarySerializer.h"
#include "StaticColliderEntity.h"
#include "Components.h"
#include <string.h>
#include "Entity2DCollection.h"
#include "GameFramework.h"
#include "EntityQuadTree.h"
#include "AnimatedSprite.h"
#include "Sprite.h"
#include "ObjectPool.h"
#include "Network.h"
#include "Log.h"
#include "NetworkID.h"
#include "Game2DLayerNetwork.h"
#include "DynArray.h"
#include "EngineUtils.h"

static VECTOR(struct EntitySerializerPair) pSerializers = NULL;

static OBJECT_POOL(struct DynamicEntityListItem) gDynamicListPool = NULL;


bool DestroyCollectionItr(struct Entity2D* pEnt, int i, void* pUser)
{
    struct GameFrameworkLayer* pLayer = pUser;
    pEnt->onDestroy(pEnt, pLayer);
    return true;
}

void Et2D_DestroyCollection(struct Entity2DCollection* pCollection, struct GameFrameworkLayer* pLayer)
{
    Et2D_IterateEntities(pCollection, &DestroyCollectionItr, pLayer);    
    pCollection->pEntityPool = FreeObjectPool(pCollection->pEntityPool);
}


void Et2D_InitCollection(struct Entity2DCollection* pCollection)
{
    pCollection->gEntityListHead = NULL_HANDLE;
    pCollection->gEntityListTail = NULL_HANDLE;
    pCollection->dynamicEntities.hDynamicListHead = NULL_HANDLE;
    pCollection->dynamicEntities.hDynamicListTail = NULL_HANDLE;
    pCollection->dynamicEntities.nDynamicListSize = 0;
    pCollection->gNumEnts = 0;
    pCollection->pEntityPool = NEW_OBJECT_POOL(struct Entity2D, 512);
    pCollection->dynamicEntities.pDynamicListItemPool = NEW_OBJECT_POOL(struct DynamicEntityListItem, 256);
    pCollection->messageQueue = NEW_VECTOR(struct EntityToEntityMessage);
    pCollection->messageQueue = VectorResize(pCollection->messageQueue, 256);
    pCollection->messageQueue = VectorClear(pCollection->messageQueue);
}

void Entity2DOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    Co_InitComponents(pEnt, pLayer);
    struct GameLayer2DData* pData = pLayer->userData;
    if(pEnt->bKeepInQuadtree)
    {
        pEnt->hQuadTreeRef = Entity2DQuadTree_Insert(&pData->entities, pData->hEntitiesQuadTree, pEnt->thisEntity, pLayer, 0, 6);
    }
    if(pEnt->bKeepInDynamicList)
    {
        pEnt->hDynamicListRef = DynL_AddEntity(&pData->entities.dynamicEntities, pEnt->thisEntity);
    }
}

void Entity2DUpdate(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    Co_UpdateComponents(pEnt, pLayer, deltaT);
}

void Entity2DUpdatePostPhysics(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    Co_Entity2DUpdatePostPhysicsFn(pEnt, pLayer, deltaT);
}

void Entity2DDraw(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex)
{
    Co_DrawComponents(pEnt, pLayer, pCam, outVerts, outIndices, pNextIndex);
}

void Entity2DDrawDebugLines(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(WorldspaceLineVertex)* outVerts)
{

}


void Entity2DInput(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    Co_InputComponents(pEnt, pLayer, context);
}

void Entity2DOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer)
{
    Co_DestroyComponents(pEnt, pLayer);
    struct GameLayer2DData* pData = pLayer->userData;
    
    if(pEnt->bKeepInDynamicList)
    {
        DynL_RemoveItem(&pData->entities.dynamicEntities, pEnt->hDynamicListRef);
    }
    if(pEnt->bKeepInQuadtree)
    {
        Entity2DQuadTree_Remove(pData->hEntitiesQuadTree, pEnt->hQuadTreeRef);
    }
}

void TilesComponentGetBoundingBox(struct TilesComponent* tiles, struct TileMap* pTilemap, vec2 tl, vec2 br)
{
    tl[0] = INFINITY;
    tl[1] = INFINITY;
    br[0] = -INFINITY;
    br[1] = -INFINITY;
    for(int i=0; i<tiles->numTiles; i++)
    {
        struct EntityTile t = tiles->tiles[i];
        struct TileMapLayer* pLayer = &pTilemap->layers[t.layer];
        vec2 tileTL = {
            t.x * pLayer->tileWidthPx,
            t.y * pLayer->tileHeightPx
        };
        vec2 tileBR = {
            tileTL[0] + pLayer->tileWidthPx,
            tileTL[1] + pLayer->tileHeightPx
        };
        if(tileTL[0] < tl[0])
        {
            tl[0] = tileTL[0];
        }
        if(tileTL[1] < tl[1])
        {
            tl[1] = tileTL[1];
        }
        if(tileBR[0] > br[0])
        {
            br[0] = tileBR[0];
        }
        if(tileBR[1] > br[1])
        {
            br[1] = tileBR[1];
        }
    }
}

void Entity2DGetBoundingBox(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR)
{
    vec2 bbtl  = {99999999999, 9999999999999};
    vec2 bbbr  = {-99999999999, -9999999999999};
    bool bSet = false;
    for(int i=0; i < pEnt->numComponents; i++)
    {
        struct Component2D* pComponent = &pEnt->components[i];
        vec2 tl, br;
        if(pComponent->type == ETE_Sprite)
        {
            bSet = true;
            SpriteComp_GetBoundingBox(pEnt, &pComponent->data.sprite, pLayer, tl, br);
        }
        else if(pComponent->type == ETE_SpriteAnimator)
        {
            bSet = true;
            AnimatedSprite_GetBoundingBox(pEnt,&pComponent->data.spriteAnimator, pLayer, tl, br);
        }
        else if(pComponent->type == ETE_Tiles)
        {
            bSet = true;
            struct GameLayer2DData* pData = pLayer->userData;
            TilesComponentGetBoundingBox(&pComponent->data.tiles, &pData->tilemap, tl, br);
        }
        if(tl[0] < bbtl[0])
        {
            bbtl[0] = tl[0];
        }
        if(tl[1] < bbtl[1])
        {
            bbtl[1] = tl[1];
        }
        if(br[0] > bbbr[0])
        {
            bbbr[0] = br[0];
        }
        if(br[1] > bbbr[1])
        {
            bbbr[1] = br[1];
        }
    }
    if(!bSet)
    {
        outTL[0] = 0;
        outTL[1] = 0;
        outBR[0] = 0;
        outBR[1] = 0;
    }
    else
    {
        outTL[0] = bbtl[0];
        outTL[1] = bbtl[1];
        outBR[0] = bbbr[0];
        outBR[1] = bbbr[1];
    }
}

#include "NetworkID.h"
void Et2D_Init(RegisterGameEntitiesFn registerGameEntities)
{
    pSerializers = NEW_VECTOR(struct EntitySerializerPair);
    pSerializers = VectorClear(pSerializers);

    struct EntitySerializerPair scCtorRect = Et2D_Get2DRectStaticColliderSerializerPair();
    Et2D_RegisterEntityType(EBET_StaticColliderRect, &scCtorRect);
    struct EntitySerializerPair scCtorCircle = Et2D_Get2DCircleStaticColliderSerializerPair();
    Et2D_RegisterEntityType(EBET_StaticColliderCircle, &scCtorCircle);


    struct EntitySerializerPair scCtorEllipse = Et2D_Get2DEllipseStaticColliderSerializerPair();
    struct EntitySerializerPair scCtorPoly  = Et2D_Get2DPolygonStaticColliderSerializerPair();

    Et2D_RegisterEntityType(EBET_StaticColliderPoly, &scCtorPoly);
    Et2D_RegisterEntityType(EBET_StaticColliderEllipse, &scCtorEllipse);

    if(registerGameEntities)
    {
        registerGameEntities();
    }
}

void Et2D_DestroyEntity(struct GameFrameworkLayer* pLayer, struct Entity2DCollection* pCollection, HEntity2D hEnt)
{
    struct Entity2D* pEnt = &pCollection->pEntityPool[hEnt];

    if(pCollection->gEntityListHead == hEnt)
    {
        pCollection->gEntityListHead = pEnt->nextSibling;
    }
    if(pCollection->gEntityListTail == hEnt)
    {
        pCollection->gEntityListTail = pEnt->previousSibling;
    }

    if(pEnt->nextSibling != NULL_HANDLE)
    {
        struct Entity2D* pNext = &pCollection->pEntityPool[pEnt->nextSibling];
        pNext->previousSibling = pEnt->previousSibling;

    }
    if(pEnt->previousSibling != NULL_HANDLE)
    {
        struct Entity2D* pPrev = &pCollection->pEntityPool[pEnt->previousSibling];
        pPrev->nextSibling = pEnt->nextSibling;
    }

    pEnt->onDestroy(pEnt, pLayer);
    pCollection->gNumEnts--;
    FreeObjectPoolIndex(pCollection->pEntityPool, hEnt);
}

static HEntity2D Et2D_AddEntityBase(struct Entity2DCollection* pCollection, struct Entity2D* pEnt, bool bAssignNetID)
{
    /* 
        - assign new ID at this point if we're the server.
        - if we're the client it might be serialized or it might be "guessed" by the client depending on context
    */
    if(NW_GetRole() == GR_ClientServer && bAssignNetID)
    {
        pEnt->networkID = NetID_GetID();
    }

    HEntity2D hEnt = NULL_HANDLE;
    pCollection->pEntityPool = GetObjectPoolIndex(pCollection->pEntityPool, &hEnt);
    EASSERT(hEnt != NULL_HANDLE);
    memcpy(&pCollection->pEntityPool[hEnt], pEnt, sizeof(struct Entity2D));
    pEnt = &pCollection->pEntityPool[hEnt];
    pEnt->nextSibling = NULL_HANDLE;
    pEnt->previousSibling = NULL_HANDLE;

    pEnt = &pCollection->pEntityPool[hEnt];
    pEnt->thisEntity = hEnt;
    if(pCollection->gEntityListHead == NULL_HANDLE)
    {
        pCollection->gEntityListHead = hEnt;
        pCollection->gEntityListTail = hEnt;
    }
    else
    {
        struct Entity2D* pLast = &pCollection->pEntityPool[pCollection->gEntityListTail];
        pLast->nextSibling = hEnt;
        pEnt->previousSibling = pCollection->gEntityListTail;
        pEnt->nextSibling = NULL_HANDLE;
        pCollection->gEntityListTail = hEnt;
    }
    pCollection->gNumEnts++;
    return hEnt;
}

HEntity2D Et2D_AddEntityNoNewNetID(struct Entity2DCollection* pCollection, struct Entity2D* pEnt)
{
    return Et2D_AddEntityBase(pCollection, pEnt, false);
}

HEntity2D Et2D_AddEntity(struct Entity2DCollection* pCollection, struct Entity2D* pEnt)
{
    return Et2D_AddEntityBase(pCollection, pEnt, true);
}

void Et2D_DeserializeEntityV1Base(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer, struct Entity2D* pOutEnt)
{
    u32 entityType = 0;
    BS_DeSerializeU32(&entityType, bs);
    Log_Verbose("Deserializing entity type: %i", entityType);
    
    pOutEnt->nextSibling = NULL_HANDLE;
    pOutEnt->previousSibling = NULL_HANDLE;
    pOutEnt->inDrawLayer = objectLayer;
    pOutEnt->type = entityType;

    Et2D_DeserializeCommon(bs, pOutEnt);

    if(pOutEnt->type < VectorSize(pSerializers))
    {
        pSerializers[pOutEnt->type].deserialize(bs, pOutEnt, pData);
    }
    else 
    {
        Log_Error("DESERIALIZE: Entity Serializer type %i out of range\n", pOutEnt->type);
    }
}
#include "LatinMacros.h"

staticus vacuum DeserializeEntityV1(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer)
{
    vimen(bs->ctx)
    {
    casu SCTX_ToFile:
    casu SCTX_ToNetwork:
        {
            struct Entity2D ent;
            memset(&ent, 0, magnitudinem(struct Entity2D));
            Et2D_DeserializeEntityV1Base(pCollection, bs, pData, objectLayer, &ent);
            Et2D_AddEntity(pCollection, &ent);    
        }
        interruptio;
    casu SCTX_ToNetworkUpdate:
        {
            i32 netID = -1;
            BS_DeSerializeI32(&netID, bs);
            NetID_DeserializedNewID(netID);
            struct Entity2D* pEnt = G2D_FindEntityWithNetID(pCollection, netID);
            si(pEnt)
            {
                pSerializers[pEnt->type].deserialize(bs, pEnt, pData);
            }
            aliter
            {
                Log_Error("entity with netID %i not found", netID);
            }
        }
        interruptio;
    }
}

static void LoadEntitiesV1(struct BinarySerializer* bs, struct GameLayer2DData* pData, struct Entity2DCollection* pCollection, int objectLayer)
{
    u32 numEntities = 0;
    BS_DeSerializeU32(&numEntities, bs);
    Log_Verbose("numEntities: %i", numEntities);
    for(int i=0; i<numEntities; i++)
    {
        DeserializeEntityV1(pCollection, bs, pData, objectLayer);
    }
}

static void LoadEntities(struct BinarySerializer* bs, struct GameLayer2DData* pData, struct Entity2DCollection* pCollection, int objectLayer)
{
    EASSERT(!bs->bSaving);
    u32 version = 0;
    BS_DeSerializeU32(&version, bs);
    switch(version)
    {
    case 1:
        LoadEntitiesV1(bs, pData, pCollection, objectLayer);
        break;
    default:
        Log_Error("E2D unsupported version %i\n", version);
        EASSERT(false);
        break;
    }
}

static u32 NumEntsToSerialize(struct Entity2DCollection* pCollection, struct BinarySerializer* bs)
{
    int i = 0;
    HEntity2D hOn = pCollection->gEntityListHead;
    while(hOn != NULL_HANDLE)
    {
        struct Entity2D* pOn = &pCollection->pEntityPool[hOn];
        switch(bs->ctx)
        {
        case SCTX_ToFile:
            if(pOn->bSerializeToDisk)
            {
                i++;
            }
            break;
		case SCTX_ToNetwork:
            if(pOn->bSerializeToNetwork)
            {
                i++;
            }
            break;
        case SCTX_ToNetworkUpdate:
            if(pOn->bSerializeInNetworkUpdate)
            {
                i++;
            }
        }
        
        hOn = pOn->nextSibling;
    }
    return i;
}

void Et2D_SerializeEntityV1Base(struct Entity2D* pOn, struct BinarySerializer* bs, struct GameLayer2DData* pData)
{
    bool bSerialize = false;
    switch(bs->ctx)
    {
    case SCTX_ToFile:
        bSerialize = pOn->bSerializeToDisk;
        break;
    case SCTX_ToNetwork:
        bSerialize = pOn->bSerializeToNetwork;
        break;
    case SCTX_ToNetworkUpdate:
        bSerialize = pOn->bSerializeInNetworkUpdate;
    }
    if(bSerialize)
    {
        if(bs->ctx != SCTX_ToNetworkUpdate)
        {
            BS_SerializeU32(pOn->type, bs);
            Log_Verbose("Entity type: %i", pOn->type);
            Et2D_SerializeCommon(bs, pOn);
        }
        else
        {
            BS_SerializeI32(pOn->networkID, bs);
        }
        if(pOn->type < VectorSize(pSerializers))
        {
            pSerializers[pOn->type].serialize(bs, pOn, pData);
        }
        else 
        {
            Log_Error("Entity Serializer type %i out of range\n", pOn->type);
        }
    }
}

static void SaveEntities(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData)
{
    EASSERT(bs->bSaving);
    BS_SerializeU32(1, bs); /* version number */
    Log_Verbose("Saving %i entities", NumEntsToSerialize(pCollection, bs));
    BS_SerializeU32(NumEntsToSerialize(pCollection, bs), bs);
    HEntity2D hOn = pCollection->gEntityListHead;
    while(hOn != NULL_HANDLE)
    {
        struct Entity2D* pOn = &pCollection->pEntityPool[hOn];
        Et2D_SerializeEntityV1Base(pOn, bs, pData);
        hOn = pOn->nextSibling;
    }
}

/* both serialize and deserialize */
void Et2D_SerializeEntities(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer)
{
    if(bs->bSaving)
    {
        SaveEntities(pCollection, bs, pData);
    }
    else
    {
        LoadEntities(bs, pData, pCollection, objectLayer);
    }
}

void Et2D_RegisterEntityType(u32 typeID, struct EntitySerializerPair* pair)
{
    //pSerializers = (pSerializers, pair);
    int size = VectorSize(pSerializers);
    EASSERT(size == typeID);
    pSerializers = VectorPush(pSerializers, pair);
}

void Et2D_DeserializeCommon(struct BinarySerializer* bs, struct Entity2D* pOutEnt)
{
    if(bs->ctx == SCTX_ToNetworkUpdate)
    {
        return; /* network updates serialize and deserialize a minimal amount of data */
    }

    u32 version = 0;
    BS_DeSerializeI32(&version, bs);
    switch(version)
    {
    case 1:
        BS_DeSerializeFloat(&pOutEnt->transform.position[0], bs);
        BS_DeSerializeFloat(&pOutEnt->transform.position[1], bs);
        BS_DeSerializeFloat(&pOutEnt->transform.scale[0], bs);
        BS_DeSerializeFloat(&pOutEnt->transform.scale[1], bs);
        BS_DeSerializeFloat(&pOutEnt->transform.rotation, bs);
        BS_DeSerializeFloat(&pOutEnt->transform.rotationPointRelative[0], bs);
        BS_DeSerializeFloat(&pOutEnt->transform.rotationPointRelative[1], bs);
        BS_DeSerializeU32(&version, bs);
        pOutEnt->bKeepInQuadtree = version != 0;
        if(bs->ctx == SCTX_ToNetwork)
        {
            BS_DeSerializeI32(&pOutEnt->networkID, bs);
            NetID_DeserializedNewID(pOutEnt->networkID);
        }

        Et2D_PopulateCommonHandlers(pOutEnt);
        break;
    }
}


void Et2D_SerializeCommon(struct BinarySerializer* bs, struct Entity2D* pInEnt)
{
    if(bs->ctx == SCTX_ToNetworkUpdate)
    {
        return; /* network updates serialize and deserialize a minimal amount of data */
    }
    u32 version = 1;
    BS_SerializeI32(version, bs);
    BS_SerializeFloat(pInEnt->transform.position[0], bs);
    BS_SerializeFloat(pInEnt->transform.position[1], bs);
    BS_SerializeFloat(pInEnt->transform.scale[0], bs);
    BS_SerializeFloat(pInEnt->transform.scale[1], bs);
    BS_SerializeFloat(pInEnt->transform.rotation, bs);
    BS_SerializeFloat(pInEnt->transform.rotationPointRelative[0], bs);
    BS_SerializeFloat(pInEnt->transform.rotationPointRelative[1], bs);
    version = (u32)pInEnt->bKeepInQuadtree;
    BS_SerializeU32(version, bs);

    if(bs->ctx == SCTX_ToNetwork)
    {
        BS_SerializeI32(pInEnt->networkID, bs);
    }
}

struct Entity2D* Et2D_GetEntity(struct Entity2DCollection* pCollection, HEntity2D hEnt)
{
    return &pCollection->pEntityPool[hEnt];
}

void Et2D_IterateEntities(struct Entity2DCollection* pCollection, Entity2DIterator itr, void* pUser)
{
    HEntity2D hOnEnt = pCollection->gEntityListHead;
    int i = 0;
    while(hOnEnt != NULL_HANDLE)
    {
        struct Entity2D* pEntity = Et2D_GetEntity(pCollection, hOnEnt);
        if(!itr(pEntity, i++, pUser))
            break;
        pEntity = Et2D_GetEntity(pCollection, hOnEnt);
        hOnEnt = pEntity->nextSibling;
    }
}

float Entity2DGetSortVal(struct Entity2D* pEnt)
{
    return pEnt->transform.position[1];
}

void Et2D_PopulateCommonHandlers(struct Entity2D* pEnt)
{
    pEnt->init = &Entity2DOnInit;
    pEnt->update = &Entity2DUpdate;
    pEnt->postPhys = &Entity2DUpdatePostPhysics;
    pEnt->draw = &Entity2DDraw;
    pEnt->drawDebugLines = &Entity2DDrawDebugLines;
    pEnt->input = &Entity2DInput;
    pEnt->onDestroy = &Entity2DOnDestroy;
    pEnt->getBB = &Entity2DGetBoundingBox;
    pEnt->getSortPos = &Entity2DGetSortVal;
}

bool PrintCollectionItr(struct Entity2D* pEnt, int i, void* pUser)
{
    int* pCount = pUser;
    Log_Info("ENTITY ID: %i NET ID: %i TYPE: %i", pEnt->thisEntity, pEnt->networkID, pEnt->type);
    if(pEnt->printEntityInfo)
    {
        pEnt->printEntityInfo(pEnt);
    }
    (*pCount)++;
    return true;
}

void Et2D_PrintEntitiesInfo(struct Entity2DCollection* pCollection)
{
    int count = 0;
    Et2D_IterateEntities(pCollection, &PrintCollectionItr, &count);
    Log_Info("Num entities: %i", count);
    EASSERT(count == pCollection->gNumEnts);
}

void Et2D_DoEntityMessagesQueue(struct Entity2DCollection* pCollection, struct GameFrameworkLayer* pLayer)
{
    for(int i = 0; i < VectorSize(pCollection->messageQueue); i++)
    {
        struct EntityToEntityMessage* pMsg = &pCollection->messageQueue[i];
        struct Entity2D* pRecipient = &pCollection->pEntityPool[pMsg->recipient];
        if(pRecipient->handleEntityMsg)
        {
            pRecipient->handleEntityMsg(
                pRecipient, 
                &pCollection->pEntityPool[pMsg->sender],
                pMsg,
                pLayer
            );
        }
        if(pMsg->freer)
        {
            pMsg->freer(pMsg);
        }
    }
    
    pCollection->messageQueue = VectorClear(pCollection->messageQueue);
}

void Et2D_SendEntity2EntityMsg(struct Entity2DCollection* pCollection, struct EntityToEntityMessage* pMsg)
{
    pCollection->messageQueue = VectorPush(pCollection->messageQueue, pMsg);
}
