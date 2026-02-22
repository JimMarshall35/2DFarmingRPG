#include <string.h>
#include "WfTree.h"
#include "BinarySerializer.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "Components.h"
#include "Atlas.h"
#include "WfGameLayerData.h"
#include "WfEntities.h"
#include "WfEntityMessages.h"
#include "Log.h"
#include "AssertLib.h"

struct WfTreeEntityData
{
    struct WfTreeDef def;
    vec2 groundContactPoint;
    float health;
};

static OBJECT_POOL(struct WfTreeEntityData) gTreeDataObjectPool;

void WfTreeInit()
{
    gTreeDataObjectPool = NEW_OBJECT_POOL(struct WfTreeEntityData, 512);
}

static void TreeOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gTreeDataObjectPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static float TreeGetPreDrawSortValue(struct Entity2D* pEnt)
{
    struct WfTreeEntityData* pData = &gTreeDataObjectPool[pEnt->user.hData];
    return pData->groundContactPoint[1];
}

static void TreeHandleEntityMsg(struct Entity2D* pEnt, struct Entity2D* pSender, struct EntityToEntityMessage* pMsg, struct GameFrameworkLayer* pLayer)
{
    struct WfTreeEntityData* pData = &gTreeDataObjectPool[pEnt->user.hData];
    switch(pMsg->type)
    {
    case E2EM_Damage:
        {
            struct WfDamageMsg* pDamageMessage = WfGetDamageMessage(pMsg);//pMsg->pMsgData;
            switch (pDamageMessage->type)
            {
            case WfAxeDamage:
                pData->health -= pDamageMessage->damage;
                if(pData->health < 0)
                {
                    pData->health = 0;
                }
                Log_Info("Tree with ID %i took %.2f %s damage from entity %i, %.2f health remaining", pEnt->thisEntity, pDamageMessage->damage, WfDamageTypeNameLUT[pDamageMessage->type], pSender->thisEntity, pData->health);
                break;
            
            default:
                EASSERT(pDamageMessage->type < sizeof(WfDamageTypeNameLUT) / sizeof(char*) && pDamageMessage->type >= 0);
                Log_Info("Tree with ID %i resisted %.2f %s damage from entity %i, immune", pEnt->thisEntity, pDamageMessage->damage, WfDamageTypeNameLUT[pDamageMessage->type], pSender->thisEntity);
                break;
            }
        }
    }
}

static void WfMakeEntityIntoTreeBasedAt(struct Entity2D* pEnt, float x, float y, struct WfTreeDef* def, struct GameLayer2DData* pGameLayerData)
{
    struct WfSprites* pSprites = &((struct WfGameLayerData*)pGameLayerData->pUserData)->sprites;

    memset(pEnt, 0, sizeof(struct Entity2D));
    const float trunkOffsetPx = 64.0f; /* Y offset from the top of the tree top sprite to the top of the trunk sprite */
    const float spriteHeight = 96.0f;
    const float combinedTreeSpriteHeight = trunkOffsetPx + spriteHeight;
    const float combinedSpriteWidth = 96.0f;
    const float bottomOfTrunkSpriteToBase = 34.0f;

    // xPos and YPos are where the base of the tree is
    pEnt->transform.position[0] = x - combinedSpriteWidth / 2.0f; // center it
    pEnt->transform.position[1] = y - (combinedTreeSpriteHeight - bottomOfTrunkSpriteToBase);
    pEnt->transform.scale[0] = 1.0f;
    pEnt->transform.scale[1] = 1.0f;
    pEnt->transform.rotation = 0.0f;
    pEnt->transform.rotationPointRelative[0] = 0.0f;
    pEnt->transform.rotationPointRelative[1] = 0.0f;
    
    pEnt->bKeepInQuadtree = true;
    pEnt->bKeepInDynamicList = false;
    pEnt->type = WfEntityType_Tree;

    struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
    struct Component2D* pComponent2 = &pEnt->components[pEnt->numComponents++];
    struct Component2D* pComponent3 = &pEnt->components[pEnt->numComponents++];

    struct WfTreeSprites* pFoundSeason = &pSprites->treeSpritesPerSeason[def->season];
    hSprite topSprite = NULL_HANDLE;
    hSprite trunkSprite = NULL_HANDLE;
    switch (def->type)
    {
    case Coniferous:
        topSprite = def->subtype == 0 ? pFoundSeason->coniferousTop1 : pFoundSeason->coniferousTop2;
        trunkSprite = pFoundSeason->trunk2;
        break;
    case Deciduous:
        topSprite = def->subtype == 0 ? pFoundSeason->deciduousTop1 : pFoundSeason->deciduousTop2;
        trunkSprite = pFoundSeason->trunk1;
        break;
    default:
        break;
    }

    /* order is important as we want the tree trunk to be drawn first and the top on top of that */
    pComponent2->type = ETE_Sprite;
    pComponent2->data.sprite.sprite = topSprite;
    memset(&pComponent2->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent2->data.sprite.transform.scale[0] = 1.0f;
    pComponent2->data.sprite.transform.scale[1] = 1.0f;
    pComponent2->data.sprite.bDraw = true;

    pComponent1->type = ETE_Sprite;
    pComponent1->data.sprite.sprite = trunkSprite;
    memset(&pComponent1->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent1->data.sprite.transform.position[1] = trunkOffsetPx;
    pComponent1->data.sprite.transform.scale[0] = 1.0f;
    pComponent1->data.sprite.transform.scale[1] = 1.0f;
    pComponent1->data.sprite.bDraw = true;

    vec2 transform2Ground = {
        combinedSpriteWidth / 2.0f,
        (combinedTreeSpriteHeight - bottomOfTrunkSpriteToBase)
    };

    pComponent3->type = ETE_StaticCollider;
    pComponent3->data.staticCollider.shape.type = PBT_Circle;
    pComponent3->data.staticCollider.shape.data.circle.center[0] = x;//transform2Ground[0];
    pComponent3->data.staticCollider.shape.data.circle.center[1] = y;//transform2Ground[1];
    pComponent3->data.staticCollider.shape.data.circle.radius = 6;
    pComponent3->data.staticCollider.bIsSensor = false;
    pComponent3->data.staticCollider.onSensorOverlapBegin = NULL;
    pComponent3->data.staticCollider.onSensorOverlapEnd = NULL;
    pComponent3->data.staticCollider.bGenerateSensorEvents = false;
    pComponent3->data.sprite.bDraw = true;

    HGeneric hTreeData = NULL_HANDLE;
    gTreeDataObjectPool = GetObjectPoolIndex(gTreeDataObjectPool, &hTreeData);
    gTreeDataObjectPool[hTreeData].def = *def;
    gTreeDataObjectPool[hTreeData].groundContactPoint[0] = x;
    gTreeDataObjectPool[hTreeData].groundContactPoint[1] = y;
    gTreeDataObjectPool[hTreeData].health = 100.0f;
    pEnt->user.hData = hTreeData;
    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->onDestroy = &TreeOnDestroy;
    pEnt->getSortPos = &TreeGetPreDrawSortValue;
    pEnt->handleEntityMsg = &TreeHandleEntityMsg;
    pEnt->bSerializeToDisk = true;
    pEnt->bSerializeToNetwork = true;
}


void WfDeSerializeTreeEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version = 0;
    BS_DeSerializeU32(&version, bs); // version
    switch (version)
    {
    case 1:
        /* code */
        {
            struct WfTreeEntityData entData;
            BS_DeSerializeI32((i32*)&entData.def.season, bs);
            BS_DeSerializeI32((i32*)&entData.def.type, bs);
            BS_DeSerializeI32((i32*)&entData.def.subtype, bs);
            BS_DeSerializeFloat(&entData.groundContactPoint[0], bs);
            BS_DeSerializeFloat(&entData.groundContactPoint[1], bs);
            
            WfMakeEntityIntoTreeBasedAt(pOutEnt, entData.groundContactPoint[0], entData.groundContactPoint[1], &entData.def, pData);
        }
        break;
    
    default:
        break;
    }
}

void WfSerializeTreeEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    struct WfTreeEntityData* pEntData = &gTreeDataObjectPool[pInEnt->user.hData];
    BS_SerializeU32(1, bs); // version
    BS_SerializeI32((i32)pEntData->def.season, bs);
    BS_SerializeI32((i32)pEntData->def.type, bs);
    BS_SerializeI32((i32)pEntData->def.subtype, bs);
    BS_SerializeFloat(pEntData->groundContactPoint[0], bs);
    BS_SerializeFloat(pEntData->groundContactPoint[1], bs);
}

HEntity2D WfAddTreeBasedAt(float x, float y, struct WfTreeDef* def, struct GameLayer2DData* pGameLayerData)
{
    struct Entity2D ent;
    WfMakeEntityIntoTreeBasedAt(&ent, x, y, def, pGameLayerData);
    return Et2D_AddEntity(&pGameLayerData->entities, &ent);
}

void WfTreeGetGroundContactPoint(struct Entity2D* pTreeEnt, vec2 outPos)
{
    EASSERT(pTreeEnt->type == WfEntityType_Tree);
    struct WfTreeEntityData* pTreeData = &gTreeDataObjectPool[pTreeEnt->user.hData];
    outPos[0] = pTreeData->groundContactPoint[0];
    outPos[1] = pTreeData->groundContactPoint[1];
}

