#include "WfPlayer.h"
#include "BinarySerializer.h"
#include "Game2DLayer.h"
#include "Entities.h"
#include "ObjectPool.h"
#include "DrawContext.h"
#include "GameFramework.h"
#include "AnimatedSprite.h"
#include "Camera2D.h"
#include "WfEntities.h"
#include "WfUI.h"
#include "WfEnums.h"
#include "WfItem.h"
#include "AssertLib.h"
#include "WfPersistantGameData.h"
#include "GameFrameworkEvent.h"
#include "Scripting.h"
#include <string.h>
#include "Log.h"
#include "Network.h"
#include "Game2DLayerNetwork.h"
#include "Network.h"
#include "NetworkID.h"

#define WALKING_UP_MALE "walk-base-male-up"
#define WALKING_DOWN_MALE "walk-base-male-down"
#define WALKING_LEFT_MALE "walk-base-male-left"
#define WALKING_RIGHT_MALE "walk-base-male-right"

#define WALKING_UP_FEMALE "walk-base-female-up"
#define WALKING_DOWN_FEMALE "walk-base-female-down"
#define WALKING_LEFT_FEMALE "walk-base-female-left"
#define WALKING_RIGHT_FEMALE "walk-base-female-right"


static void WfMakeIntoPlayerEntityBase(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos, bool bNetworkControlled, int networkPlayerNum);

static OBJECT_POOL(struct WfPlayerEntData) gPlayerEntDataPool = NULL;


void WfInitPlayer()
{
    gPlayerEntDataPool = NEW_OBJECT_POOL(struct WfPlayerEntData, 4);
}

static void OnInitPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pPlayerEntData->bMovingLastFrame = false;
    pPlayerEntData->metersPerSecondWalkSpeedBase = 100.0f;
    pPlayerEntData->speedMultiplier = 3.0f;
    pPlayerEntData->movementBits = 0;
    
    if(pPlayerEntData->bNetworkControlled)
    {
        WfMakeIntoPlayerEntityBase(pEnt, pLayer, pPlayerEntData->createNetPlayerOnInitArgs.netPlayerSpawnAtPos, true, pPlayerEntData->createNetPlayerOnInitArgs.netPlayerSlot);
        Entity2DOnInit(pEnt,pLayer, pDrawCtx, pInputCtx);
        vec2 vel = {0,0};
        Ph_SetDynamicBodyVelocity(
            pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider.id,
            vel
        );
        return;
    }
    else if(NW_GetRole() != GR_Singleplayer)
    {
        /* serialize the local players entity in network game state updates */
        pEnt->bSerializeInNetworkUpdate = true;
    }

    Entity2DOnInit(pEnt,pLayer, pDrawCtx, pInputCtx);

    pPlayerEntData->bNetworkControlled = false;
    pPlayerEntData->networkPlayerNum = -1;

    pPlayerEntData->movementBits = 0;
    pPlayerEntData->moveUpBinding    = In_FindButtonMapping(pInputCtx, "playerMoveUp");
    pPlayerEntData->moveDownBinding  = In_FindButtonMapping(pInputCtx, "playerMoveDown");
    pPlayerEntData->moveLeftBinding  = In_FindButtonMapping(pInputCtx, "playerMoveLeft");
    pPlayerEntData->moveRightBinding = In_FindButtonMapping(pInputCtx, "playerMoveRight");

    pPlayerEntData->nextItemBinding = In_FindButtonMapping(pInputCtx, "nextItem");
    pPlayerEntData->prevItemBinding = In_FindButtonMapping(pInputCtx, "prevItem");

    pPlayerEntData->settingsMenuBinding = In_FindButtonMapping(pInputCtx, "settings");
    memset(&pPlayerEntData->playerControlsMask, 0, sizeof(struct ActiveInputBindingsMask));
    In_ActivateButtonBinding(pPlayerEntData->moveUpBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveDownBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveLeftBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveRightBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->settingsMenuBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->nextItemBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->prevItemBinding, &pPlayerEntData->playerControlsMask);

    In_SetMask(&pPlayerEntData->playerControlsMask, pInputCtx);
    
    ClampCameraToTileLayer(pLayer->userData, 0);

    if(NW_GetRole() == GR_Client && !pPlayerEntData->bNetworkControlled)
    {
        /* client has made their player entity, cause it to be created on server + other clients */
        struct CreateEntity_RPC rpc = 
        {
            .pData = pLayer->userData,
            .pEnt = pEnt
        };
        Log_Info("Sending G2DRPC_CreateEntity for clients player entity");
        G2D_SendRPC(-1, G2DRPC_CreateEntity, &rpc);
    }
}

static void OnDestroyPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gPlayerEntDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static void SetBasePlayerAnimation(enum WfDirection dir, struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
    pSprite->bIsAnimating = pPlayerEntData->movementBits != 0;
    switch(dir)
    {
    case Up:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_UP_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    case Down:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_DOWN_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    case Left:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_LEFT_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    case Right:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_RIGHT_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    
    }
}

static void SyncAnimA2B(struct AnimatedSprite* pA, struct AnimatedSprite* pB)
{
    pB->bDraw = pA->bDraw;
    pB->bIsAnimating = pA->bIsAnimating;
    pB->bRepeat = pA->bRepeat;
    pB->fps = pA->fps;
    pB->numSprites = pA->numSprites;
    pB->onSprite = pA->onSprite;
    pB->timer = pA->timer;
}

void WfSetPlayerOverlayAnimations(enum WfDirection dir, struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    struct Component2D* pCompBaseAnimator = &pEnt->components[PLAYER_SPRITE_COMP_INDEX];
    struct AnimatedSprite* pBaseAnimatedSprite = &pCompBaseAnimator->data.spriteAnimator;
    EASSERT(pCompBaseAnimator->type == ETE_SpriteAnimator);
    struct GameLayer2DData* pLayerData = pLayer->userData;
    for(int i=0; i<WfNumBackgroundAnimationLayers; i++)
    {
        if(pPlayerEntData->animationSet.bgLayersMask & (1 << i))
        {
            struct Component2D* pComp = &pEnt->components[PLAYER_BG_SPRITE_COMP_START + i];
            EASSERT(pComp->type == ETE_SpriteAnimator);
            struct AnimatedSprite* pOverlayAnimator = &pComp->data.spriteAnimator;
            AnimatedSprite_SetAnimation(pLayer, pOverlayAnimator, pPlayerEntData->animationSet.bgLayers[i].walkAnimations[dir], false, false);
            SyncAnimA2B(pBaseAnimatedSprite, pOverlayAnimator);
        }
    }

    for(int i=0; i<WfNumAnimationLayers; i++)
    {
        if(pPlayerEntData->animationSet.layersMask & (1 << i))
        {
            struct Component2D* pComp = &pEnt->components[PLAYER_SPRITE_COMP_INDEX + 1 + i];
            EASSERT(pComp->type == ETE_SpriteAnimator);
            struct AnimatedSprite* pOverlayAnimator = &pComp->data.spriteAnimator;
            AnimatedSprite_SetAnimation(pLayer, pOverlayAnimator, pPlayerEntData->animationSet.layers[i].walkAnimations[dir], false, false);
            SyncAnimA2B(pBaseAnimatedSprite, pOverlayAnimator);
        }
    }
}

static void SetPlayerAnimation(struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    /* set the base animation */
    struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
    pSprite->bIsAnimating = pPlayerEntData->movementBits != 0;
    for(int i=0; i<WfNumAnimationLayers; i++)
    {
        if(pPlayerEntData->animationSet.layersMask & (1 << i))
        {
            pEnt->components[PLAYER_SPRITE_COMP_INDEX + 1 + i].data.spriteAnimator.bIsAnimating = pSprite->bIsAnimating;
        }
    }
    if(!(pPlayerEntData->movementBits != 0) && pPlayerEntData->bMovingLastFrame)
    {
        pSprite->onSprite = 0;
        for(int i=0; i<WfNumAnimationLayers; i++)
        {
            if(pPlayerEntData->animationSet.layersMask & (1 << i))
            {
                pEnt->components[PLAYER_SPRITE_COMP_INDEX + 1 + i].data.spriteAnimator.onSprite = 0;
            }
        }
    }
    if(pPlayerEntData->movementBits & WfPD_Down)
    {
        // moving down
        SetBasePlayerAnimation(Down, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Down, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Down;
    }
    else if(pPlayerEntData->movementBits & WfPD_Up)
    {
        // moving up
        SetBasePlayerAnimation(Up, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Up, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Up;
    }
    else if(pPlayerEntData->movementBits & WfPD_Right)
    {
        // moving right
        SetBasePlayerAnimation(Right, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Right, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Right;
    }
    else if(pPlayerEntData->movementBits & WfPD_Left)
    {
        // moving left
        SetBasePlayerAnimation(Left, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Left, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Left;
    }
}

static void GetMovementVector(u8 movementBits, vec2 movementVec)
{
    movementVec[0] = 0.0f;
    movementVec[1] = 0.0f;

    if(movementBits & WfPD_Up)
    {
        movementVec[1] = -1.0f;
    }
    if(movementBits & WfPD_Down)
    {
        movementVec[1] = 1.0f;
    }
    if(movementBits & WfPD_Right)
    {
        movementVec[0] = 1.0f;
    }
    if(movementBits & WfPD_Left)
    {
        movementVec[0] = -1.0f;
    }
    glm_vec2_normalize(movementVec);
}

static void OnUpdatePlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    switch (pPlayerEntData->state)
    {
    case WfWalking:
        {
            vec2 scaledMovement, mov;
            GetMovementVector(pPlayerEntData->movementBits, mov);
            scaledMovement[0] = mov[0] * pPlayerEntData->metersPerSecondWalkSpeedBase * deltaT * pPlayerEntData->speedMultiplier;
            scaledMovement[1] = mov[1] * pPlayerEntData->metersPerSecondWalkSpeedBase * deltaT * pPlayerEntData->speedMultiplier;
            Ph_SetDynamicBodyVelocity(
                pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider.id,
                scaledMovement
            );
            SetPlayerAnimation(pLayer, pPlayerEntData, pEnt);
        }
        
        break;
    case WfAttacking:
        {
            struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
            if(!pSprite->bIsAnimating)
            {
                pPlayerEntData->state = WfWalking;
                SetBasePlayerAnimation(pPlayerEntData->directionFacing, pLayer, pPlayerEntData, pEnt);
                WfSetPlayerOverlayAnimations(pPlayerEntData->directionFacing, pLayer, pPlayerEntData, pEnt);
            }
        }
        break;
    default:
        break;
    }
    
    Entity2DUpdate(pEnt, pLayer, deltaT);
}


static struct WfInventory* WfGetPlayerInventory(struct WfPlayerEntData* pEntData)
{
    if(pEntData->bNetworkControlled)
    {
        return WfGetNetworkPlayersInventory(pEntData->networkPlayerNum);
    }
    return WfGetInventory();
}


static void ChangeItem(struct GameFrameworkLayer* pLayer, struct Entity2D* pEnt, struct WfPlayerEntData* pPlayerEntData, int incr)
{
    struct WfInventory* pInv = WfGetPlayerInventory(pPlayerEntData);

    int oldIndex = pInv->selectedItem;
    pInv->selectedItem += incr;
    if(pInv->selectedItem >= WF_INVENTORY_ROW_SIZE)
    {
        pInv->selectedItem = 0;
    }
    if(pInv->selectedItem < 0)
    {
        pInv->selectedItem = WF_INVENTORY_ROW_SIZE - 1;
    }
    struct ScriptCallArgument arg;
    arg.type = SCA_int;
    arg.val.i = pInv->selectedItem;
    struct LuaListenedEventArgs args = { .numArgs = 1, .args = &arg };
    
    if(!pPlayerEntData->bNetworkControlled)
    {
        Ev_FireEvent("SelectedItemChanged", &args);
    }

    struct WfItemDef* pOldItem = WfGetItemDef(pInv->pItems[oldIndex].itemIndex);
    struct WfItemDef* pNewItem = WfGetItemDef(pInv->pItems[pInv->selectedItem].itemIndex);
    if(pOldItem)
    {
        pOldItem->onStopBeingCurrent(pEnt, pLayer);
    }
    if(pNewItem)
    {
        pNewItem->onMakeCurrent(pEnt, pLayer);
    }
}

static void OnInputPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];

    /* network players need to do this part too so that their sprite animation is reset when they stop moving,
     but bMovingThisFrame and the movement vector is set from the network not input */
    pPlayerEntData->bMovingLastFrame = pPlayerEntData->movementBits != 0;

    if(pPlayerEntData->bNetworkControlled)
    {
        return;
    }

    Entity2DInput(pEnt, pLayer, context);
    pPlayerEntData->movementBits = 0;
    struct GameLayer2DData* pLayerData = pLayer->userData;
    
    if(In_GetButtonValue(context, pPlayerEntData->moveUpBinding))
    {
        pPlayerEntData->movementBits |= WfPD_Up;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveDownBinding))
    {
        pPlayerEntData->movementBits |= WfPD_Down;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveLeftBinding))
    {
        pPlayerEntData->movementBits |= WfPD_Left;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveRightBinding))
    {
        pPlayerEntData->movementBits |= WfPD_Right;
    }
    if(In_GetButtonPressThisFrame(context, pPlayerEntData->settingsMenuBinding))
    {
        GF_PopGameFrameworkLayer();
        WfPushSettings(pLayerData->pDrawContext);
    }
    if(In_GetButtonPressThisFrame(context, pPlayerEntData->nextItemBinding))
    {
        ChangeItem(pLayer, pEnt, pPlayerEntData, 1);
    }
    if(In_GetButtonPressThisFrame(context, pPlayerEntData->prevItemBinding))
    {
        ChangeItem(pLayer, pEnt, pPlayerEntData, -1);
    }
}



float WfGetPlayerSortPosition(struct Entity2D* pEnt)
{
    return pEnt->transform.position[1] - gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform[1];
}

void WfPlayerGetGroundContactPoint(struct Entity2D* pEnt, vec2 outPos)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    vec2 pos2ground = {
        -pPlayerEntData->groundColliderCenter2EntTransform[0],
        -pPlayerEntData->groundColliderCenter2EntTransform[1],
    };
    glm_vec2_add(pEnt->transform.position, pos2ground, outPos);
}

void WfPlayerPostPhys(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    struct GameLayer2DData* pLayerData = pLayer->userData;
    Entity2DUpdatePostPhysics(pEnt, pLayer, deltaT);
    vec2 pixelsPos, physPos;
    struct DynamicCollider* pCollider = &pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider;
    
    Ph_GetDynamicBodyPosition(pCollider->id, physPos);
    Ph_PhysicsCoords2PixelCoords(pLayerData->hPhysicsWorld, physPos, pixelsPos);
    glm_vec2_add(pixelsPos, pPlayerEntData->groundColliderCenter2EntTransform, pEnt->transform.position);

    if(!pPlayerEntData->bNetworkControlled)
    {
        CenterCameraAt(pixelsPos[0], pixelsPos[1], &pLayerData->camera, pLayerData->windowW, pLayerData->windowH);
    }
}

static void WfPrintPlayerInfo(struct Entity2D* pEnt)
{
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pEnt->user.hData];
    Log_Info("Network controlled? : %s Network player number: %i",
        pEntData->bNetworkControlled ? "true" : "false",
        pEntData->networkPlayerNum
    );
}

static void WfMakeIntoPlayerEntityBase(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos, bool bNetworkControlled, int networkPlayerNum)
{
    struct GameLayer2DData* pData = pLayer->userData;
    
    if(pEnt->user.hData == NULL_HANDLE)
    {
        gPlayerEntDataPool = GetObjectPoolIndex(gPlayerEntDataPool, &pEnt->user.hData);
    }
    
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pEntData->directionFacing = Down;
    pEntData->groundColliderCenter2EntTransform[0] = -32;
    pEntData->groundColliderCenter2EntTransform[1] = -60;
    pEntData->animationSet.layersMask = 0;
    pEntData->state = WfWalking;
    pEntData->bNetworkControlled = bNetworkControlled;
    pEntData->networkPlayerNum = networkPlayerNum - 1;
    if(!bNetworkControlled && NW_GetRole() == GR_Client)
    {
        pEnt->networkID = NetID_GetID();
    }

    pEnt->numComponents = 0;
    pEnt->type = WfEntityType_Player;

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

    /*
        player background animation 1
    */

    struct Component2D* pComponent3 = &pEnt->components[pEnt->numComponents++];
    pComponent3->type = ETE_SpriteAnimator;
    struct AnimatedSprite* pAnimatedSprite = &pComponent3->data.spriteAnimator;
    memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));

    /*
        player background animation 2
    */

    struct Component2D* pComponent4 = &pEnt->components[pEnt->numComponents++];
    pComponent4->type = ETE_SpriteAnimator;
    pAnimatedSprite = &pComponent4->data.spriteAnimator;
    memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));

    /*
        Base Animated Sprite
    */
    struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
    pComponent1->type = ETE_SpriteAnimator;
    pAnimatedSprite = &pComponent1->data.spriteAnimator;
    memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));
    pAnimatedSprite->animationName = WALKING_DOWN_MALE;
    pAnimatedSprite->bRepeat = true;
    pAnimatedSprite->transform.scale[0] = 1.0f;
    pAnimatedSprite->transform.scale[1] = 1.0f;
    pAnimatedSprite->bIsAnimating = false;
    pAnimatedSprite->bDraw = true;
    

    glm_vec2_add(spawnAtGroundPos, gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform, pEnt->transform.position);
    pEnt->transform.scale[0] = 1.0;
    pEnt->transform.scale[1] = 1.0;

    /* Animated sprite overlay layers */
    for(int i=0; i<WfNumAnimationLayers; i++)
    {
        struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
        pComponent1->type = ETE_SpriteAnimator;
        struct AnimatedSprite* pAnimatedSprite = &pComponent1->data.spriteAnimator;
        memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));
        pAnimatedSprite->animationName = "";
        pAnimatedSprite->bRepeat = true;
        pAnimatedSprite->transform.scale[0] = 1.0f;
        pAnimatedSprite->transform.scale[1] = 1.0f;
        pAnimatedSprite->bIsAnimating = false;
        pAnimatedSprite->bDraw = false;
    }
    

    pEnt->inDrawLayer = 0;

    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->init = &OnInitPlayer;
    pEnt->update = &OnUpdatePlayer;
    pEnt->getSortPos = &WfGetPlayerSortPosition;
    pEnt->postPhys = &WfPlayerPostPhys;
    pEnt->input = &OnInputPlayer;
    pEnt->printEntityInfo = &WfPrintPlayerInfo;
    pEnt->bKeepInQuadtree = false;
    pEnt->bKeepInDynamicList = true;
    pEnt->bSerializeToDisk = false;
    pEnt->bSerializeToNetwork = true;

    struct WfInventory* pInv = WfGetPlayerInventory(pEntData);
    struct WfInventoryItem* pItem = &pInv->pItems[pInv->selectedItem];
    if(pItem->itemIndex >= 0)
    {
        struct WfItemDef* pDef = WfGetItemDef(pItem->itemIndex);
        pDef->onMakeCurrent(pEnt, pLayer);
    }
}

void WfSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    /* 
        to serialize:
            - ground position
            - persistent data index
    */
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pInEnt->user.hData];
    BS_SerializeU32(1, bs); /* version */
    if(bs->ctx == SCTX_ToNetworkUpdate)
    {
        struct DynamicCollider* pCollider = &pInEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider;
        vec2 physPos;
        Ph_GetDynamicBodyPosition(pCollider->id, physPos);

        /* physics position */
        BS_SerializeFloat(physPos[0], bs);
        BS_SerializeFloat(physPos[1], bs);

        /* pack movement bits and selected inventory item into an 8 bit val */
        u8 val = pEntData->movementBits;
        struct WfInventory* pInv = WfGetInventory();
        val |= ((u8)pInv->selectedItem << 4);
        BS_SerializeU8(val, bs);
    }
    else
    {
        vec2 out;
        WfPlayerGetGroundContactPoint(pInEnt, out);
        BS_SerializeFloat(out[0], bs);
        BS_SerializeFloat(out[1], bs);
        /* get the index of the next network player */
        int numSlots = WfGetNumNetworkPlayerPersistentDataSlots();
        BS_SerializeI32(numSlots, bs);
    }
}

void WfDeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 val;
    BS_DeSerializeU32(&val, bs);
    switch(val)
    {
    case 1:
        {
            if(bs->ctx == SCTX_ToNetworkUpdate)
            {
                struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pOutEnt->user.hData];
                
                /* physics position */
                vec2 physPos;
                BS_DeSerializeFloat(&physPos[0], bs);
                BS_DeSerializeFloat(&physPos[1], bs);
                struct DynamicCollider* pCollider = &pOutEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider;
                Ph_SetDynamicBodyPosition(pCollider->id, physPos);
                u8 val = 0;

                /* movement enum and selected item */
                BS_DeSerializeU8(&val, bs);
                u8 movement = val & 0xf;
                u8 selectedItem = (val >> 4);
                pEntData->movementBits = movement;
                

                struct WfInventory* pInv = WfGetPlayerInventory(pEntData);
                if(pInv->selectedItem != selectedItem)
                {
                    if(selectedItem > pInv->selectedItem)
                    {
                        ChangeItem(pData->pLayer, pOutEnt, pEntData, selectedItem - pInv->selectedItem);
                    }
                    else
                    {
                        ChangeItem(pData->pLayer, pOutEnt, pEntData, selectedItem - pInv->selectedItem);
                    }   
                }
            }
            else
            {
                Log_Info("WfDeSerializePlayerEntity");
                vec2 groundPoint = {0.0f, 0.0f};
                i32 slotNum = 0;
                gPlayerEntDataPool = GetObjectPoolIndex(gPlayerEntDataPool, &pOutEnt->user.hData);
                struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pOutEnt->user.hData];
                BS_DeSerializeFloat(&pEntData->createNetPlayerOnInitArgs.netPlayerSpawnAtPos[0], bs);
                BS_DeSerializeFloat(&pEntData->createNetPlayerOnInitArgs.netPlayerSpawnAtPos[1], bs);
                BS_DeSerializeI32(&pEntData->createNetPlayerOnInitArgs.netPlayerSlot, bs);
                Log_Info("WfDeSerializePlayerEntity Player slot: %i netID: %i", pEntData->createNetPlayerOnInitArgs.netPlayerSlot, pOutEnt->networkID);
                pEntData->bNetworkControlled = true;
                pOutEnt->init = &OnInitPlayer; /* set this one and in the init function the rest will be bootstrapped */
                pOutEnt->update = NULL;
                pOutEnt->postPhys = NULL;
                pOutEnt->draw = NULL;
                Log_Info("Player pos: x %.2f, y %.2f. Player slot %i", 
                    pEntData->createNetPlayerOnInitArgs.netPlayerSpawnAtPos[0], 
                    pEntData->createNetPlayerOnInitArgs.netPlayerSpawnAtPos[1], slotNum);
            }
        }
        break;
    default:
        Log_Error("DeserializePlayer unknown schema version %u", val);
    }
}

void WfMakeIntoPlayerEntity(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos)
{
    WfMakeIntoPlayerEntityBase(pEnt, pLayer, spawnAtGroundPos, false, -1);
}

struct WfAnimationSet* WfGetPlayerAnimationSet(struct Entity2D* pInPlayerEnt)
{
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pInPlayerEnt->user.hData];
    return &pEntData->animationSet;
}

void WfSetPlayerAnimationSet(struct Entity2D* pInPlayerEnt, const struct WfAnimationSet* pInSet)
{
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pInPlayerEnt->user.hData];
    memcpy(&pEntData->animationSet, pInSet, sizeof(struct WfAnimationSet));
}

struct WfPlayerEntData* WfGetPlayerEntData(struct Entity2D* pInEnt)
{
    return &gPlayerEntDataPool[pInEnt->user.hData];
}

struct Component2D* WfGetPlayerAnimationLayerComponent(struct Entity2D* pPlayer, enum WfAnimationLayerNames layer)
{
    return &pPlayer->components[PLAYER_SPRITE_COMP_INDEX + 1 + (int)layer];
}
