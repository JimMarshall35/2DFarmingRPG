#ifndef WFGAMELAYERDATA_H
#define WFGAMELAYERDATA_H

#include "WfSprites.h"

struct GameLayer2DData;

/*
    Custom per Game2DLayer data used by the game
    - Sprites
*/
struct WfGameLayerData
{
    struct WfSprites sprites;
    struct GameFrameworkEventListener* HUDPushedEventListener;
};

void WfInitGameLayerData(struct GameLayer2DData* pGameLayerData, struct WfGameLayerData* pOutData);

#endif