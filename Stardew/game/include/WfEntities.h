#ifndef WFENTITIES_H
#define WFENTITIES_H
#include "Entities.h"

enum WfEntityTypes
{
    WfEntityType_PlayerStart = EBET_Last,
    WfEntityType_Exit,
    WfEntityType_WoodedArea,
    WfEntityType_DebrisField,
    WfEntityType_Tree,
    WfEntityType_Rock,
    WfEntityType_Player,
    WfEntityType_ItemPickup,
};

void WfRegisterEntityTypes();


#endif