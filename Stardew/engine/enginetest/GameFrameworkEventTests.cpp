#include <gtest/gtest.h>
#include <filesystem>
#include "GameFramework.h"
#include "XMLUIGameLayer.h"
#include "Scripting.h"
#include "DrawContext.h"
#include "GameFrameworkEvent.h"
#include "DataNode.h"
#include "lua.h"
#include <lualib.h>
#include <lauxlib.h>

struct GameFrameworkEventListener* gOnUIPushListener = NULL;

HUIVertexBuffer MockNewUIVertexBufferFn(int size)
{
    return NULL_HANDLE;
}

void MockSetCurrentAtlasFn(hTexture atlas)
{
    
}

static bool bOnPushCalled = false;

void OnGameUIPush(void* pUserData, void* pEventData)
{
    bOnPushCalled = true;

    struct DataNode* pNode = (struct DataNode*)pEventData;
    enum DNPropValType type = pNode->fnGetPropType(pNode, "prop");
    ASSERT_EQ(type, DN_String);
    ASSERT_TRUE(pNode->fnStrCmp(pNode, "prop", "val"));
    
    char* c = (char*)malloc(pNode->fnGetStrlen(pNode, "prop") + 1);
    pNode->fnGetStrcpy(pNode, "prop", c);

    ASSERT_EQ(strcmp(c, "val"), 0);

    free(c);

    type = pNode->fnGetPropType(pNode, "intProp");
    ASSERT_EQ(type, DN_Int);
    int intv = pNode->fnGetInt(pNode, "intProp");
    ASSERT_EQ(intv, 42);

    type = pNode->fnGetPropType(pNode, "floatProp");
    ASSERT_EQ(type, DN_Float);
    float fV = pNode->fnGetFloat(pNode, "floatProp");
    ASSERT_FLOAT_EQ(fV, 32.5);
    
    type = pNode->fnGetPropType(pNode, "boolProp");
    ASSERT_EQ(type, DN_Bool);
    bool bV = pNode->fnGetBool(pNode, "boolProp");
    ASSERT_EQ(bV, false);
}

/* TODO: FIX THIS AND THE ONE BELOW */
// TEST(Events, FilePresent)
// {
// 	ASSERT_TRUE(std::filesystem::exists("data/GameFrameworkEventTestUILayer.lua"));
// 	ASSERT_TRUE(std::filesystem::exists("data/GameFrameworkEventTestUILayer.xml"));
// }


bool bLuaTestCallbackCalled = false;
int L_LuaTestCallback(lua_State* L)
{
    bLuaTestCallbackCalled = true;
    
    // todo: add tests here for the passed args

    return 0;
}

// TEST(Events, EventsIntegrationTest)
// {
//     // 1.) Setup environment
//     DrawContext dc;
//     memset(&dc, 0, sizeof(DrawContext));
//     dc.NewUIVertexBuffer = &MockNewUIVertexBufferFn;
//     dc.SetCurrentAtlas = &MockSetCurrentAtlasFn;
//     Sc_InitScripting();
//     Sc_RegisterCFunction("TestCallback", &L_LuaTestCallback);
//     UI_Init();
//     GF_InitGameFramework();
    
//     // 2.) subscribe to event "OnGameHUDPush"
//     gOnUIPushListener = Ev_SubscribeEvent("OnGameHUDPush", &OnGameUIPush, NULL);
    
//     // 3.) setup a layer to be pushed
//     struct GameFrameworkLayer testLayer;
//     memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
//     struct XMLUIGameLayerOptions options;
//     options.bLoadImmediately = false;
//     options.xmlPath = "./data/GameFrameworkEventTestUILayer.xml";
//     options.pDc = NULL;
//     testLayer.flags |= (EnableOnPush | EnableOnPop);
//     XMLUIGameLayer_Get(&testLayer, &options);
//     GF_PushGameFrameworkLayer(&testLayer);
    
//     // 4.) end the frame, actually pushing the layer. 
//     // The push will trigger a callback in the vm, which will fire the event  we subscribed to in 2.),
//     // as well as subscribe to another event, UpdateDisplayedInventory
//     GF_EndFrame(&dc, NULL);
//     ASSERT_TRUE(bOnPushCalled);

//     // 5.) setup lua event call args
//     // todo: add args here and test in the callback
//     struct LuaListenedEventArgs args {0,0};
    
//     // 6.) fire event listened to by the viewmodel
//     Ev_FireEvent("UpdateDisplayedInventory", &args);
//     ASSERT_EQ(bLuaTestCallbackCalled, true);

//     // 7.) cleanup - unsubscribe events
//     ASSERT_TRUE(Ev_UnsubscribeEvent(gOnUIPushListener));
//     GF_PopGameFrameworkLayer(); // this calls the lua unsubscribe function and the on pop callback - or should

//     // todo: add tests to verify unsubscription
//     Sc_DeInitScripting();
// }