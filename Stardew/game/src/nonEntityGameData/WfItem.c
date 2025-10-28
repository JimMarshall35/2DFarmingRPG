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
}

int l_GetItemUISpriteName(lua_State* L)
{
	if(lua_isinteger(L, -1))
    {
        int arg = lua_tointeger(L, -1);
        volatile int sz = VectorSize(gItemDefs);
        if(arg < VectorSize(gItemDefs) && arg >= 0)
        {
            lua_pushstring(L, gItemDefs[arg].UISpriteName);
            return 1;
        }
        else
        {
            printf("l_GetItemUISpriteName ARGUMENT OUT OF RANGE: %i. Itemdefs size: %i\n", arg, VectorSize(gItemDefs));
        }
    }
    else
    {
        printf("l_GetItemUISpriteName BAD ARGS, expected int\n");
    }
    lua_pushstring(L, "no-item");
    return 1;
}

void WfRegisterItemScriptFunctions()
{
    //void Sc_RegisterCFunction(const char* name, int(*fn)(lua_State*));
    Sc_RegisterCFunction("WfGetItemSpriteName", &l_GetItemUISpriteName);
}