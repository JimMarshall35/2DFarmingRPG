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
#include "ObjectPool.h"

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

void Entity2DInput(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    Co_InputComponents(pEnt, pLayer, context);
}

void Entity2DOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer)
{
    Co_DestroyComponents(pEnt);
    struct GameLayer2DData* pData = pLayer->userData;
    if(pEnt->bKeepInDynamicList)
    {
        DynL_RemoveItem(&pData->entities.dynamicEntities, pEnt->hDynamicListRef);
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
        pCollection->gEntityListTail = pEnt->nextSibling;
    }
    if(pCollection->gEntityListTail == hEnt)
    {
        pCollection->gEntityListHead = pEnt->previousSibling;
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
    FreeObjectPool(pCollection->dynamicEntities.pDynamicListItemPool);
}

HEntity2D Et2D_AddEntity(struct Entity2DCollection* pCollection, struct Entity2D* pEnt)
{
    HEntity2D hEnt = NULL_HANDLE;
    pEnt->nextSibling = NULL_HANDLE;
    pEnt->previousSibling = NULL_HANDLE;
    pCollection->pEntityPool = GetObjectPoolIndex(pCollection->pEntityPool, &hEnt);
    EASSERT(hEnt != NULL_HANDLE);
    memcpy(&pCollection->pEntityPool[hEnt], pEnt, sizeof(struct Entity2D));
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

static void DeserializeEntityV1(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer)
{
    u32 entityType;
    BS_DeSerializeU32(&entityType, bs);
    struct Entity2D ent;
    memset(&ent, 0, sizeof(struct Entity2D));
    ent.nextSibling = NULL_HANDLE;
    ent.previousSibling = NULL_HANDLE;
    ent.inDrawLayer = objectLayer;
    ent.type = entityType;

    Et2D_DeserializeCommon(bs, &ent);

    if(ent.type < VectorSize(pSerializers))
    {
        pSerializers[ent.type].deserialize(bs, &ent, pData);
    }
    else 
    {
        printf("DESERIALIZE: Entity Serializer type %i out of range\n", ent.type);
    }
    Et2D_AddEntity(pCollection, &ent);
}

static void LoadEntitiesV1(struct BinarySerializer* bs, struct GameLayer2DData* pData, struct Entity2DCollection* pCollection, int objectLayer)
{
    u32 numEntities = 0;
    BS_DeSerializeU32(&numEntities, bs);
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
        printf("E2D unsupported version %i\n", version);
        EASSERT(false);
        break;
    }
}

static u32 NumEntsToSerialize(struct Entity2DCollection* pCollection)
{
    int i = 0;
    HEntity2D hOn = pCollection->gEntityListHead;
    while(hOn != NULL_HANDLE)
    {
        struct Entity2D* pOn = &pCollection->pEntityPool[hOn];
        if(pOn->bSerialize)
        {
            i++;
        }
        hOn = pOn->nextSibling;
    }
    return i;
}

static void SaveEntities(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData)
{
    EASSERT(bs->bSaving);
    BS_SerializeU32(NumEntsToSerialize(pCollection), bs);
    HEntity2D hOn = pCollection->gEntityListHead;
    while(hOn != NULL_HANDLE)
    {
        struct Entity2D* pOn = &pCollection->pEntityPool[hOn];
        if(pOn->bSerialize)
        {
            BS_SerializeU32(pOn->type, bs);
            Et2D_SerializeCommon(bs, pOn);
            if(pOn->type < VectorSize(pSerializers))
            {
                pSerializers[pOn->type].serialize(bs, pOn, pData);
            }
            else 
            {
                printf("Entity Serializer type %i out of range\n", pOn->type);
            }
        }
        hOn = pOn->nextSibling;
    }
}

/* both serialize and deserialize */
void Et2D_SerializeEntities(struct Entity2DCollection* pCollection, struct BinarySerializer* bs, struct GameLayer2DData* pData, int objectLayer)
{
    if(bs->bSaving)
    {
        SaveEntities(bs, pData, pCollection);
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
        
        BS_DeSerializeU32(&version, bs);
        pOutEnt->bKeepInQuadtree = version != 0;
        Et2D_PopulateCommonHandlers(pOutEnt);
        break;
    }
}


void Et2D_SerializeCommon(struct BinarySerializer* bs, struct Entity2D* pInEnt)
{
    u32 version = 1;
    BS_SerializeI32(version, bs);
    BS_SerializeFloat(pInEnt->transform.position[0], bs);
    BS_SerializeFloat(pInEnt->transform.position[1], bs);
    BS_SerializeFloat(pInEnt->transform.scale[0], bs);
    BS_SerializeFloat(pInEnt->transform.scale[1], bs);
    BS_SerializeFloat(pInEnt->transform.rotation, bs);
    version = (u32)pInEnt->bKeepInQuadtree;
    BS_SerializeU32(version, bs);
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
        hOnEnt = pEntity->nextSibling;
    }
    volatile int e = 0;
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
    pEnt->input = &Entity2DInput;
    pEnt->onDestroy = &Entity2DOnDestroy;
    pEnt->getBB = &Entity2DGetBoundingBox;
    pEnt->getSortPos = &Entity2DGetSortVal;
}
