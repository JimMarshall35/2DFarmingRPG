#ifndef COMPONENTS2D_H
#define COMPONENTS2D_H

#include "DynArray.h"
#include "DrawContext.h"

struct Entity2D;
struct GameFrameworkLayer;
struct InputContext;
typedef struct InputContext InputContext;
struct Transform2D;
struct Component2D;

typedef void(*InitComponentsFn)(struct Component2D* pComponent, struct Entity2D* entity, struct GameFrameworkLayer* pLayer);
typedef void(*UpdateComponentsFn)(struct Component2D* pComponent, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
typedef void(*UpdatePostPhysComponentsFn)(struct Component2D* pComponent, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
typedef void(*InputComponentsFn)(struct Component2D* pComponent, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context);
typedef void(*DestroyComponentsFn)(struct Component2D* pComponent, struct Entity2D* entity, struct GameFrameworkLayer* pLayer);
typedef void(*DrawComponentsFn)(struct Component2D* pComponent, struct Entity2D* entity, 
    struct GameFrameworkLayer* pLayer,
    struct Transform2D* pCam,
    VECTOR(Worldspace2DVert)* outVerts,
    VECTOR(VertIndexT)* outIndices,
    VertIndexT* pNextIndex);


void Co_InitComponents(struct Entity2D* entity, struct GameFrameworkLayer* pLayer);
void Co_UpdateComponents(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
void Co_Entity2DUpdatePostPhysicsFn(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);

void Co_InputComponents(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context);
void Co_DestroyComponents(struct Entity2D* entity, struct GameFrameworkLayer* pLayer);
void Co_DrawComponents(
    struct Entity2D* entity, 
    struct GameFrameworkLayer* pLayer,
    struct Transform2D* pCam,
    VECTOR(Worldspace2DVert)* outVerts,
    VECTOR(VertIndexT)* outIndices,
    VertIndexT* pNextIndex);

void Co_RegisterGameComponentHandlers(
    InitComponentsFn init,
    UpdateComponentsFn update,
    UpdatePostPhysComponentsFn postPhys,
    InputComponentsFn input,
    DestroyComponentsFn destroy,
    DrawComponentsFn draw
);

#endif