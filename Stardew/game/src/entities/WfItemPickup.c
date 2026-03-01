#include "WfItemPickup.h"

#include "Entities.h"
#include "BinarySerializer.h"
#include "Game2DLayer.h"
#include "WfItem.h"
#include "WfEntities.h"
#include "ObjectPool.h"
#include "GameFramework.h"
#include "Log.h"
#include "WfPlayer.h"
#include "WfPersistantGameData.h"
#include "WfGameLayer.h"

struct WfItemPickupEntData
{
    struct WfItemPickupDef def;
    // this could in future include other stuff like a timer after which it deletes, etc
};

OBJECT_POOL(struct WfItemPickupEntData) gItemPickupDataPool = NULL;

void WfInitItemPickup()
{
    gItemPickupDataPool = NEW_OBJECT_POOL(struct WfItemPickupEntData, 32);
}

void WfDeSerializeItemPickupEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{

}

void WfSerializeItemPickupEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{

}

void WfOnPickupSensorOverlapBegin(struct GameFrameworkLayer* pLayer, HEntity2D hOverlappingEntity, HEntity2D thisSensorEntity)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    struct Entity2D* pSensorEnt = Et2D_GetEntity(&pLayerData->entities, thisSensorEntity);
    struct Entity2D* pOverlappingEnt = Et2D_GetEntity(&pLayerData->entities, hOverlappingEntity);
    struct WfItemPickupEntData* pEntData = &gItemPickupDataPool[pSensorEnt->user.hData];
    if(pOverlappingEnt->type = WfEntityType_Player)
    {
        /* add to players inventory here */
        struct WfPlayerEntData* pPlayerEntData = WfGetPlayerEntData(pOverlappingEnt);
        struct WfInventory* pInv = WfGetPlayerInventory(pPlayerEntData);
        WfAddToInventory(pInv, pEntData->def.itemID, pEntData->def.itemQuantity);
        Et2D_DestroyEntity(pLayer, &pLayerData->entities, thisSensorEntity);
        if(!pPlayerEntData->bNetworkControlled)
        {
            WfPublishInventoryChangedEvent();
        }
    }
}

static void ItemPickupOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    
    FreeObjectPoolIndex(gItemPickupDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

void WfMakeEntityIntoPickupBasedAt(float x, float y, struct WfItemPickupDef* def, struct GameLayer2DData* pGameLayerData, struct Entity2D* pEnt)
{
    const struct WfItemDef* pDef = WfGetItemDef(def->itemID);
    hSprite s = At_FindSprite(pDef->pickupSpriteName, pGameLayerData->hAtlas);
    AtlasSprite* pSprite = At_GetSprite(s, pGameLayerData->hAtlas);
    
    memset(pEnt, 0, sizeof(struct Entity2D));

    gItemPickupDataPool = GetObjectPoolIndex(gItemPickupDataPool, &pEnt->user.hData);
    gItemPickupDataPool[pEnt->user.hData].def = *def;

    // sprite is centered at x, y
    pEnt->transform.position[0] = x - pSprite->widthPx / 2;
    pEnt->transform.position[1] = y - pSprite->heightPx / 2;
    pEnt->transform.scale[0] = 1.0f;
    pEnt->transform.scale[1] = 1.0f;
    pEnt->transform.rotation = 0.0f;
    pEnt->transform.rotationPointRelative[0] = 0.0f;
    pEnt->transform.rotationPointRelative[1] = 0.0f;

    pEnt->bKeepInQuadtree = true;//true;
    pEnt->bKeepInDynamicList = false;//false;
    pEnt->type = WfEntityType_ItemPickup;

    struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
    struct Component2D* pComponent2 = &pEnt->components[pEnt->numComponents++];

    pComponent2->type = ETE_Sprite;
    pComponent2->data.sprite.sprite = s;
    memset(&pComponent2->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent2->data.sprite.transform.scale[0] = 1.0f;
    pComponent2->data.sprite.transform.scale[1] = 1.0f;
    pComponent2->data.sprite.bDraw = true;

    pComponent1->type = ETE_StaticCollider;
    pComponent1->data.staticCollider.bIsSensor = true;
    pComponent1->data.staticCollider.onSensorOverlapBegin = &WfOnPickupSensorOverlapBegin;
    pComponent1->data.staticCollider.onSensorOverlapEnd = NULL;
    pComponent1->data.staticCollider.shape.type = PBT_Rect;
    pComponent1->data.staticCollider.shape.data.rect.w = pSprite->widthPx;
    pComponent1->data.staticCollider.shape.data.rect.h = pSprite->heightPx;
    pComponent1->data.staticCollider.bGenerateSensorEvents = true;


    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->onDestroy = &ItemPickupOnDestroy;
    //pEnt->getSortPos = &TreeGetPreDrawSortValue;
    //pEnt->handleEntityMsg = &TreeHandleEntityMsg;
    //pEnt->update = &TreeUpdate;
    pEnt->bSerializeToDisk = true;
    pEnt->bSerializeToNetwork = true;
}

HEntity2D WfAddPickupBasedAt(float x, float y, struct WfItemPickupDef* def, struct GameLayer2DData* pGameLayerData)
{
    struct Entity2D ent;
    WfMakeEntityIntoPickupBasedAt(x, y, def, pGameLayerData, &ent);
    HEntity2D hEnt = Et2D_AddEntity(&pGameLayerData->entities, &ent);
    struct Entity2D* pEnt = Et2D_GetEntity(&pGameLayerData->entities, hEnt);
    pEnt->init(pEnt, pGameLayerData->pLayer, pGameLayerData->pDrawContext, NULL);
}