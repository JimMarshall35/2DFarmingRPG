#ifndef WFGAMELAYERDATA_H
#define WFGAMELAYERDATA_H

#include "WfSprites.h"
#include <cglm/cglm.h>

struct GameLayer2DData;

/*
    Custom per Game2DLayer data used by the game
    - Sprites
*/
struct WfGameLayerData
{
    struct WfSprites sprites;
    struct GameFrameworkEventListener* HUDPushedEventListener;
    struct GameFrameworkEventListener* SettingsPushedEventListener;
};

void WfInitGameLayerData(struct GameLayer2DData* pGameLayerData, struct WfGameLayerData* pOutData);

#endif