#include "WfPlayer.h"
#include "BinarySerializer.h"
#include "Game2DLayer.h"
#include "Entities.h"
#include "ObjectPool.h"
#include "DrawContext.h"
#include "InputContext.h"
#include "GameFramework.h"
#include "AnimatedSprite.h"
#include "Camera2D.h"
#include "WfEntities.h"
#include <string.h>

#define WALKING_UP_MALE "walk-base-male-up"
#define WALKING_DOWN_MALE "walk-base-male-down"
#define WALKING_LEFT_MALE "walk-base-male-left"
#define WALKING_RIGHT_MALE "walk-base-male-right"

#define WALKING_UP_FEMALE "walk-base-female-up"
#define WALKING_DOWN_FEMALE "walk-base-female-down"
#define WALKING_LEFT_FEMALE "walk-base-female-left"
#define WALKING_RIGHT_FEMALE "walk-base-female-right"

#define PLAYER_SPRITE_COMPONENT_INDEX 0
#define PLAYER_GROUND_COLLIDER_COMPONENT_INDEX 1

#define PLAYER_SPRITE_COMP_INDEX 0
#define PLAYER_COLLIDER_COMP_INDEX 1

struct WfPlayerEntData
{
    vec2 groundColliderCenter2EntTransform;
    struct ButtonBinding moveUpBinding;
    struct ButtonBinding moveDownBinding;
    struct ButtonBinding moveLeftBinding;
    struct ButtonBinding moveRightBinding;
    struct ActiveInputBindingsMask playerControlsMask;
    /* value I set this to is NOT meters per second, TODO: fix */
    float metersPerSecondWalkSpeedBase;

    float speedMultiplier;

    vec2 movementVector;

    /* flags section */
    u32 bMovingThisFrame : 1;
    u32 bMovingLastFrame : 1;
};

static OBJECT_POOL(struct WfPlayerEntData) gPlayerEntDataPool = NULL;

void WfInitPlayer()
{
    gPlayerEntDataPool = NEW_OBJECT_POOL(struct WfPlayerEntData, 4);
}

static void OnInitPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    Entity2DOnInit(pEnt,pLayer, pDrawCtx, pInputCtx);
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pPlayerEntData->movementVector[0] = 0.0f;
    pPlayerEntData->movementVector[1] = 0.0f;
    pPlayerEntData->moveUpBinding    = In_FindButtonMapping(pInputCtx, "playerMoveUp");
    pPlayerEntData->moveDownBinding  = In_FindButtonMapping(pInputCtx, "playerMoveDown");
    pPlayerEntData->moveLeftBinding  = In_FindButtonMapping(pInputCtx, "playerMoveLeft");
    pPlayerEntData->moveRightBinding = In_FindButtonMapping(pInputCtx, "playerMoveRight");
    In_ActivateButtonBinding(pPlayerEntData->moveUpBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveDownBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveLeftBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveRightBinding, &pPlayerEntData->playerControlsMask);
    In_SetMask(&pPlayerEntData->playerControlsMask, pInputCtx);
    pPlayerEntData->bMovingThisFrame = false;
    pPlayerEntData->metersPerSecondWalkSpeedBase = 100.0f;
    pPlayerEntData->speedMultiplier = 3.0f;
    ClampCameraToTileLayer(pLayer->userData, 0);
}

static void OnDestroyPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gPlayerEntDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static void SetPlayerAnimation(struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
    pSprite->bIsAnimating = pPlayerEntData->bMovingThisFrame;
    if(!pPlayerEntData->bMovingThisFrame && pPlayerEntData->bMovingLastFrame)
    {
        pSprite->onSprite = 0;
    }
    if(pPlayerEntData->movementVector[1] > 1e-5f)
    {
        // moving down
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_DOWN_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
    }
    else if(pPlayerEntData->movementVector[1] < -1e-5f)
    {
        // moving up
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_UP_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
    }
    else if(pPlayerEntData->movementVector[0] > 1e-5f)
    {
        // moving right
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_RIGHT_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
    }
    else if(pPlayerEntData->movementVector[0] < -1e-5f)
    {
        // moving left
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_LEFT_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
    }
}

static void OnUpdatePlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    vec2 scaledMovement;
    scaledMovement[0] = pPlayerEntData->movementVector[0] * pPlayerEntData->metersPerSecondWalkSpeedBase * deltaT * pPlayerEntData->speedMultiplier;
    scaledMovement[1] = pPlayerEntData->movementVector[1] * pPlayerEntData->metersPerSecondWalkSpeedBase * deltaT * pPlayerEntData->speedMultiplier;
    Ph_SetDynamicBodyVelocity(
        pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider.id,
        scaledMovement
    );
    SetPlayerAnimation(pLayer, pPlayerEntData, pEnt);
    Entity2DUpdate(pEnt, pLayer, deltaT);
}

static void OnInputPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    Entity2DInput(pEnt, pLayer, context);
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pPlayerEntData->movementVector[0] = 0.0f;
    pPlayerEntData->movementVector[1] = 0.0f;
    pPlayerEntData->bMovingLastFrame = pPlayerEntData->bMovingThisFrame;
    pPlayerEntData->bMovingThisFrame = false;
    if(In_GetButtonValue(context, pPlayerEntData->moveUpBinding))
    {
        vec2 add = {0.0f, -1.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveDownBinding))
    {
        vec2 add = {0.0f, 1.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);   
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveLeftBinding))
    {
        vec2 add = {-1.0f, 0.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);   
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveRightBinding))
    {
        vec2 add = {1.0f, 0.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);   
        pPlayerEntData->bMovingThisFrame = true;
    }
    glm_vec2_normalize(pPlayerEntData->movementVector);
}

void WfDeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
}

void WfSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
}

float WfGetPlayerSortPosition(struct Entity2D* pEnt)
{
    return pEnt->transform.position[1] - gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform[1];
}

void WfPlayerPostPhys(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    struct GameLayer2DData* pLayerData = pLayer->userData;
    Entity2DUpdatePostPhysics(pEnt, pLayer, deltaT);
    vec2 pixelsPos, physPos;
    struct DynamicCollider* pCollider = &pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider;
    
    Ph_GetDymaicBodyPosition(pCollider->id, physPos);
    Ph_PhysicsCoords2PixelCoords(pLayerData->hPhysicsWorld, physPos, pixelsPos);
    glm_vec2_add(pixelsPos, pPlayerEntData->groundColliderCenter2EntTransform, pEnt->transform.position);

    CenterCameraAt(pixelsPos[0], pixelsPos[1], &pLayerData->camera, pLayerData->windowW, pLayerData->windowH);
}

void WfMakeIntoPlayerEntity(struct Entity2D* pEnt, struct GameLayer2DData* pData, vec2 spawnAtGroundPos)
{
    pEnt->nextSibling = NULL_HANDLE;
    pEnt->previousSibling = NULL_HANDLE;
    gPlayerEntDataPool = GetObjectPoolIndex(gPlayerEntDataPool, &pEnt->user.hData);
    gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform[0] = -32;
    gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform[1] = -60;
    pEnt->numComponents = 0;
    pEnt->type = WfEntityType_Player;
    /*
        Animated Sprite
    */
    struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
    pComponent1->type = ETE_SpriteAnimator;
    struct AnimatedSprite* pAnimatedSprite = &pComponent1->data.spriteAnimator;
    memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));
    pAnimatedSprite->animationName = WALKING_DOWN_MALE;
    pAnimatedSprite->bRepeat = true;
    pAnimatedSprite->transform.scale[0] = 1.0f;
    pAnimatedSprite->transform.scale[1] = 1.0f;
    pAnimatedSprite->bIsAnimating = false;

    /*
        Ground Collider
    */
    struct Component2D* pComponent2 = &pEnt->components[pEnt->numComponents++];
    pComponent2->type = ETE_DynamicCollider;
    pComponent2->data.dynamicCollider.shape.type = PBT_Circle;
    pComponent2->data.dynamicCollider.shape.data.circle.center[0] = spawnAtGroundPos[0];
    pComponent2->data.dynamicCollider.shape.data.circle.center[1] = spawnAtGroundPos[1];
    pComponent2->data.dynamicCollider.shape.data.circle.radius = 10;
    pComponent2->data.dynamicCollider.bIsSensor = false;
    pComponent2->data.dynamicCollider.bGenerateSensorEvents = true;
    pComponent2->data.dynamicCollider.onSensorOverlapBegin = NULL;
    pComponent2->data.dynamicCollider.onSensorOverlapEnd = NULL;

    glm_vec2_add(spawnAtGroundPos, gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform, pEnt->transform.position);
    pEnt->transform.scale[0] = 1.0;
    pEnt->transform.scale[1] = 1.0;
    

    pEnt->inDrawLayer = 0;

    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->init = &OnInitPlayer;
    pEnt->update = &OnUpdatePlayer;
    pEnt->getSortPos = &WfGetPlayerSortPosition;
    pEnt->postPhys = &WfPlayerPostPhys;
    pEnt->input = &OnInputPlayer;
    pEnt->bKeepInQuadtree = false;
    pEnt->bKeepInDynamicList = true;
    pEnt->bSerialize = false;
}
