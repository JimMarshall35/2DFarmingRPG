#include "WfEntityMessages.h"
#include "ObjectPool.h"
#include "Entity2DCollection.h"
#include "AssertLib.h"
#include <stdlib.h>

static OBJECT_POOL(struct WfDamageMsg) gDamageMsgPool = NULL;

char* WfDamageTypeNameLUT[3] = {
    "Axe",
    "Pickaxe",
    "Weapon"
};


void WfInitEntityMessages()
{
    gDamageMsgPool = NEW_OBJECT_POOL(struct WfDamageMsg, 32);
}

HGeneric WfAllocateDamageMessageData(struct WfDamageMsg** pOutDamageMsgPtr)
{
    HGeneric h = NULL_HANDLE;
    gDamageMsgPool = GetObjectPoolIndex(gDamageMsgPool, &h);
    *pOutDamageMsgPtr = &gDamageMsgPool[h];
    return h;
}

void WfFreeDamageMsg(HGeneric hMsg)
{
    FreeObjectPoolIndex(gDamageMsgPool, hMsg);
}

void WfDamageMsgFreer(struct EntityToEntityMessage* pMsg)
{
    WfFreeDamageMsg(pMsg->data.hMsgData);
}

struct WfDamageMsg* WfGetDamageMessage(struct EntityToEntityMessage* pMsg)
{
    EASSERT(pMsg->type == E2EM_Damage);
    return &gDamageMsgPool[pMsg->data.hMsgData];
}
