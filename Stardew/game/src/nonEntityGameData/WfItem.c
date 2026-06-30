// std lib
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

// 3rd party
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "cwalk.h"
#include "lua.h"
#include <lualib.h>

// zzfx
#include "ZzFX.h"

// engine
#include "DynArray.h"
#include "AssertLib.h"
#include "Scripting.h"
#include "Log.h"
#include "main.h"
#include "FilesystemUtils.h"
#include "XMLHelpers.h"
#include "XMLSchema.h"
#include "SharedLib.h"
#include "StringKeyHashMap.h"

// game
#include "WfAllItems.h"
#if BAKE_ITEM_DEFS
#include "BakedItems.h"
#endif

static VECTOR(struct WfItemDef) gItemDefs = NULL;
static struct HashMap gItemNameHashmap;


void WfAddItemDef(struct WfItemDef* pDef)
{
    gItemDefs = VectorPush(gItemDefs, pDef);
    int i = VectorSize(gItemDefs) - 1;
    HashmapInsert(&gItemNameHashmap, pDef->itemName, &i);
}

static void* ResolveXMLSpecifiedFunction(xmlNode* pXMLFn)
{
    void* pR = NULL;
    if(!pXMLFn)
    {
        return NULL;
    }
    xmlNode* pChild = GetNthChild(pXMLFn, 0);
    if(strcmp( pChild->name, "c-function") == 0)
    {
        xmlChar* dll = NULL;
        xmlChar* name = xmlGetProp(pChild, "name");
        if(dll = xmlGetProp(pChild, "dll"))
        {
            char buf[256];
            cwk_path_join(gCmdArgs.assetsDir, name, buf, 256);
#ifdef WIN32_
            strcat(dll, ".dll");
#else
            strcat(dll, ".so");
#endif
            struct SharedLib* pSO = SharedLib_LoadSharedLib(buf);
            pR = SharedLib_GetProc(pSO, name);
        }
        else
        {
            pR = SharedLib_GetCurrentlyLoadedFn(name);
        }
    }
    else if(strcmp(pChild->name, "lua-function") == 0)
    {
        EASSERT(false); // not implemented yet
    }
    return pR;
}

enum WfActionAnimation GetAnimationFromString(xmlChar* str)
{
    if(xmlStrcmp(str, "WfSlashAnim") == 0)
    {
        return WfSlashAnim;
    }
    else if(xmlStrcmp(str, "WfThrustAnim") == 0)
    {
        return WfThrustAnim;
    }
    return WfNoActionAnim;
}

static struct ZZFXSound ParseZzfxSound(xmlNode* pZzfxOnPickup)
{
    if(!pZzfxOnPickup)
    {
        struct ZZFXSound snd = {};
        return snd;
    }
    float vals[21];
    xmlChar* zzfxString = xmlGetProp(pZzfxOnPickup, "str");
    sscanf(zzfxString, "{%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f}",
        &vals[0],
        &vals[1],
        &vals[2],
        &vals[3],
        &vals[4],
        &vals[5],
        &vals[6],
        &vals[7],
        &vals[8],
        &vals[9],
        &vals[10],
        &vals[11],
        &vals[12],
        &vals[13],
        &vals[14],
        &vals[15],
        &vals[16],
        &vals[17],
        &vals[18],
        &vals[19],
        &vals[20]
    );
    struct ZZFXSound r = {
        vals[0],
        vals[1],
        vals[2],
        vals[3],
        vals[4],
        vals[5],
        vals[6],
        vals[7],
        vals[8],
        vals[9],
        vals[10],
        vals[11],
        vals[12],
        vals[13],
        vals[14],
        vals[15],
        vals[16],
        vals[17],
        vals[18],
        vals[19],
        vals[20]
    };
    return r;
}

static struct WfItemConfigPropertyBag ParseConfigData(xmlNode* pConfigData)
{
    struct WfItemConfigPropertyBag b;
    b.bSet = false;
    if(!pConfigData)
    {
        return b;
    }
    HashmapInit(&b.properties, 16, sizeof(struct WfItemConfigProperty));
    unsigned long numchildren = xmlChildElementCount(pConfigData);
    for(int i = 0; i < numchildren; i++)
    {
        xmlNode* pChildI = GetNthChild(pConfigData, i);
        int setCount = 0;
        struct WfItemConfigProperty prop;
        xmlChar* propName = xmlGetProp(pChildI, "name");
        xmlChar* value = xmlGetProp(pChildI, "value");
        if(strcmp(pChildI->name, "Float") == 0)
        {
            prop.type = WfItemConfig_Float;
            prop.val.floatVal = (float)atof(value);
        }
        if(strcmp(pChildI->name, "Int") == 0)
        {
            prop.type = WfItemConfig_Int;
            prop.val.intVal = atoi(value);
        }
        if(strcmp(pChildI->name, "Bool") == 0)
        {
            prop.type = WfItemConfig_Bool;
            prop.val.boolVal = (strcmp(value, "true") == 0) ? true : false;
        }
        if(strcmp(pChildI->name, "String") == 0)
        {
            prop.type = WfItemConfig_String;
            prop.val.stringVal = malloc(strlen(value) + 1);
            strcpy(prop.val.stringVal, value);
        }
        if(strcmp(pChildI->name, "Array") == 0)
        {
            prop.type = WfItemConfig_Array;
            unsigned long numGrandchildrend = xmlChildElementCount(pChildI);
            prop.val.propertyArray.pData = malloc(sizeof(struct WfItemConfigProperty) * numGrandchildrend);
            prop.val.propertyArray.length = numGrandchildrend;
            for(int j=0; j<numGrandchildrend; j++)
            {
                prop.val.propertyArray.pData[j].val.bag = ParseConfigData(GetNthChild(pChildI, j));
            }
        }
        HashmapInsert(&b.properties, propName, &prop);
        b.bSet = true;
    }
    return b;
}

static void AddItemDefXML(xmlNode* pChildI)
{
    xmlNode* pUISpriteName     = XMLFindChild(pChildI, "ui-sprite-name");
    xmlNode* pOnMakeCurrent    = XMLFindChild(pChildI, "on-make-current");
    xmlNode* pOnStopCurrent    = XMLFindChild(pChildI, "on-stop-being-current");
    xmlNode* pOnUseItem        = XMLFindChild(pChildI, "on-use-item");
    xmlNode* pOnTryEquip       = XMLFindChild(pChildI, "on-try-equip");
    xmlNode* pOnTryUnEquip     = XMLFindChild(pChildI, "on-try-unequip");
    xmlNode* pOnDrawDebugLines = XMLFindChild(pChildI, "on-draw-debug-lines");
    xmlNode* pOnUseAnimation   = XMLFindChild(pChildI, "on-use-animation");
    xmlNode* pOnInitItemDef    = XMLFindChild(pChildI, "on-init-itemdef");
    xmlNode* pCanUseItem       = XMLFindChild(pChildI, "can-use-item");
    xmlNode* pPickupSpriteName = XMLFindChild(pChildI, "pickup-sprite-name");
    xmlNode* pOnGamelayerPush  = XMLFindChild(pChildI, "on-gamelayer-push");
    xmlNode* pConfigData       = XMLFindChild(pChildI, "config-data");
    xmlNode* pbSFXOnPickup     = XMLFindChild(pChildI, "b-sound-effect-on-pickup");
    xmlNode* pZzfxOnPickup     = XMLFindChild(pChildI, "zzfx-sound-on-pickup");

    xmlChar* itemName = xmlGetProp(pChildI, "name");

    struct WfItemDef def = {
        .UISpriteName         = xmlGetProp(pUISpriteName, "str"),
        .onMakeCurrent        = ResolveXMLSpecifiedFunction(pOnMakeCurrent),
        .onStopBeingCurrent   = ResolveXMLSpecifiedFunction(pOnStopCurrent),
        .onUseItem            = ResolveXMLSpecifiedFunction(pOnUseItem),
        .onTryEquip           = ResolveXMLSpecifiedFunction(pOnTryEquip),
        .onTryUnequip         = ResolveXMLSpecifiedFunction(pOnTryUnEquip),
        .onGameLayerPush      = ResolveXMLSpecifiedFunction(pOnGamelayerPush),
        .drawDebugLines       = ResolveXMLSpecifiedFunction(pOnDrawDebugLines),
        .init                 = ResolveXMLSpecifiedFunction(pOnInitItemDef),
        .onUseAnimation       = GetAnimationFromString(xmlGetProp(pOnUseAnimation, "str")),
        .bCanUseItem          = strcmp(xmlGetProp(pCanUseItem, "bool"), "true") == 0,
        .pickupSpriteName     = xmlGetProp(pPickupSpriteName, "str"),
        .bSoundEffectOnPickup = pbSFXOnPickup && (strcmp(xmlGetProp(pbSFXOnPickup, "bool"), "true") == 0),
        .zzfxPickup           = ParseZzfxSound(pZzfxOnPickup),
        .itemName             = malloc(strlen(itemName) + 1),
        .config               = ParseConfigData(pConfigData),
    };
    strcpy(def.itemName, itemName);
    WfAddItemDef(&def);
}

static void InitItemDefs()
{
    for(int i=0; i<VectorSize(gItemDefs); i++)
    {
        if(gItemDefs->init)
        {
            gItemDefs->init(&gItemDefs[i]);
        }
    }
}

void WfInitItems()
{
#if BAKE_ITEM_DEFS
    gItemDefs = NEW_VECTOR(struct WfItemDef);
    HashmapInit(&gItemNameHashmap, 32, sizeof(int));
    WfInitBakedItems();
    InitItemDefs();
    return;
#else
    char buf[256];
    char buf2[256];
    gItemDefs = NEW_VECTOR(struct WfItemDef);
    HashmapInit(&gItemNameHashmap, 32, sizeof(int));
    cwk_path_join(gCmdArgs.assetsDir, "items_data.xml", buf, 256);
    cwk_path_join(gCmdArgs.assetsDir, "items_data_schema.xml", buf2, 256);
    xmlDoc* pXMLDoc = NULL;
    if(FS_DoesFileExist(buf))
    {
        if(FS_DoesFileExist(buf2))
        {
            bool valid = XS_XSDValidate(buf2, buf);
            if(!valid)
            {
                Log_Error("items_data.xml: invalid schema. Falling back to builtin items");
            }
        }
        else
        {
            Log_Error("Schema for item data doesn't exist! Attempting to continue without");
        }
        pXMLDoc = xmlReadFile(buf, NULL, 0);
        
        if (pXMLDoc)
        {
            Log_Verbose("pXMLDoc is valid");
            xmlNode* root = xmlDocGetRootElement(pXMLDoc);
            unsigned long numchildren = xmlChildElementCount(root);
            for(int i = 0; i < numchildren; i++)
            {
                xmlNode* pChildI = GetNthChild(root, i);
                AddItemDefXML(pChildI);
            }
        }
    }
    if (pXMLDoc)
    {
        xmlFreeDoc(pXMLDoc);
    }
    InitItemDefs();
#endif
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

struct WfItemDef* WfGetItemDefs(int* numDefs)
{
    *numDefs = VectorSize(gItemDefs);
    return gItemDefs;
}

struct WfItemConfigProperty* WfGetItemConfigProperty(struct WfItemConfigPropertyBag* pBag, const char* propertyName)
{
    if(!pBag->bSet)
    {
        Log_Warning("Warning, item has no config properties set!");
        return NULL;
    }
    return HashmapSearch(&pBag->properties, propertyName);
}
