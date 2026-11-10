#include <string.h>
#include "components/WfHealthComponent.h"
#include "components/WfComponents.h"
#include "Entities.h"
#include "Entity2DCollection.h"
#include "WfEntityMessages.h"
#include "GameComponents.h"

/*
typedef bool (*AcceptsMsgFn)(struct Component2D* pComponent, int msgType, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer);
typedef bool (*ProcessMsgFn)(struct Component2D* pComponent, struct EntityToEntityMessage* pMsg, struct Entity2D* pRecipient, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer);

*/


static bool AcceptsMsg(struct Component2D* pComponent, int msgType, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer) 
{
    return msgType == E2EM_Damage;
}

static bool ProcessMsg(struct Component2D* pComponent, struct EntityToEntityMessage* pMsg, struct Entity2D* pRecipient, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer)
{
    struct WfDamageMsg* pDamage =  WfGetDamageMessage(pMsg);
    float damage = pDamage->damage;
    if(pComponent->data.health.damageModifier)
    {
        damage = pComponent->data.health.damageModifier(pRecipient, pComponent, pDamage, pSender, pLayer);
    }
    
    if(damage > 1e-5)
    {
        pComponent->data.health.health -= damage;
        if(pComponent->data.health.onDamageCallback)
        {
            pComponent->data.health.onDamageCallback(
                pRecipient, &pComponent->data.health, pDamage, pSender, pLayer
            );
        }
        if(pComponent->data.health.health <= 0.0f)
        {
            if(pComponent->data.health.onKill)
            {
                pComponent->data.health.onKill(
                    pRecipient, &pComponent->data.health, pDamage, pSender, pLayer
                );
            }
        }
    }
}

void WfAddHealthComponent(struct WfHealthComponent* pComponent, struct Entity2D* pEnt)
{
    struct Component2D* pNew = &pEnt->components[pEnt->numComponents++];
    pNew->acceptsMsg = &AcceptsMsg;
    pNew->processMsg = &ProcessMsg;
    pNew->type = GCT_HealthComponent;
    memcpy(&pNew->data.health, pComponent, sizeof(struct WfHealthComponent));
}