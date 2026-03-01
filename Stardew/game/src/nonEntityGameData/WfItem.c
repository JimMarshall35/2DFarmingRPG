#include "WfItem.h"
#include "DynArray.h"
#include "AssertLib.h"
#include "Scripting.h"
#include <stdlib.h>
#include "lua.h"
#include <lualib.h>
#include "WfBasicAxe.h"
#include "WfBasicBow.h"
#include "WfBasicFishingRod.h"
#include "WfBasicHoe.h"
#include "WfBasicPickaxe.h"
#include "WfBasicScythe.h"
#include "WfBasicSword.h"
#include "Log.h"

static VECTOR(struct WfItemDef) gItemDefs = NULL;

void WfInitItems()
{
    gItemDefs = NEW_VECTOR(struct WfItemDef);
    WfAddBuiltinItems();
}

void WfAddItemDef(struct WfItemDef* pDef)
{
    gItemDefs = VectorPush(gItemDefs, pDef);
}

void WfAddBuiltinItems()
{
    WfAddBasicAxeDef();
    WfAddBasicSwordDef();
    WfAddBasicPickaxeDef();
    WfAddBasicScytheDef();
    WfAddBasicFishingRodDef();
    WfAddBasicHoeDef();
    WfAddBasicBowDef();
    WfAddWoodItemDef();
}

int l_GetItemUISpriteName(lua_State* L)
{
	if(lua_isinteger(L, -1))
    {
        int arg = lua_tointeger(L, -1);
        if(arg < VectorSize(gItemDefs) && arg >= 0)
        {
            lua_pushstring(L, gItemDefs[arg].UISpriteName);
            return 1;
        }
        else
        {
            Log_Error("l_GetItemUISpriteName ARGUMENT OUT OF RANGE: %i. Itemdefs size: %i", arg, VectorSize(gItemDefs));
        }
    }
    else
    {
        Log_Error("l_GetItemUISpriteName BAD ARGS, expected int");
    }
    lua_pushstring(L, "no-item");
    return 1;
}

void WfRegisterItemScriptFunctions()
{
    Sc_RegisterCFunction("WfGetItemSpriteName", &l_GetItemUISpriteName);
}

const struct WfItemDef* WfGetItemDef(int itemIndex)
{
    if(itemIndex < 0)
    {
        return NULL;
    }
    return &gItemDefs[itemIndex];
}
