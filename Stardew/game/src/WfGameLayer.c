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
#include "Scripting.h"
#include "GameFrameworkEvent.h"
#include "WfPersistantGameData.h"

static void WfPublishInventoryChangedEvent()
{
    struct WfInventory* pInv = WfGetInventory();

    struct ScriptCallArgument arg;
	arg.type = SCA_table;
    Sc_NewTableOnStack(12, 0);
    for(int i=0; i<12; i++)
    {
        Sc_PushInt(i + 1);
        Sc_NewTableOnStack(0,2);
        Sc_SetIntAtTableKey("item", pInv->pItems[i].itemIndex);
        Sc_SetIntAtTableKey("quantity", pInv->pItems[i].quantity);
        Sc_SetTable();
    }
    
    int tableRef = Sc_RefTable();

    arg.val.table = tableRef;
    struct LuaListenedEventArgs args = { .numArgs = 1, .args = &arg };
    Ev_FireEvent("InventoryChanged", &args);
    Sc_UnRefTable(tableRef);
	

}

static void WfOnHUDLayerPushed(void* pUserData, void* pEventData)
{
    WfPublishInventoryChangedEvent();
}

void WfGameLayerOnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    pEngineLayer->pUserData = malloc(sizeof(struct WfGameLayerData));
    memset(pEngineLayer->pUserData, 0, sizeof(struct WfGameLayerData));
    GameLayer2D_OnPush(pLayer, drawContext, inputContext);
    struct WfGameLayerData* pWfData = pEngineLayer->pUserData;
    pWfData->HUDPushedEventListener = Ev_SubscribeEvent("onHUDLayerPushed", &WfOnHUDLayerPushed, pLayer);
}

void WfPreFirstInit(struct GameLayer2DData* pEngineLayer)
{
    WfInitGameLayerData(pEngineLayer, (struct WfGameLayerData*)pEngineLayer->pUserData);
}

void WfGameLayerOnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    Game2DLayer_OnPop(pLayer, drawContext, inputContext);
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    struct WfGameLayerData* pWFUserData = pEngineLayer->pUserData;
    Ev_UnsubscribeEvent(pWFUserData->HUDPushedEventListener);
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
