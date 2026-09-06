#include "WfGenerateMap.h"
#include <lua.h>
#include <lualib.h>
#include "HandleDefs.h"
#include "Game2DLayer.h"
#include "EngineUtils.h"
#include "Log.h"
#include <stdbool.h>
#include "Scripting.h"
#include "Atlas.h"
#include "WfPlayerStart.h"
#include "WfItemHelpers.h"
#include "StaticColliderEntity.h"
#include "WfExit.h"
#include "WfProceduralDungeonEntrance.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// C Functions

static struct TileMapLayer* TilemapAddLayer(struct TileMap* pTileMap, enum TileMapLayerType type, vec2 position, enum ObjectLayer2DDrawOrder drawOrder)
{
    struct TileMapLayer layer;
    ZeroMemory(&layer, sizeof(struct TileMapLayer));
    layer.bIsObjectLayer = type == TilemapType_Entity ? true : false;
    layer.drawOrder = drawOrder;
    layer.type = type;
    layer.transform.position[0] = position[0];
    layer.transform.position[1] = position[1];

    if(!layer.bIsObjectLayer)
    {
        layer.tileHeightPx = 32;
        layer.tileWidthPx = 32;
    }
    pTileMap->layers = VectorPush(pTileMap->layers, &layer);
    return &pTileMap->layers[VectorSize(pTileMap->layers) - 1];
}

/// @brief Resize layer but preserve the tiles
/// @param pTargetLayer 
/// @param newW 
/// @param newH 
static void TileLayerResize(struct TileMapLayer* pTargetLayer, int newW, int newH)
{
    if(newW < pTargetLayer->widthTiles)
    {
        Log_Error("TileLayerResize: newW (%i) is < old width (%i)", newW, pTargetLayer->tileWidthPx);
        return;
    }
    
    if(newH < pTargetLayer->heightTiles)
    {
        Log_Error("TileLayerResize: newH (%i) is < old height (%i)", newH, pTargetLayer->tileHeightPx);
        return;
    }

    TileIndex* newTilemap = malloc(newW * newH * sizeof(TileIndex));
    ZeroMemory(newTilemap, sizeof(TileIndex) * newW * newH);
    TileIndex* pWrite = newTilemap;
    TileIndex* pRead = pTargetLayer->Tiles;

    for(int i = 0; i < pTargetLayer->heightTiles; i++)
    {
        memcpy(pWrite, pRead, sizeof(TileIndex) * pTargetLayer->widthTiles);
        pWrite += newW;
        pRead += pTargetLayer->widthTiles;
    }
    
    free(pTargetLayer->Tiles);
    pTargetLayer->Tiles = newTilemap;
    pTargetLayer->widthTiles = newW;
    pTargetLayer->heightTiles = newH;
}

/// @brief Set a tile, if the coordiates are outside of the tilemap layer size, resize the tilemap layer
/// @param pTileMap 
/// @param tile 
/// @param layer 
/// @param x 
/// @param y 
static void TilemapSetTile(struct TileMap* pTileMap, TileIndex tile, int layer, int x, int y)
{
    if(VectorSize(pTileMap->layers) <= layer)
    {
        Log_Error("TilemapSetTile: tile layer %i out of bounds (theirs %i layers)", layer, VectorSize(pTileMap->layers));
        return;
    }
    struct TileMapLayer* pTargetLayer = &pTileMap->layers[layer];
    if(pTargetLayer->type != TilemapType_Tile)
    {
        Log_Error("TilemapSetTile: target layer %i is not a tilemap type, it's a %i", layer, pTargetLayer->type);
        return;
    }
    if(x >= pTargetLayer->widthTiles || y >= pTargetLayer->heightTiles)
    {
        // we need to resize
        if(x >= pTargetLayer->widthTiles && y >= pTargetLayer->heightTiles)
        {
            // point out of bounds in both dims
            TileLayerResize(pTargetLayer, x + 1, y + 1);
        }
        else if (x >= pTargetLayer->widthTiles)
        {
            // point out of bounds in x
            TileLayerResize(pTargetLayer, x + 1, pTargetLayer->heightTiles);
        }
        else if(y >= pTargetLayer->heightTiles)
        {
            // point out of bounds in y
            TileLayerResize(pTargetLayer, pTargetLayer->widthTiles, y + 1);
        }
    }
    pTargetLayer->Tiles[y * pTargetLayer->widthTiles + x] = tile;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Lua Wrappers

static int L_TilemapAddLayer(lua_State* L)
{
    /* struct TileMap* pTileMap, enum TileMapLayerType type, float positionX, float positionY, enum ObjectLayer2DDrawOrder drawOrder */
    if(!lua_isinteger(L, -1))
    {
        // drawOrder
        Log_Error("L_TilemapAddLayer, arg 5 should be a draw order enum");
    }

    if(!lua_isnumber(L, -2))
    {
        // position y
        Log_Error("L_TilemapAddLayer, arg 4 should be a number");
    }

    if(!lua_isnumber(L, -3))
    {
        // position x
        Log_Error("L_TilemapAddLayer, arg 3 should be a number");
    }

    if(!lua_isinteger(L, -4))
    {
        // type
        Log_Error("L_TilemapAddLayer, arg 2 should be a TileMapLayerType enum");
    }

    if(!lua_islightuserdata(L, -5))
    {
        // pTilemap
        Log_Error("L_TilemapAddLayer, arg 1 should be a light user data");
    }

    enum ObjectLayer2DDrawOrder drawOrder = (enum ObjectLayer2DDrawOrder)lua_tointeger(L, -1);
    vec2 pos;
    pos[0] = lua_tonumber(L, -3);
    pos[1] = lua_tonumber(L, -2);
    enum TileMapLayerType type = (enum TileMapLayerType)lua_tointeger(L, -4);
    struct TileMap* pTileMap = (struct TileMap*)lua_topointer(L, -5);
    struct TileMapLayer* pNewLayer = TilemapAddLayer(pTileMap, type, pos, drawOrder);
    lua_pushlightuserdata(L, pNewLayer);
    return 1;
}

static int L_TilemapSetTile(lua_State* L)
{
    /* struct TileMap* pTileMap, TileIndex tile, int layer, int x, int y */
    if(!lua_isinteger(L, -1))
    {
        // y pos
        Log_Error("L_TilemapSetTile, arg 5 should be a y pos integer");
    }
    if(!lua_isinteger(L, -2))
    {
        // x pos
        Log_Error("L_TilemapSetTile, arg 4 should be an x pos integer");
    }
    if(!lua_isinteger(L, -3))
    {
        // layer
        Log_Error("L_TilemapSetTile, arg 3 should be a layer index");
    }
    if(!lua_isinteger(L, -4))
    {
        // tile index
        Log_Error("L_TilemapSetTile, arg 2 should be a tile index");
    }
    if(!lua_islightuserdata(L, -5))
    {
        // pTileMap
        Log_Error("L_TilemapSetTile, arg 1 should be a light user data");
    }

    int y = lua_tointeger(L, -1);
    int x = lua_tointeger(L, -2);
    int layer = lua_tointeger(L, -3);
    TileIndex tileIndex = (TileIndex)lua_tointeger(L, -4);
    struct TileMap* pTileMap = (struct TileMap*)lua_topointer(L, -5);
    TilemapSetTile(pTileMap, tileIndex, layer, x, y);
    return 0;
}

static int L_At_LookupNamedTile(lua_State* L)
{
    if(!lua_isstring(L, -1))
    {
        Log_Error("L_At_LookupNamedTile, arg 2 should be a string");
    }
    if(!lua_isinteger(L, -2))
    {
        Log_Error("L_At_LookupNamedTile, arg 1 should be an atlas handle");
    }
    const char* name = lua_tostring(L, -1);
    hAtlas atlas = (hAtlas)lua_tointeger(L, -2);
    TileIndex tile = At_LookupNamedTile(atlas, name);
    lua_pushinteger(L, tile);
    return 1;
}

static int L_AddRectangularStaticCollider(lua_State* L)
{
    // pEntities, x, y, w, h
    if(!lua_isnumber(L, -1))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 5 should be a float (h)");
    }
    if(!lua_isnumber(L, -2))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 4 should be a float (w)");
    }
    if(!lua_isnumber(L, -3))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 3 should be a float (y)");
    }
    if(!lua_isnumber(L, -4))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 2 should be a float (x)");
    }
    if(!lua_islightuserdata(L, -5))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 1 should be a float (entities)");
    }
    struct Entity2DCollection* pEntities = lua_topointer(L, -5);
    float x = lua_tonumber(L, -4);
    float y = lua_tonumber(L, -3);
    float w = lua_tonumber(L, -2);
    float h = lua_tonumber(L, -1);
    HEntity2D hEnt = Et2D_AddRectangularStaticColliderEntity(pEntities, x, y, w, h);
    lua_pushinteger(L, hEnt);
    return 1;
}

static int L_AddPlayerStartEntityAt(lua_State* L)
{
    // from, thislocation, bUsePrevLocationX, bUsePrevLocationY, struct Entity2DCollection* pEntities, float x, float y
    // returns HEntity
    if(!lua_isnumber(L, -1))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 7 should be a float (y position)");
    }
    if(!lua_isnumber(L, -2))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 6 should be a float (x position)");
    }
    if(!lua_islightuserdata(L, -3))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 5 should be a ptr (struct Entity2DCollection* pEntities)");
    }
    if(!lua_isboolean(L, -4))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 4 should be a bool (bUsePrevLocationY)");
    }
    if(!lua_isboolean(L, -5))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 3 should be a bool (bUsePrevLocationX)");
    }
    if(!lua_isstring(L, -6))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 2 should be a bool (thisLocation)");
    }
    if(!lua_isstring(L, -7))
    {
        Log_Error("L_AddPlayerStartEntityAt arg 1 should be a bool (from)");
    }



    float y = lua_tonumber(L, -1);
    float x = lua_tonumber(L, -2);
    struct Entity2DCollection* pEntities = lua_topointer(L, -3);

    struct WfPlayerStartData data = {
        .bUsePrevLocationX = lua_toboolean(L, -5),
        .bUsePrevLocationY = lua_toboolean(L, -4)
    };
    const char* from = lua_tostring(L, -7);
    const char* thisLocation = lua_tostring(L, -6);
    strcpy(data.from, from);
    strcpy(data.thisLocation, thisLocation);

    HEntity2D hEnt = WfAddPlayerStartEntityAt(pEntities, &data, x, y);
    lua_pushinteger(L, hEnt);
    return 1;
}

static int L_GetTileAtXY(lua_State* L)
{
    //TileIndex* WfGetTileAtXY(struct TileMapLayer* pLayer, int x, int y)
    if(!lua_isinteger(L, -1))
    {
        // y pos
        Log_Error("L_GetTileAtXY, arg 4 should be a y pos integer");
    }
    if(!lua_isinteger(L, -2))
    {
        // x pos
        Log_Error("L_GetTileAtXY, arg 3 should be an x pos integer");
    }
    if(!lua_islightuserdata(L, -3))
    {
        // pLayer
        Log_Error("L_GetTileAtXY, arg 2 should be a struct TileMap*");
    }
    if(!lua_isinteger(L, -4))
    {
        // pLayer
        Log_Error("L_GetTileAtXY, arg 1 should be an int, layer");
    }
    struct TileMap* pTileMap = lua_topointer(L, -3);
    int x = lua_tointeger(L, -2);
    int y = lua_tointeger(L, -1);
    int layer = lua_tointeger(L, -4);

    TileIndex* pi = WfGetTileAtXYIndex(pTileMap, layer, x, y);
    if(pi)
    {
        TileIndex t = *pi;
        lua_pushinteger(L, (int)t);
    }
    else
    {
        Log_Warning("L_GetTileAtXY, WfGetTileAtXY returned null ptr");
        lua_pushinteger(L, 0);
    }
    return 1;

}

static bool AnyNonzeroTile(struct TileMap* pTileMap, int x, int y)
{
    VECTOR(struct TileMapLayer) pLayers = pTileMap->layers;
    bool bAnyNonzero = false;
    for(int i = 0; i < VectorSize(pLayers); i++)
    {
        if(pLayers[i].bIsObjectLayer)
        {
            continue;
        }
        TileIndex* pTile = WfGetTileAtXY(&pLayers[i], x, y);
        if(pTile != NULL)
        {
            bAnyNonzero = *pTile != 0;
            if(bAnyNonzero)
                break;
        }
    }
    return bAnyNonzero;
}

static int L_AddMaskLayer(lua_State* L)
{
    /*
        Add a layer on the top of the layer stack that has a black tile
        at all positions where no tile is set on the layers below.
        The idea is this will block out any parts of sprites that clip outside the bounds of the rooms and corridors.
        All done in C for speed.
    */
    if(!lua_islightuserdata(L, -1))
    {
        // pLayer
        Log_Error("L_AddMaskLayer, arg 2 should be a struct TileMap*");
    }
    if(!lua_isinteger(L, -2))
    {
        Log_Error("L_At_LookupNamedTile, arg 1 should be an atlas handle");
    }

    struct TileMap* pTileMap = lua_topointer(L, -1);
    hAtlas atlas = lua_tointeger(L, -2);
    int biggestW = 0;
    int biggestH = 0;
    TileIndex blackTile = At_LookupNamedTile(atlas, "all_black");
    VECTOR(struct TileMapLayer) pLayers = pTileMap->layers;
    for(int i = 0; i < VectorSize(pLayers); i++)
    {
        if(pLayers[i].bIsObjectLayer)
        {
            continue;
        }
        if(pLayers[i].widthTiles > biggestW)
        {
            biggestW = pLayers[i].widthTiles;
        }
        if(pLayers[i].heightTiles > biggestH)
        {
            biggestH = pLayers[i].heightTiles;
        }
    }
    vec2 pos = {0,0};
    struct TileMapLayer* pMaskLayer = TilemapAddLayer(pTileMap, TilemapType_Tile, pos, DrawOrder_TopDown);
    int topLayer = VectorSize(pTileMap->layers) - 1;
    TileLayerResize(pMaskLayer, biggestW, biggestH);
    for(int i = 0; i < pMaskLayer->heightTiles; i++)
    {
        for(int j = 0; j < pMaskLayer->widthTiles; j++)
        {
            if(!AnyNonzeroTile(pTileMap, j, i))
            {
                TilemapSetTile(pTileMap, blackTile, topLayer, j, i);
            }
        }
    }
    return 0;
}

static int L_AddExitAt(lua_State* L)
{
    //HEntity2D WfAddExitAt(struct Entity2DCollection* pEntities, float x, float y, float w, float h, const char* toArea);
    if(!lua_isstring(L, -1))
    {
        Log_Error("L_AddMaskLayer, arg 6 should be a string (toArea)");
    }
    if(!lua_isnumber(L, -2))
    {
        Log_Error("L_AddMaskLayer, arg 5 should be a float (h)");
    }
    if(!lua_isnumber(L, -3))
    {
        Log_Error("L_AddMaskLayer, arg 4 should be a float (w)");
    }
    if(!lua_isnumber(L, -4))
    {
        Log_Error("L_AddMaskLayer, arg 3 should be a float (y)");
    }
    if(!lua_isnumber(L, -5))
    {
        Log_Error("L_AddMaskLayer, arg 2 should be a float (x)");
    }
    if(!lua_islightuserdata(L, -6))
    {
        Log_Error("L_AddMaskLayer, arg 1 should be a struct Entity2DCollection*");
    }

    const char* toArea = lua_tostring(L, -1);
    float x = lua_tonumber(L, -5);
    float y = lua_tonumber(L, -4);
    float w = lua_tonumber(L, -3);
    float h = lua_tonumber(L, -2);
    struct Entity2DCollection* pEntities = lua_topointer(L, -6);

    HEntity2D hEnt = WfAddExitAt(pEntities, x, y, w, h, toArea);
    lua_pushinteger(L, hEnt);
    return 1;
}

static int L_AddProceduralDungeonEntranceAt(lua_State* L)
{
    //HEntity2D WfAddProceduralDungeonEntranceAt(struct Entity2DCollection* pEntities, float x, float y, float w, float h, const char* genScript, const char* genFn)
    if(!lua_isstring(L, -1))
    {
        Log_Error("L_AddMaskLayer, arg 7 should be a string (genFn)");
    }

    if(!lua_isstring(L, -2))
    {
        Log_Error("L_AddMaskLayer, arg 6 should be a string (genScript)");
    }
    if(!lua_isnumber(L, -3))
    {
        Log_Error("L_AddMaskLayer, arg 5 should be a float (h)");
    }
    if(!lua_isnumber(L, -4))
    {
        Log_Error("L_AddMaskLayer, arg 4 should be a float (w)");
    }
    if(!lua_isnumber(L, -5))
    {
        Log_Error("L_AddMaskLayer, arg 3 should be a float (y)");
    }
    if(!lua_isnumber(L, -6))
    {
        Log_Error("L_AddMaskLayer, arg 2 should be a float (x)");
    }
    if(!lua_islightuserdata(L, -7))
    {
        Log_Error("L_AddMaskLayer, arg 1 should be a struct Entity2DCollection*");
    }

    const char* genFn = lua_tostring(L, -1);
    const char* genScript = lua_tostring(L, -2);
    float x = lua_tonumber(L, -6);
    float y = lua_tonumber(L, -5);
    float w = lua_tonumber(L, -4);
    float h = lua_tonumber(L, -3);
    struct Entity2DCollection* pEntities = lua_topointer(L, -7);

    HEntity2D hEnt = WfAddProceduralDungeonEntranceAt(pEntities, x, y, w, h, genScript, genFn);
    lua_pushinteger(L, hEnt);
    return 1;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Register Lua Wrappers


void WfRegisterMapGenLuaFunctions()
{
    Sc_RegisterCFunction("TilemapAddLayer", &L_TilemapAddLayer);
    Sc_RegisterCFunction("TilemapSetTile", &L_TilemapSetTile);
    Sc_RegisterCFunction("LookupNamedTile", &L_At_LookupNamedTile);
    Sc_RegisterCFunction("AddPlayerStartEntityAt", &L_AddPlayerStartEntityAt);
    Sc_RegisterCFunction("GetTileAtXY", &L_GetTileAtXY);
    Sc_RegisterCFunction("AddRectangularStaticCollider", &L_AddRectangularStaticCollider);
    Sc_RegisterCFunction("AddMaskLayer", &L_AddMaskLayer);
    Sc_RegisterCFunction("AddExitAt", &L_AddExitAt);
    Sc_RegisterCFunction("AddProceduralDungeonEntranceAt", &L_AddProceduralDungeonEntranceAt);
}
