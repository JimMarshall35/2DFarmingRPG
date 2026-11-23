#ifndef ANIMATEDSPRITE_H
#define ANIMATEDSPRITE_H
#include <cglm/cglm.h>
#include "DrawContext.h"
struct AnimatedSprite;
struct Entity2D;
struct GameFrameworkLayer;
struct Transform2D;
#define VECTOR(a)a*
void AnimatedSprite_OnInit(struct AnimatedSprite* pAnimatedSprite, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void AnimatedSprite_OnUpdate(struct AnimatedSprite* pAnimatedSprite, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void AnimatedSprite_GetBoundingBox(struct Entity2D* pEnt, struct AnimatedSprite* pAnimatedSprite, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR);
void AnimatedSprite_Draw(struct AnimatedSprite* pSpriteComp, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex);
void AnimatedSprite_OnDestroy(struct Entity2D* pEnt);
void AnimatedSprite_SetAnimation(struct GameFrameworkLayer* pLayer, struct AnimatedSprite* pSpriteComp, const char* animName, bool bResetOnFrame, bool bResetTimer);

#endif