#ifndef WFITEMHELPERS_H
#define WFITEMHELPERS_H

#include "TimerPool.h"
#include <cglm/cglm.h>
#include "IntTypes.h"
#include "HandleDefs.h"
#include "WfEntityMessages.h"

#define VECTOR(a) a*

struct Entity2D;
struct GameFrameworkLayer;
struct GameLayer2DData;
enum WfDirection;
struct WfPlayerEntData;
struct TileMap;
struct TileMapLayer;
struct WfItemDef;
enum WfActionAnimation;

struct WfSearchFan
{
    vec2 base;
    vec2 direction;
    float widthRadians;
    float length;
};

enum WfEntitySearchType
{
    WfDynamicEntities = 1,
    WfStaticEntities = 2
};

void WfInitItemHelpers();

HTimer WfScheduleCallbackOnAnimation(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, 
    TimerCallbackFn callback,
    float percentageIntoAnimation,
    char* anim_name, 
    void* pCallbackUserData);

bool WfGetEntityGroundContactPoint(struct Entity2D* pEnt, vec2 outPoint);

VECTOR(struct Entity2D*) WfFindEntitiesWithinFan(
    struct WfSearchFan* pFan, 
    struct GameFrameworkLayer* pLayer, 
    struct GameLayer2DData* pData, 
    enum WfEntitySearchType searchType,
    VECTOR(struct Entity2D*) pEntityListOut);

void WfGetDirectionVector(enum WfDirection dir, vec2 outVec);

struct Entity2D* WfFindClosestEntity(vec2 pointClosestTo, VECTOR(struct Entity2D*) pEnt);

TileIndex* WfGetTileInFrontOfPlayer(
    struct GameLayer2DData* pData, 
    struct WfPlayerEntData* pPlayerData, 
    float distanceInFront, 
    HEntity2D hEntPlayer, 
    int tileLayer,
    int* pXOut,
    int* pYOut
);

/// @brief 
/// @param tileX 
/// @param tileY 
/// @param tilemap 
/// @param layer 
/// @param indices An array of indicies that count as an occupied tile
/// @param numIndices 
/// @return 
u8 WfGetTerrainLUTIndex(int tileX, int tileY, struct TileMap* tilemap, int layer, TileIndex* indices, int numIndices);


TileIndex* WfGetTileAtXY(struct TileMapLayer* pLayer, int x, int y);

/// @brief how to calculate damage
enum WfDamageCalculationType
{
    /// @brief always a constant amount of damage dealt
    DCT_Constant,

    /// @brief call a callback to determine damage dealt
    DCT_Callback
};

/// @brief callback to calculate damage
typedef float(*WfCalculateDamageFn)(struct Entity2D* dealer, struct Entity2D* recipient, struct WfItemDef* itemDef);

struct WfDamagingWeaponDef
{
    /// @brief The length of the fan that captures entities to damage
    float fanLength;

    /// @brief The width of the fan that captures entities to damage in radians
    float fanWidth;

    /// @brief pointer to the item defintion (optional, needed for callback damage type)
    struct WfItemDef* pItemDef;

    /// @brief 
    enum WfDamageType damageType;

    /// @brief What animation does the damaging weapon use
    enum WfActionAnimation animation;
    
    /// @brief how is the damage calculated;
    enum WfDamageCalculationType damageCalculationType;

    union 
    {
        float damageConst;
        WfCalculateDamageFn damageCallback;
    } damageCalcData;
};

bool WfOnUseDamagingWeaponItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, struct WfDamagingWeaponDef* pDef);

#endif
