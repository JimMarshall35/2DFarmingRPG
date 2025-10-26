#include "Components.h"
#include "Entities.h"
#include "AssertLib.h"
#include "GameFramework.h"
#include "InputContext.h"
#include "Game2DLayer.h"
#include "Sprite.h"
#include "AnimatedSprite.h"

void Co_InitComponents(struct Entity2D* entity, struct GameFrameworkLayer* pLayer)
{
    struct GameLayer2DData* pGameLayerData = pLayer->userData;
    for(int i=0; i<entity->numComponents; i++)
    {
        switch(entity->components[i].type)
        {
        case ETE_Sprite:
            break;
        case ETE_StaticCollider:
            entity->components[i].data.staticCollider.id = Ph_GetStaticBody2D(
                pGameLayerData->hPhysicsWorld,
                &entity->components[i].data.staticCollider.shape, 
                &entity->transform, 
                entity->thisEntity,
                entity->components[i].data.staticCollider.bIsSensor,
                i,
                entity->components[i].data.staticCollider.bGenerateSensorEvents
            );
            break;
        case ETE_DynamicCollider:
            entity->components[i].data.dynamicCollider.id = Ph_GetDynamicBody(
                pGameLayerData->hPhysicsWorld, 
                &entity->components[i].data.dynamicCollider.shape, 
                &entity->components[i].data.dynamicCollider.options, 
                &entity->transform, 
                entity->thisEntity,
                entity->components[i].data.dynamicCollider.bIsSensor,
                i,
                entity->components[i].data.dynamicCollider.bGenerateSensorEvents
            );
            break;
        case ETE_TextSprite:
            break;
        case ETE_SpriteAnimator:
            AnimatedSprite_OnInit(&entity->components[i].data.spriteAnimator, entity, pLayer, 0.0f);
            break;
        default:
            EASSERT(false);
        }
    }
}

void Co_UpdateComponents(struct Entity2D* entity, struct GameFrameworkLayer* pLayer, float deltaT)
{
    for(int i=0; i<entity->numComponents; i++)
    {
        switch(entity->components[i].type)
        {
        case ETE_Sprite:
            break;
        case ETE_StaticCollider:
            break;
        case ETE_DynamicCollider:
            break;
        case ETE_TextSprite:
            break;
        case ETE_SpriteAnimator: AnimatedSprite_OnUpdate(&entity->components[i].data.spriteAnimator, entity, pLayer, deltaT);
            break;
        default:
            EASSERT(false);
        }
    }
}

void Co_Entity2DUpdatePostPhysicsFn(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{

}

void Co_InputComponents(struct Entity2D* entity, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    for(int i=0; i<entity->numComponents; i++)
    {
        switch(entity->components[i].type)
        {
        case ETE_Sprite:
            break;
        case ETE_StaticCollider:
            break;
        case ETE_DynamicCollider:
            break;
        case ETE_TextSprite:
            break;
        case ETE_SpriteAnimator:
            break;
        default:
            EASSERT(false);
        }
    }
}

void Co_DestroyComponents(struct Entity2D* entity)
{
    for(int i=0; i<entity->numComponents; i++)
    {
        switch(entity->components[i].type)
        {
        case ETE_Sprite:
            break;
        case ETE_StaticCollider:
            break;
        case ETE_DynamicCollider:
            break;
        case ETE_TextSprite:
            break;
        case ETE_SpriteAnimator:
            break;
        default:
            EASSERT(false);
        }
    }
}

void Co_DrawComponents(
    struct Entity2D* entity, 
    struct GameFrameworkLayer* pLayer,
    struct Transform2D* pCam,
    VECTOR(Worldspace2DVert)* outVerts,
    VECTOR(VertIndexT)* outIndices,
    VertIndexT* pNextIndex)
{
    for(int i=0; i<entity->numComponents; i++)
    {
        switch(entity->components[i].type)
        {
        case ETE_Sprite:
            SpriteComp_Draw(
                &entity->components[i].data.sprite,
                entity,
                pLayer,
                pCam,
                outVerts,
                outIndices,
                pNextIndex
            );
            break;
        case ETE_StaticCollider:
            break;
        case ETE_DynamicCollider:
            break;
        case ETE_TextSprite:
            break;
        case ETE_SpriteAnimator:
            AnimatedSprite_Draw(
                &entity->components[i].data.spriteAnimator,
                entity,
                pLayer,
                pCam,
                outVerts,
                outIndices,
                pNextIndex
            );
            break;
        default:
            EASSERT(false);
        }
    }
}