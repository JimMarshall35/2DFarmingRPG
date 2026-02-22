#ifndef WFITEMHELPERS_H
#define WFITEMHELPERS_H

#include "TimerPool.h"
#include <cglm/cglm.h>

#define VECTOR(a) a*

struct Entity2D;
struct GameFrameworkLayer;
struct GameLayer2DData;
enum WfDirection;

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

#endif
