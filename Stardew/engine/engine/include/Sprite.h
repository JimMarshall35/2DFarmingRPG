#ifndef STARDEWSPRITE_H
#define STARDEWSPRITE_H
#include "DrawContext.h"
#include "DynArray.h"

struct Entity2D;
struct GameFrameworkLayer;
struct DrawContext;
struct InputContext;
typedef struct InputContext InputContext;
struct Sprite;
struct Transform2D;

void SpriteComp_OnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void SpriteComp_Update(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void SpriteComp_UpdatePostPhysics(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void SpriteComp_Draw(struct Sprite* pSpriteComp, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex);
void SpriteComp_Input(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context);
void SpriteComp_OnDestroy(struct Entity2D* pEnt);

void SpriteComp_GetBoundingBox(struct Entity2D* pEnt, struct Sprite* pSpriteComp, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR);

#endif

