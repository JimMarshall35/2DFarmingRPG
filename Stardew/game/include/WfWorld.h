#ifndef WFWORLD_H
#define WFWORLD_H
#include "WfEnums.h"
#include "StringKeyHashMap.h"
#include <stdbool.h>

#define MAX_LOCATION_NAME_LEN 128


struct WfLocation
{
    char levelFilePath[256];
    bool bIsInterior;
};

struct DrawContext;
typedef struct DrawContext DrawContext;

void WfWorld_AddLocation(const struct WfLocation* pLocation, const char* locationName);

void WfWorld_LoadLocation(const char* locationName, DrawContext* pDC);

const char* WfWorld_GetCurrentLocationName();

void WfWorld_SetCurrentLocationName(const char* name);

void WfWorldInit();

void WfWorld_ClearLocations();

#endif