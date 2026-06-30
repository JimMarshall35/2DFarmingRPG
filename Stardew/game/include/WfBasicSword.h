#ifndef WFBASICSWORD_H
#define WFBASICSWORD_H
#include <stdbool.h>
#include "SharedLib.h"
#include "WfItem.h"

struct GameFrameworkLayer;
struct Entity2D;

bool STARDEW_API WfBasicSwordOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicSwordOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicSwordOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);

#endif
