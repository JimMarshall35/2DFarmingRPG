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
#include "GameFramework.h"
#include "Maths.h"
#include "ZzFX.h"

#define STUMP_SPITE_INDEX  0
#define TRUNK_SPRITE_INDEX 1
#define TOP_SPRITE_INDEX   2

#define TREE_FALL_SPEED 60.0f

// increase speed by this many rpm every second
#define TREE_FALL_CHANGE_SPEED_PER_SECOND 90.0f

#define STUMP_HEALTH 30.0f;

#define STUMP_HEIGHT (50.0f) // the height of just the stump part of the tree sprite to the bottom of the sprite

struct ZZFXSound gTreeHitSFX = {1.0,0.05,63.14,0.049,0.086,0.203,2,1.0,3.678,-4.693,0.0,0.0,0.261,0.185,0.0,0.699,0.0,0.388,0.108,0.087,0.0};
struct ZZFXSound gTreeFallSFX = {1.0,0.05,590.6,0.005,0.28,0.257,1,1.0,0.0,0.0,-196.86,0.159,0.165,0.12,15.624050000000004,0.168,0.0,0.635,0.227,0.421,0.0};

enum WfTreeState
{
    WfStanding,
    WfFalling,
    WfFallen,
    WfStump,
};

struct WfTreeEntityData
{
    struct WfTreeDef def;
    vec2 groundContactPoint;
    float health;
    enum WfTreeState state;
    float treeFallDirection;
    float treeFallRate;
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
    struct GameLayer2DData* pGameLayerData = pLayer->userData;
    struct WfTreeEntityData* pData = &gTreeDataObjectPool[pEnt->user.hData];
    switch(pMsg->type)
    {
    case E2EM_Damage:
        {
            struct WfDamageMsg* pDamageMessage = WfGetDamageMessage(pMsg);//pMsg->pMsgData;
            switch (pDamageMessage->type)
            {
            case WfAxeDamage:
                Au_PlayZzFX(&gTreeHitSFX);
                pData->health -= pDamageMessage->damage;
                if(pData->health <= 0)
                {
                    switch(pData->state)
                    {
                    case WfStanding:
                        {
                            if(pSender->type == WfEntityType_Player)
                            {
                                vec2 playerGroundPos, treeGroundContactPoint;
                                WfPlayerGetGroundContactPoint(pSender, playerGroundPos);
                                WfTreeGetGroundContactPoint(pEnt, treeGroundContactPoint);
                                if(playerGroundPos[0] > treeGroundContactPoint[0])
                                {
                                    // player to the right of the tree
                                    pData->treeFallDirection = -1.0f;
                                }
                                else
                                {
                                    pData->treeFallDirection = 1.0f;
                                }
                            }
                            else
                            {
                                pData->treeFallDirection = 1.0f;
                            }
                            pData->health = 0;
                    
                                Au_PlayZzFX(&gTreeFallSFX);
                                pData->state = WfFalling;
                        }
                        break;
                    case WfStump:
                        {
                            Et2D_DestroyEntity(pLayer, &pGameLayerData->entities, pEnt->thisEntity);
                        }
                        break;
                    }
                    
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

void TreeUpdate(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct GameLayer2DData* pGameLayerData = pLayer->userData;
    struct WfTreeEntityData* pData = &gTreeDataObjectPool[pEnt->user.hData];
    switch (pData->state)
    {
    case WfStanding:
        break;
    case WfFalling:
        {
            float treeFallRPS = pData->treeFallRate / 60.0f;
            pEnt->components[TOP_SPRITE_INDEX].data.sprite.transform.rotation += (treeFallRPS * deltaT) * pData->treeFallDirection;
            pEnt->components[TRUNK_SPRITE_INDEX].data.sprite.transform.rotation += (treeFallRPS * deltaT) * pData->treeFallDirection;
            bool bFinished = false;
            if(pData->treeFallDirection <= 0.0f)
            {
                bFinished = pEnt->components[TOP_SPRITE_INDEX].data.sprite.transform.rotation < -DEGREES_TO_RADIANS(90.0f);
            }
            else
            {
                bFinished = pEnt->components[TOP_SPRITE_INDEX].data.sprite.transform.rotation > DEGREES_TO_RADIANS(90.0f);
            }
            if(bFinished)
            {
                pEnt->components[TOP_SPRITE_INDEX].data.sprite.bDraw = false;
                pEnt->components[TRUNK_SPRITE_INDEX].data.sprite.bDraw = false;
                pData->state = WfStump;
                pData->health = STUMP_HEALTH;
            }
            pData->treeFallRate += TREE_FALL_CHANGE_SPEED_PER_SECOND * deltaT;
        }
        break;
    case WfStump:
        break;
    default:
        break;
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
    struct Component2D* pComponent4 = &pEnt->components[pEnt->numComponents++];

    struct WfTreeSprites* pFoundSeason = &pSprites->treeSpritesPerSeason[def->season];
    hSprite topSprite = NULL_HANDLE;
    hSprite trunkSprite = NULL_HANDLE;
    hSprite stumpSprite = NULL_HANDLE;

    switch (def->type)
    {
    case Coniferous:
        topSprite = def->subtype == 0 ? pFoundSeason->coniferousTop1 : pFoundSeason->coniferousTop2;
        trunkSprite = pFoundSeason->trunk2;
        stumpSprite = pFoundSeason->stump2;
        break;
    case Deciduous:
        topSprite = def->subtype == 0 ? pFoundSeason->deciduousTop1 : pFoundSeason->deciduousTop2;
        trunkSprite = pFoundSeason->trunk1;
        stumpSprite = pFoundSeason->stump1;
        break;
    default:
        break;
    }

    /* order is important as we want the tree trunk to be drawn first and the top on top of that */

    pComponent1->type = ETE_Sprite;
    pComponent1->data.sprite.sprite = stumpSprite;
    memset(&pComponent1->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent1->data.sprite.transform.position[1] = trunkOffsetPx;
    pComponent1->data.sprite.transform.scale[0] = 1.0f;
    pComponent1->data.sprite.transform.scale[1] = 1.0f;
    pComponent1->data.sprite.bDraw = true;

    pComponent2->type = ETE_Sprite;
    pComponent2->data.sprite.sprite = trunkSprite;
    memset(&pComponent2->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent2->data.sprite.transform.position[1] = trunkOffsetPx;
    pComponent2->data.sprite.transform.scale[0] = 1.0f;
    pComponent2->data.sprite.transform.scale[1] = 1.0f;
    pComponent2->data.sprite.transform.rotationPointRelative[0] = (float)At_GetSprite(trunkSprite, pGameLayerData->hAtlas)->widthPx / 2.0f;
    pComponent2->data.sprite.transform.rotationPointRelative[1] = (float)At_GetSprite(trunkSprite, pGameLayerData->hAtlas)->heightPx - 46.0f;
    pComponent2->data.sprite.bDraw = true;

    pComponent3->type = ETE_Sprite;
    pComponent3->data.sprite.sprite = topSprite;
    memset(&pComponent3->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent3->data.sprite.transform.scale[0] = 1.0f;
    pComponent3->data.sprite.transform.scale[1] = 1.0f;
    pComponent3->data.sprite.transform.rotationPointRelative[0] = (float)At_GetSprite(topSprite, pGameLayerData->hAtlas)->widthPx / 2.0f;
    pComponent3->data.sprite.transform.rotationPointRelative[1] = (float)At_GetSprite(topSprite, pGameLayerData->hAtlas)->heightPx + (trunkOffsetPx - 46.0f);
    pComponent3->data.sprite.bDraw = true;

    pComponent4->type = ETE_StaticCollider;
    pComponent4->data.staticCollider.shape.type = PBT_Circle;
    pComponent4->data.staticCollider.shape.data.circle.center[0] = x;//transform2Ground[0];
    pComponent4->data.staticCollider.shape.data.circle.center[1] = y;//transform2Ground[1];
    pComponent4->data.staticCollider.shape.data.circle.radius = 6;
    pComponent4->data.staticCollider.bIsSensor = false;
    pComponent4->data.staticCollider.onSensorOverlapBegin = NULL;
    pComponent4->data.staticCollider.onSensorOverlapEnd = NULL;
    pComponent4->data.staticCollider.bGenerateSensorEvents = false;
    pComponent4->data.sprite.bDraw = true;

    HGeneric hTreeData = NULL_HANDLE;
    gTreeDataObjectPool = GetObjectPoolIndex(gTreeDataObjectPool, &hTreeData);
    struct WfTreeEntityData* pTreeData = &gTreeDataObjectPool[hTreeData];
    pTreeData->def = *def;
    pTreeData->groundContactPoint[0] = x;
    pTreeData->groundContactPoint[1] = y;
    pTreeData->health = 100.0f;
    pTreeData->state = WfStanding;
    pTreeData->treeFallRate = TREE_FALL_SPEED;

    pEnt->user.hData = hTreeData;
    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->onDestroy = &TreeOnDestroy;
    pEnt->getSortPos = &TreeGetPreDrawSortValue;
    pEnt->handleEntityMsg = &TreeHandleEntityMsg;
    pEnt->update = &TreeUpdate;
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
        {
            struct WfTreeEntityData entData;
            BS_DeSerializeI32((i32*)&entData.def.season, bs);
            BS_DeSerializeI32((i32*)&entData.def.type, bs);
            BS_DeSerializeI32((i32*)&entData.def.subtype, bs);
            BS_DeSerializeFloat(&entData.groundContactPoint[0], bs);
            BS_DeSerializeFloat(&entData.groundContactPoint[1], bs);
            BS_DeSerializeFloat(&entData.health, bs);
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
    BS_SerializeFloat(pEntData->health, bs);
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

