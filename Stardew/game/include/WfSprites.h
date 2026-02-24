#ifndef WFSPRITES_H
#define WFSPRITES_H

#include "HandleDefs.h"
#include "WfEnums.h"

struct GameLayer2DData;

struct WfTreeSprites
{
    hSprite coniferousTop1;
    hSprite coniferousTop2;
    hSprite deciduousTop1;
    hSprite deciduousTop2;
    hSprite trunk1;
    hSprite trunk2;

    hSprite stump1;
    hSprite stump22;

};

struct WfSprites
{
    struct WfTreeSprites treeSpritesPerSeason[NumSeasons];
};

/* Populate the WfSprites struct with the sprites from the GameLayer2DData's atlas */
void WfInitSprites(struct WfSprites* pSprites, struct GameLayer2DData* pLayerData);


#endif