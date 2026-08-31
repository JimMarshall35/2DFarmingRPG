#include "WfScriptFunctions.h"
#include "WfItem.h"
#include "WfUI.h"
#include "lua.h"
#include <lualib.h>
#include "Scripting.h"
#include "main.h"
#include "Game2DLayer.h"
#include "WfPersistantGameData.h"
#include "Log.h"
#include "WfPlayerStart.h" /*WfGetCurrentLocalPlayer*/
#include "Entities.h"
#include "WfPlayer.h"
#include "WfGenerateMap.h"

static int L_PushHUDLayer(lua_State* L)
{
    DrawContext* pDC = GetDrawContext();
    WfPushHUD(pDC);
    return 0;
}

static int L_SavePreferences(lua_State* L)
{
    if(lua_gettop(L) != 1)
    {
        Log_Error("L_SavePreferences ERROR");
    }
    else if(lua_islightuserdata(L, -1))
    {
        struct GameLayer2DData* pGL2D = lua_topointer(L, -1);
        struct WfPlayerPreferences* pPrefs = WfGetPreferences();
        pPrefs->zoomLevel = pGL2D->camera.scale[0];
    }
    else
    {
        Log_Error("L_SavePreferences ERROR");
    }
    return 0;
}

static int L_GetPlayerLocation(lua_State* L)
{
    if(lua_gettop(L) != 1)
    {
        Log_Error("L_GetPlayerLocation ERROR");
    }
    if(!lua_islightuserdata(L, 1))
    {
        Log_Error("L_GetPlayerLocation argument needs to be Game2DLayerData ptr");
        lua_pushnumber(L,0);
        lua_pushnumber(L,0);
        return 2;
    }
    HEntity2D hPlayer = WfGetCurrentLocalPlayer();
    vec2 pos = { 0.0f, 0.0f };

    if(hPlayer != NULL_HANDLE)
    {
        struct GameLayer2DData* pGL2D = lua_topointer(L, 1);
        struct Entity2D* pPlayer = Et2D_GetEntity(&pGL2D->entities, hPlayer);
        WfPlayerGetGroundContactPoint(pPlayer, pos);
    }
    else
    {
        Log_Error("L_GetPlayerLocation hPlayer != NULL_HANDLE");
    }
    lua_pushnumber(L,pos[0]);
    lua_pushnumber(L,pos[1]);
    return 2;
}

static int L_SetInventorySlot(lua_State* L)
{
    if(!lua_isinteger(L, -1))
    {
        Log_Error("L_GetLocalPlayer, arg 1 should be an int");
    }

    if(!lua_isinteger(L, -2))
    {
        Log_Error("L_GetLocalPlayer, arg 2 should be an int");
    }

    if(!lua_isinteger(L, -3))
    {
        Log_Error("L_GetLocalPlayer, arg 3 should be an int");
    }

    int quantity = lua_tointeger(L, -1);
    int item = lua_tointeger(L, -2);
    int slot = lua_tointeger(L, -3);

    struct WfPersistantData* pPersistant = WfGetLocalPlayerPersistantGameData();
    pPersistant->inventory.pItems[slot].itemIndex = item;
    pPersistant->inventory.pItems[slot].quantity = quantity;
    return 1;
}

static int L_SetEquipmentSlot(lua_State* L)
{
    if(!lua_isinteger(L, -1))
    {
        Log_Error("L_SetEquipmentSlot, arg 1 should be an int");
    }

    if(!lua_isinteger(L, -2))
    {
        Log_Error("L_SetEquipmentSlot, arg 2 should be an int");
    }

    int slot = lua_tointeger(L, -1);
    int item = lua_tointeger(L, -2);
    struct WfPersistantData* pPersistant = WfGetLocalPlayerPersistantGameData();

    switch(slot)
    {
    case 1:
        pPersistant->inventory.legItem = item;
        {
            struct WfItemDef* pItemDef = WfGetItemDef(pPersistant->inventory.legItem);

        }
        break;
    case 2:
        pPersistant->inventory.torsoItem = item;
        break;
    }
    return 0;
}


void WfRegisterScriptFunctions()
{
    WfRegisterItemScriptFunctions();
    Sc_RegisterCFunction("WfPushHUD", &L_PushHUDLayer);
    Sc_RegisterCFunction("WfSavePreferences", &L_SavePreferences);
    Sc_RegisterCFunction("WfGetPlayerLocation", &L_GetPlayerLocation);
    Sc_RegisterCFunction("WfSetInventorySlot", &L_SetInventorySlot);
    Sc_RegisterCFunction("WfSetEquipmentSlot", &L_SetEquipmentSlot);
    WfRegisterMapGenLuaFunctions();
}

