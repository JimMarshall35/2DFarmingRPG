#include "WfItemHelpers.h"
#include "GameFramework.h"
#include "Game2DLayer.h"
#include <cglm/cglm.h>
#include "Entities.h"
#include "WfEntities.h"
#include <stdbool.h>
#include "AssertLib.h"
#include "WfTree.h"
#include "WfPlayer.h"
#include "Geometry.h"
#include "EntityQuadTree.h"

bool WfGetEntityGroundContactPoint(struct Entity2D* pEnt, vec2 outPoint)
{
    switch (pEnt->type)
    {
    case WfEntityType_Exit:
        // not implemented for this kind
        return false;
        break;
    case WfEntityType_WoodedArea:
    case WfEntityType_DebrisField:
        EASSERT("WfEntityType_WoodedArea and WfEntityType_DebrisField are transient and shouldn't be passed to this function normally" && false);
        return false;
    case WfEntityType_Tree:
        WfTreeGetGroundContactPoint(pEnt, outPoint);
        return true;
    case WfEntityType_Rock:
        EASSERT("Not implemented yet but will need to be so assert(false) as a reminder" && false);
        return false;
    case WfEntityType_Log:
        EASSERT("Not implemented yet but will need to be so assert(false) as a reminder" && false);
        return false;
    case WfEntityType_Player:
        WfPlayerGetGroundContactPoint(pEnt, outPoint);
        return true;
    }
    return false;
}

HTimer WfScheduleCallbackOnAnimation(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, 
    TimerCallbackFn callback,
    float percentageIntoAnimation,
    char* anim_name, 
    void* pCallbackUserData)
{
    /* delay the actual damage by a certain amount of time based on animation length */
    struct GameLayer2DData* pData = pLayer->userData;
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    struct AtlasAnimation* pCurrentAnim = At_FindAnim(pData->hAtlas, anim_name);
    float animLen = (float)VectorSize(pCurrentAnim->frames) / pCurrentAnim->fps;

    struct SDTimer timerDef = {
        .bActive = true,
        .bRepeat = false,
        .bAutoReset = false,
        .total = animLen * percentageIntoAnimation,
        .fnCallback = callback,
        .pUserData = pCallbackUserData,
        .elapsed = 0.0
    };
    HTimer timer = TP_GetTimer(&pData->timerPool, &timerDef);
    return timer;
}

VECTOR(struct Entity2D*) WfFindEntitiesWithinFan(
    struct WfSearchFan* pFan, 
    struct GameFrameworkLayer* pLayer, 
    struct GameLayer2DData* pData, 
    enum WfEntitySearchType searchType,
    VECTOR(struct Entity2D*) pEntityListOut)
{
    pEntityListOut = VectorClear(pEntityListOut);
    static VECTOR(HEntity2D) sFoundEnts = NULL;
	if(!sFoundEnts)
	{
		sFoundEnts = NEW_VECTOR(HEntity2D);
	}

    sFoundEnts = VectorClear(sFoundEnts);

    vec2 tl = {
        pFan->base[0] - pFan->length,
        pFan->base[1] - pFan->length,
    };
    vec2 br = {
        pFan->base[0] + pFan->length,
        pFan->base[1] + pFan->length,
    };

    if( searchType & WfStaticEntities)
    {
        sFoundEnts = Entity2DQuadTree_Query(pData->hEntitiesQuadTree, tl, br, sFoundEnts, &pData->entities, pLayer);
    }

    if(searchType & WfDynamicEntities)
    {
        sFoundEnts = Et2D_QueryDynEntities(pLayer, &pData->entities, tl, br, sFoundEnts);
    }

	int foundEnts = VectorSize(sFoundEnts);
    for(int i=0; i<foundEnts; i++)
    {
        HEntity2D hEnt = sFoundEnts[i];
        struct Entity2D* pEnt = Et2D_GetEntity(&pData->entities, hEnt);
        vec2 entityGroundPoint;
        bool bSuccess = WfGetEntityGroundContactPoint(pEnt, entityGroundPoint);
        if(!bSuccess)
        {
            continue;
        }
        float distance = Ge_DistanceBetweenPoints(pFan->base, entityGroundPoint);
        if(distance > pFan->length)
        {
            continue;
        }
        vec2 base2Ent;
        glm_vec2_sub(entityGroundPoint, pFan->base, base2Ent);
        float angle = Ge_AngleBetweenVec2s(pFan->direction, base2Ent);
        if(angle <= pFan->widthRadians)
        {
            pEntityListOut = VectorPush(pEntityListOut, &pEnt);
        }

    }
    return pEntityListOut;
}

void WfGetDirectionVector(enum WfDirection dir, vec2 outVec)
{
    static const vec2 dirs[NumDirections] = {
        { 0.0f,-1.0f}, // up
        { 0.0f, 1.0f}, // down
        {-1.0f, 0.0f}, // left
        { 1.0f, 0.0f}, // right
    };
    outVec[0] = dirs[dir][0];
    outVec[1] = dirs[dir][1];
}

struct Entity2D* WfFindClosestEntity(vec2 pointClosestTo, VECTOR(struct Entity2D*) pEnt)
{
    float distance = FLT_MAX;
    struct Entity2D* pRet = NULL;
    for(int i=0; i<VectorSize(pEnt); i++)
    {
        vec2 g;
        WfGetEntityGroundContactPoint(pEnt[i], g);
        float d = Ge_DistanceBetweenPoints(g, pointClosestTo);
        if(d < distance)
        {
            pRet = pEnt[i];
            distance = d;
        }
    }
    return pRet;
}

TileIndex* WfGetTileAtXY(struct TileMapLayer* pLayer, int x, int y)
{
    return pLayer->Tiles + (y * pLayer->widthTiles + x);
}

TileIndex* WfGetTileAtPoint(vec2 pt, struct GameLayer2DData* pData, int tileLayer, int* pXOut, int* pYOut)
{
    int iX = ((int)pt[0] / (int)pData->tilemap.layers[tileLayer].tileWidthPx);
    int iY = ((int)pt[1] / (int)pData->tilemap.layers[tileLayer].tileHeightPx);
    struct TileMapLayer* pLayer = &pData->tilemap.layers[tileLayer];
    TileIndex* pIndex = WfGetTileAtXY(pLayer, iX, iY);
    *pXOut = iX;
    *pYOut = iY;
    return pIndex;
}

TileIndex* WfGetTileInFrontOfPlayer(struct GameLayer2DData* pData, struct WfPlayerEntData* pPlayerData, float distanceInFront, HEntity2D hEntPlayer, int tileLayer, int* pXOut, int* pYOut)
{
    vec2 facingVec, pt;
    WfGetDirectionVector(pPlayerData->directionFacing, facingVec);
    vec2 playerGroundPos;
    struct Entity2D* pPlayerEnt = Et2D_GetEntity(&pData->entities, hEntPlayer);
    WfPlayerGetGroundContactPoint(pPlayerEnt, playerGroundPos);
    glm_vec2_scale(facingVec, distanceInFront, facingVec);
    glm_vec2_add(playerGroundPos, facingVec, pt);

    return WfGetTileAtPoint(pt, pData, tileLayer, pXOut, pYOut);
}

static bool IsIndexInSet(TileIndex index, TileIndex* indices, int numIndices)
{
    for(int i=0; i<numIndices; i++)
    {
        if(indices[i] == index)
        {
            return true;
        }
    }
    return false;
}

u8 WfGetTerrainLUTIndex(int tileX, int tileY, struct TileMap* tilemap, int layer, TileIndex* indices, int numIndices)
{
    u8 r = 0;
    struct TileMapLayer* pLayer = &tilemap->layers[layer];
    TileIndex indices_surrounding[8] = {
        *(WfGetTileAtXY(pLayer, tileX-1, tileY-1)),
        *(WfGetTileAtXY(pLayer, tileX,   tileY-1)),
        *(WfGetTileAtXY(pLayer, tileX+1, tileY-1)),

        *(WfGetTileAtXY(pLayer, tileX-1, tileY)),
        *(WfGetTileAtXY(pLayer, tileX+1, tileY)),

        *(WfGetTileAtXY(pLayer, tileX-1, tileY+1)),
        *(WfGetTileAtXY(pLayer, tileX,   tileY+1)),
        *(WfGetTileAtXY(pLayer, tileX+1, tileY+1)),
    };
    for(int i=0; i<8; i++)
    {
        if(IsIndexInSet(indices_surrounding[i], indices, numIndices))
        {
            r |= (1 << i);
        }
    }
    return r;
}
