#ifndef WFENTITY_MSGS
#define WFENTITY_MSGS
#include "WfItem.h"
#include "HandleDefs.h"

struct EntityToEntityMessage;

enum WfDamageType
{
    WfAxeDamage,
    WfPickaxeDamage,
    WfWeaponDamage,
};

extern char* WfDamageTypeNameLUT[3];

struct WfDamageMsg
{
    enum WfDamageType type;
    float damage;
};

void WfInitEntityMessages();

HGeneric WfAllocateDamageMessageData(struct WfDamageMsg** pOutDamageMsgPtr);

void WfFreeDamageMsg(HGeneric hMsg);

void WfDamageMsgFreer(struct EntityToEntityMessage* pMsg);

struct WfDamageMsg* WfGetDamageMessage(struct EntityToEntityMessage* pMsg);

#endif
