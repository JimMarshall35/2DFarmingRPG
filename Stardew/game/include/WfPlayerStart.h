#ifndef WFPLAYER_START_H
#define WFPLAYER_START_H

#include "HandleDefs.h"
#include <stdbool.h>
struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

struct WfPlayerStartData
{
    char from[64];
    char thisLocation[64];
    bool bUsePrevLocationX;
    bool bUsePrevLocationY;
};

void WfDeSerializePlayerStartEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializePlayerStartEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

void WfInitPlayerStart();

HEntity2D WfGetCurrentLocalPlayer();

void WfAddPlayerStartEntityAt(struct WfPlayerStartData* pData, float x, float y);

#endif