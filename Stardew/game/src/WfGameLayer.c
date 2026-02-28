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
#include "Network.h"
#include "WfNetwork.h"
#include "main.h"
#include "cwalk.h"

static void WfPublishInventoryChangedEvent()
{
    
    struct WfInventory* pInv = WfGetInventory();
    Sc_ResetStack();
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

void WfPublishSelectedItemChangedEvent()
{
    struct WfInventory* pInv = WfGetInventory();
    struct ScriptCallArgument arg;
    arg.type = SCA_int;
    arg.val.i = pInv->selectedItem;
    struct LuaListenedEventArgs args = { .numArgs = 1, .args = &arg };
    Ev_FireEvent("SelectedItemChanged", &args);
}

static void WfPublishInitSettingsEvent(struct GameFrameworkLayer* pLayer)
{
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    struct ScriptCallArgument arg;
    arg.type = SCA_userdata;
    arg.val.userData = pEngineLayer;
    struct LuaListenedEventArgs args = { .numArgs = 1, .args = &arg };
    Ev_FireEvent("InitSettings", &args);
}

static void WfOnHUDLayerPushed(void* pUserData, void* pEventData)
{
    WfPublishInventoryChangedEvent();
    WfPublishSelectedItemChangedEvent();
}

static void WfOnSettingsLayerPushed(void* pUserData, void* pEventData)
{
    WfPublishInitSettingsEvent(pUserData);
}


void WfGameLayerOnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    pEngineLayer->pUserData = malloc(sizeof(struct WfGameLayerData));
    memset(pEngineLayer->pUserData, 0, sizeof(struct WfGameLayerData));
    GameLayer2D_OnPush(pLayer, drawContext, inputContext);
    struct WfGameLayerData* pWfData = pEngineLayer->pUserData;
    pWfData->HUDPushedEventListener = Ev_SubscribeEvent("onHUDLayerPushed", &WfOnHUDLayerPushed, pLayer);
    pWfData->SettingsPushedEventListener = Ev_SubscribeEvent("onSettingsMenuPushed", &WfOnSettingsLayerPushed, pLayer);
}

void WfPreLoadLevel(struct GameLayer2DData* pEngineLayer)
{
    WfInitGameLayerData(pEngineLayer, (struct WfGameLayerData*)pEngineLayer->pUserData);
    WfNetworkInit(pEngineLayer);
}

void WfGameLayerOnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    Game2DLayer_OnPop(pLayer, drawContext, inputContext);
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    struct WfGameLayerData* pWFUserData = pEngineLayer->pUserData;
    Ev_UnsubscribeEvent(pWFUserData->HUDPushedEventListener);
    Ev_UnsubscribeEvent(pWFUserData->SettingsPushedEventListener);
    free(pEngineLayer->pUserData);
    
}

void WfPushGameLayer(DrawContext* pDC, const char* lvlFilePath)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct Game2DLayerOptions options;
    memset(&options, 0, sizeof(struct Game2DLayerOptions));
    char buf[256];
    cwk_path_join(gCmdArgs.assetsDir, "out/main.atlas", buf, 256);
    options.atlasFilePath = buf;
    options.levelFilePath = lvlFilePath;
    Game2DLayer_Get(&testLayer, &options, pDC);
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
