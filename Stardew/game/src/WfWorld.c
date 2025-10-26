#include "WfWorld.h"
#include "WfGameLayer.h"
#include "AssertLib.h"
#include "GameFramework.h"
#include <string.h>

struct WfWorld
{
    struct HashMap locationsHashMap;
    char currentLocation[MAX_LOCATION_NAME_LEN];
};

static struct WfWorld gWorld;

void WfWorldInit()
{
    HashmapInit(&gWorld.locationsHashMap, 32, sizeof(struct WfLocation));
}

void WfWorld_AddLocation(const struct WfLocation* pLocation, const char* locationName)
{
    HashmapInsert(&gWorld.locationsHashMap, locationName, pLocation);
}

void WfWorld_LoadLocation(const char* locationName, DrawContext* pDC)
{
    struct WfLocation* pLocation = HashmapSearch(&gWorld.locationsHashMap, locationName);
    if(pLocation)
    {
        // GF_PopGameFrameworkLayer();
        // GF_PopGameFrameworkLayer(); // pop the old game layer
        WfPushGameLayer(pDC, pLocation->levelFilePath);
    }
}

const char* WfWorld_GetCurrentLocationName()
{
    return gWorld.currentLocation;
}

void WfWorld_SetCurrentLocationName(const char* name)
{
    EASSERT(strlen(name) < MAX_LOCATION_NAME_LEN);
    strcpy(gWorld.currentLocation, name);
}

void WfWorld_ClearLocations()
{
    // HashmapDeleteItem()
    struct HashmapKeyIterator itr = GetKeyIterator(&gWorld.locationsHashMap);
    char* key = NULL;
    VECTOR(char*) keys = NEW_VECTOR(char*);
    keys = VectorResize(keys, 10);
    keys = VectorClear(keys);
    while(key = NextHashmapKey(&itr))
    {
        char* cpy = malloc(strlen(key) + 1);
        strcpy(cpy, key);
        keys = VectorPush(keys, &cpy);
    }
    for(int i=0; i< VectorSize(keys); i++)
    {
        HashmapDeleteItem(&gWorld.locationsHashMap, keys[i]);
        free(keys[i]);
    }
    DestoryVector(keys);
    WfWorld_SetCurrentLocationName("UNINITIALIZED");
}
