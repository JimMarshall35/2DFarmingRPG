#ifndef WFHEALTHCOMPONENT_H
#define WFHEALTHCOMPONENT_H

#include "IntTypes.h"
#include "WfEntityMessages.h"

struct Entity2D;
struct WfHealthComponent;

typedef void(*OnDamageCallbackFn)(struct Entity2D* pEnt, struct WfHealthComponent* pComponent, struct WfDamageMsg* pDamageMsg, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer);
typedef void(*OnKillCallbackFn)(struct Entity2D* pEnt, struct WfHealthComponent* pComponent, struct WfDamageMsg* pDamageMsg, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer);
typedef float(*DamageModifierFn)(struct Entity2D* pEnt, struct WfHealthComponent* pComponent, struct WfDamageMsg* pDamageMsg, struct Entity2D* pSender, struct GameFrameworkLayer* pLayer);

struct WfHealthComponent
{
    float health;
    OnDamageCallbackFn onDamageCallback;
    OnKillCallbackFn onKill;
    DamageModifierFn damageModifier;
};


void WfAddHealthComponent(struct WfHealthComponent* pComponent, struct Entity2D* pEnt);

#endif