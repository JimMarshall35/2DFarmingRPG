#ifndef PHYSICS2D_H
#define PHYSICS2D_H
#include "HandleDefs.h"
#include <cglm/cglm.h>
#include "DynArray.h"

typedef vec2 Physics2DPoint;
struct Transform2D;

struct Physics2DRect
{
    float w, h;
};

struct Physics2DPoly
{
    VECTOR(Physics2DPoint) pPoints;
};

struct Physics2DCircle
{
    vec2 center;
    float radius;
};

enum PhysicsBodyType
{
    PBT_Rect,
    PBT_Circle,
    PBT_Ellipse,
    PBT_Poly
};
struct GameFrameworkLayer;

struct PhysicsShape2D
{
    enum PhysicsBodyType type;
    union
    {
        struct Physics2DCircle circle;
        struct Physics2DPoly poly;
        struct Physics2DRect rect;
    }data;
};

struct KinematicBodyOptions
{
    u32 bLockRotation : 1;
};

void Ph_Init();

HPhysicsWorld Ph_GetPhysicsWorld(float gravityX, float gravityY, float pixelsPerMeter);

void Ph_PhysicsWorldStep(HPhysicsWorld hWorld, float timestep, int substepCount);

void Ph_PhysicsWorldDoCollisionEvents(struct GameFrameworkLayer* pLayer);

void Ph_DestroyPhysicsWorld(HPhysicsWorld world);

void Ph_PixelCoords2PhysicsCoords(HPhysicsWorld world, vec2 inPixelCoords, vec2 outPhysicsCoords);

void Ph_PhysicsCoords2PixelCoords(HPhysicsWorld world, vec2 inPhysicsCoords, vec2 outPixelCoords);

H2DBody Ph_GetStaticBody2D(HPhysicsWorld world, struct PhysicsShape2D* pShape, struct Transform2D* pTransform, HEntity2D entity, bool bIsSensor, int entityComponentIndex, bool bGenerateSensorEvents);

H2DBody Ph_GetDynamicBody(HPhysicsWorld world, struct PhysicsShape2D* pShape, struct KinematicBodyOptions* pOptions, struct Transform2D* pTransform, HEntity2D entity, bool bIsSensor, int entityComponentIndex, bool bGenerateSensorEvents);

float Ph_GetPixelsPerMeter(HPhysicsWorld world);

void Ph_SetDynamicBodyVelocity(H2DBody hBody, vec2 velocity);

void Ph_GetDynamicBodyVelocity(H2DBody hBody, vec2 outVelocity);

void Ph_GetDymaicBodyPosition(H2DBody hBody, vec2 outPos);

void Ph_UnpackShapeUserData(void* pUserData, HEntity2D* pOutEnt, u16* pOutCompIndex, u16* pOutBodyType);

u64 Ph_PackShapeUserData(HEntity2D hEnt, u16 componentIndex, u16 bodyType);

#endif
