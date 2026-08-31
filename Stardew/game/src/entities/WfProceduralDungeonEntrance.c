#include "WfProceduralDungeonEntrance.h"
#include "Entities.h"
#include "GameFramework.h"
#include "Game2DLayer.h"
#include "BinarySerializer.h"
#include "ObjectPool.h"
#include "Components.h"
#include "Entities.h"
#include "WfEntities.h"
#include "WfWorld.h"
#include "WfUI.h"
#include "Log.h"
#include "WfGameLayerData.h"
#include "WfPlayer.h"
#include "main.h"
#include "WfGameLayer.h"
#include "WfPersistantGameData.h"
#include "Scripting.h"
#include <lua.h>
#include "AssertLib.h"
#include "cwalk.h"
#include "Entity2DCollection.h"

struct WfProceduralDungeonEntranceEntityData
{
    char genScript[64];
    char genFn[64];
    float w, h;
};

static OBJECT_POOL(struct WfProceduralDungeonEntranceEntityData) gProceduralEntranceEntityDataPool = NULL;

static void DestroyProceduralDungeonEntranceEntity(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer)
{
    FreeObjectPoolIndex(gProceduralEntranceEntityDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pLayer);
}

void WfInitProceduralDungeonEntrance()
{
    gProceduralEntranceEntityDataPool = NEW_OBJECT_POOL(struct WfProceduralDungeonEntranceEntityData, 16);
}

static void GenerateProceduralLevel(struct TileMap* pTileMap, DrawContext* pDC, hAtlas atlas, struct GameLayer2DData* pData, void* pUser, struct Entity2DCollection* pEntities)
{
    struct WfProceduralDungeonEntranceEntityData* pUserData = pUser;
    
    char script[256];
    cwk_path_join(gCmdArgs.assetsDir, pUserData->genScript, script, 256);
    
    if(!Sc_OpenFile(script))
    {
        Log_Error("GenerateProceduralLevel can't open file %s", pUserData->genScript);
        return;
    }
    
    struct ScriptCallArgument arguments[5] = 
    {
        {
            .type = SCA_userdata,
            .val.userData = pTileMap
        },
        {
            .type = SCA_userdata,
            .val.userData = pTileMap
        },
        {
            .type = SCA_int,
            .val.userData = atlas
        },
        {
            .type = SCA_userdata,
            .val.userData = pData
        },
        {
            .type = SCA_userdata,
            .val.userData = NULL /* set up a value to be passed here representing stuff such as "what level of the dungeon are we on?", "what is the luck value today?" */
        },
        {
            .type = SCA_userdata,
            .val.userData = pEntities
        }
    };

    Sc_CallGlobalFunc(pUserData->genFn, &arguments[0], 6);
}

void WfPushProceduralDungeonLayer(struct WfProceduralDungeonEntranceEntityData* pSensorData)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct Game2DLayerOptions options;
    memset(&options, 0, sizeof(struct Game2DLayerOptions));
    char buf[256];
    cwk_path_join(gCmdArgs.assetsDir, "out/main.atlas", buf, 256);
    options.atlasFilePath = buf;
    options.Generator = &GenerateProceduralLevel;
    options.pGeneratorUserData = pSensorData;
    options.levelFilePath = "PROCEDURALLY GENERATED";
    Game2DLayer_Get(&testLayer, &options, GetDrawContext());
    testLayer.onPush = &WfGameLayerOnPush;
    testLayer.onPop = &WfGameLayerOnPop;
    struct GameLayer2DData* pEngineLayer = testLayer.userData;
    pEngineLayer->preLoadLevelFn = &WfPreLoadLevel;
    pEngineLayer->preFirstInitCallback = NULL;
    testLayer.flags |= (EnableOnPop | EnableOnPush | EnableUpdateFn | EnableDrawFn | EnableInputFn);
    struct WfPlayerPreferences* pPrefs = WfGetPreferences();
    pEngineLayer->camera.scale[0] = pPrefs->zoomLevel;
    pEngineLayer->camera.scale[1] = pPrefs->zoomLevel;
    GF_PushGameFrameworkLayer(&testLayer);

}

void WfOnProceduralDungeonEntranceSensorOverlapBegin(struct GameFrameworkLayer* pLayer, HEntity2D hOverlappingEntity, HEntity2D thisSensorEntity)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    struct Entity2D* pSensorEnt = Et2D_GetEntity(&pLayerData->entities, thisSensorEntity);
    struct Entity2D* pOverlappingEnt = Et2D_GetEntity(&pLayerData->entities, hOverlappingEntity);
        
    if(pOverlappingEnt->type == WfEntityType_Player)
    {
        // set contact point so next layer can maybe read it back and use to set x or y coordinate
        //WfPlayerGetGroundContactPoint(pOverlappingEnt, gPrevAreaPos);

        struct WfProceduralDungeonEntranceEntityData* pSensorData = &gProceduralEntranceEntityDataPool[pSensorEnt->user.hData];
        if(pLayerData->bCurrentLocationIsDirty)
        {
            char* pPath = WfWorld_GetCurrentLocationFilePath();
            Log_Info("Saving level %s", pPath);
            Game2DLayer_SaveLevelFile(pLayerData, pPath);
            pLayerData->bCurrentLocationIsDirty = false;
        }
        GF_PopGameFrameworkLayer();
        GF_PopGameFrameworkLayer();
        //WfWorld_LoadLocation(pSensorData->toArea, pLayerData->pDrawContext);
        WfPushProceduralDungeonLayer(pSensorData);
        WfPushHUD(pLayerData->pDrawContext);
    }
}

static void WfDeSerializeProceduralDungeonEntranceEntityV1(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    HGeneric hExitData = NULL_HANDLE;
    gProceduralEntranceEntityDataPool = GetObjectPoolIndex(gProceduralEntranceEntityDataPool, &hExitData);
    pOutEnt->user.hData = hExitData;
    struct WfProceduralDungeonEntranceEntityData* pEntranceData = &gProceduralEntranceEntityDataPool[hExitData];
    BS_DeSerializeFloat(&pEntranceData->w, bs);
    BS_DeSerializeFloat(&pEntranceData->h, bs);
    BS_DeSerializeStringInto(pEntranceData->genScript, bs);
    BS_DeSerializeStringInto(pEntranceData->genFn, bs);
    Et2D_PopulateCommonHandlers(pOutEnt);
    pOutEnt->onDestroy = &DestroyProceduralDungeonEntranceEntity;
    struct Component2D* pComponent1 = &pOutEnt->components[pOutEnt->numComponents++];
    pComponent1->type = ETE_StaticCollider;
    pComponent1->data.staticCollider.bIsSensor = true;
    pComponent1->data.staticCollider.onSensorOverlapBegin = &WfOnProceduralDungeonEntranceSensorOverlapBegin;
    pComponent1->data.staticCollider.onSensorOverlapEnd = NULL;
    pComponent1->data.staticCollider.shape.type = PBT_Rect;
    pComponent1->data.staticCollider.shape.data.rect.w = gProceduralEntranceEntityDataPool[hExitData].w;
    pComponent1->data.staticCollider.shape.data.rect.h = gProceduralEntranceEntityDataPool[hExitData].h;
    pComponent1->data.staticCollider.bGenerateSensorEvents = true;
    pOutEnt->bSerializeToDisk = true;
    pOutEnt->bSerializeToNetwork = true;
}

void WfDeSerializeProceduralDungeonEntranceEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version;
    BS_DeSerializeU32(&version, bs);
    switch (version)
    {
    case 1:
        /* code */
        WfDeSerializeProceduralDungeonEntranceEntityV1(bs, pOutEnt, pData);
        break;
    default:
        break;
    }
}

void WfSerializeProceduralDungeonEntranceEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    BS_SerializeU32(1, bs);
    struct WfProceduralDungeonEntranceEntityData* pEntData = &gProceduralEntranceEntityDataPool[pInEnt->user.hData];
    BS_SerializeFloat(pEntData->w, bs);
    BS_SerializeFloat(pEntData->h, bs);
    BS_SerializeString(pEntData->genScript, bs);
    BS_SerializeString(pEntData->genFn, bs);

}

