#ifndef WFPLAYER_H
#define WFPLAYER_H

#include <cglm/cglm.h>
#include "WfEnums.h"
#include "InputContext.h"


struct BinarySerializer;
struct Entity2D;
struct Component2D;
struct GameLayer2DData;
struct GameFrameworkLayer;

#define NUM_ANIMATIONS NumDirections

#define PLAYER_SPRITE_COMP_INDEX 3
#define PLAYER_BG_SPRITE_COMP_START 1
#define PLAYER_NUM_BG_SPRITES 2

#define PLAYER_COLLIDER_COMP_INDEX 0


/// @brief A set of animations for everything the player can do
struct WfAnimationSetLayer
{
    const char* walkAnimations[NUM_ANIMATIONS];
    const char* slashAnimations[NUM_ANIMATIONS];
    const char* thrustAnimations[NUM_ANIMATIONS];
};

/* layers on top of the base */
enum WfAnimationLayerNames
{
    WfHairAnimationLayer,
    WfTorsoAnimationLayer,
    WfLegAnimationLayer,
    WfToolAnimationLayer,
    WfNumAnimationLayers
};

/* layers behind the base */
enum WfBGAnimationLayerNames
{
    WfBG1,
    WfBG2,
    WfNumBackgroundAnimationLayers
};

/// @brief A set of anumations drawn behind and in front of the base sprite
struct WfAnimationSet
{
    /// @brief Layers behind the sprite
    struct WfAnimationSetLayer bgLayers[WfNumBackgroundAnimationLayers];
    
    /// @brief Layers in front of the sprite
    struct WfAnimationSetLayer layers[WfNumAnimationLayers];
    
    /// @brief which layers in front are drawn
    unsigned int layersMask;

    /// @brief which layers behind are drawn
    unsigned int bgLayersMask;
};

enum WfPlayerState
{
    WfWalking,
    WfAttacking,
    WfNumPlayerStates
};

enum WfPlayerDirection
{
    WfPD_Up = 1,
    WfPD_Down = (1 << 1),
    WfPD_Left = (1 << 2),
    WfPD_Right = (1 << 3),
};


struct WfPlayerEntData
{
    vec2 groundColliderCenter2EntTransform;
    struct ButtonBinding moveUpBinding;
    struct ButtonBinding moveDownBinding;
    struct ButtonBinding moveLeftBinding;
    struct ButtonBinding moveRightBinding;

    struct ButtonBinding nextItemBinding;
    struct ButtonBinding prevItemBinding;


    struct ButtonBinding settingsMenuBinding;
    struct ActiveInputBindingsMask playerControlsMask;
    /* value I set this to is NOT meters per second, TODO: fix */
    float metersPerSecondWalkSpeedBase;

    float speedMultiplier;

    u8 movementBits;

    struct WfAnimationSet animationSet;

    enum WfDirection directionFacing;

    enum WfPlayerState state;

    /* 
        its shit having to have this, the position to spawn the network player when it is initialized.
        The network player is created in two steps, first they're deserialized into a player entity then the player entity is initialized.
        We have to call WfMakeIntoPlayerEntityBase when it is initialized, and so must serialize args for it in the deserialize stage
    */
    struct 
    {
        vec2 netPlayerSpawnAtPos; 
        int netPlayerSlot;
    } createNetPlayerOnInitArgs;
    

    int networkPlayerNum;

    /* flags section */
    u32 bMovingLastFrame : 1;
    u32 bNetworkControlled : 1;
};

void WfInitPlayer();

struct WfAnimationSet* WfGetPlayerAnimationSet(struct Entity2D* pInPlayerEnt);

void WfSetPlayerAnimationSet(struct Entity2D* pInPlayerEnt, const struct WfAnimationSet* pInSet);

void WfDeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

void WfMakeIntoPlayerEntity(struct Entity2D* pInEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos);

struct WfPlayerEntData* WfGetPlayerEntData(struct Entity2D* pInEnt);

struct Component2D* WfGetPlayerAnimationLayerComponent(struct Entity2D* pPlayer, enum WfAnimationLayerNames layer);

void WfSetPlayerOverlayAnimations(enum WfDirection dir, struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt);

void WfPlayerGetGroundContactPoint(struct Entity2D* pEnt, vec2 outPos);

#endif