#ifndef WFBASICFISHINGROD_H
#define WFBASICFISHINGROD_H

#include "WfItem.h"
#include "SharedLib.h"

struct Entity2D;
struct GameFrameworkLayer;

void STARDEW_API WfBasicFishingRodOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicFishingRodOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicFishingRodOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);


#endif