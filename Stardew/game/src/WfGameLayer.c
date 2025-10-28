#include "GameFramework.h"
#include <string.h>
#include "Game2DLayer.h"
#include "XMLUIGameLayer.h"
#include "DynArray.h"
#include "Entities.h"
#include "EntityQuadTree.h"
#include "WfEntities.h"
#include "Physics2D.h"
#include "WfInit.h"
#include "Random.h"
#include "WfGameLayerData.h"
#include "WfGameLayer.h"

static void WfOnHUDLayerPushed(void* pUserData, void* pEventData)
{

}

void WfGameLayerOnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    pEngineLayer->pUserData = malloc(sizeof(struct WfGameLayerData));
    memset(pEngineLayer->pUserData, 0, sizeof(struct WfGameLayerData));
    GameLayer2D_OnPush(pLayer, drawContext, inputContext);
    Ev_SubscribeEvent("onHUDLayerPushed", &WfOnHUDLayerPushed, pLayer);
}

void WfPreFirstInit(struct GameLayer2DData* pEngineLayer)
{
    WfInitGameLayerData(pEngineLayer, (struct WfGameLayerData*)pEngineLayer->pUserData);
}

void WfGameLayerOnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    Game2DLayer_OnPop(pLayer, drawContext, inputContext);
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    free(pEngineLayer->pUserData);
}

void WfPushGameLayer(DrawContext* pDC, const char* lvlFilePath)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct Game2DLayerOptions options;
    memset(&options, 0, sizeof(struct Game2DLayerOptions));
    options.atlasFilePath = "./Assets/out/main.atlas";
    options.levelFilePath = lvlFilePath;
    Game2DLayer_Get(&testLayer, &options, pDC);
    testLayer.onPush = &WfGameLayerOnPush;
    testLayer.onPop = &WfGameLayerOnPop;
    struct GameLayer2DData* pEngineLayer = testLayer.userData;
    pEngineLayer->preFirstInitCallback = &WfPreFirstInit;
    testLayer.flags |= (EnableOnPop | EnableOnPush | EnableUpdateFn | EnableDrawFn | EnableInputFn);
    GF_PushGameFrameworkLayer(&testLayer);
}
