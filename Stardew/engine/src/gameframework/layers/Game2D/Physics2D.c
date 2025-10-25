#include "Physics2D.h"
#include "box2d/box2d.h"
#include "ObjectPool.h"
#include "Game2DLayer.h"
#include "AssertLib.h"

struct Phys2dWorld
{
    /* data */
    b2WorldId id;
    float gravX;
    float gravY;
    float pxlPerMeter;
};

struct Body2D
{
    b2BodyType type;
    b2BodyId bodyID;

    enum PhysicsBodyType shapeType;
    union 
    {
        b2Polygon poly;
        b2Circle circle;
    }shape;
    
    b2ShapeDef shapedef;
    b2ShapeId shapeID;
};

static OBJECT_POOL(struct Phys2dWorld) gWorldDefPool = NULL;

static OBJECT_POOL(struct Body2D) g2DPhysBodyPool = NULL;



void Ph_DestroyPhysicsWorld(HPhysicsWorld world)
{
    b2DestroyWorld(gWorldDefPool[world].id);
    FreeObjectPoolIndex(gWorldDefPool, world);
}

void Ph_Init()
{
    gWorldDefPool = NEW_OBJECT_POOL(struct Phys2dWorld, 32);
    g2DPhysBodyPool = NEW_OBJECT_POOL(struct Body2D, 256);
}

HPhysicsWorld Ph_GetPhysicsWorld(float gravityX, float gravityY, float pixelsPerMeter)
{
    HPhysicsWorld index = -1;
    gWorldDefPool = GetObjectPoolIndex(gWorldDefPool, &index);
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity.x = gravityX;
    def.gravity.y = gravityY;
    gWorldDefPool[index].gravX = gravityX;
    gWorldDefPool[index].gravY = gravityY;
    gWorldDefPool[index].pxlPerMeter = pixelsPerMeter;
    gWorldDefPool[index].id = b2CreateWorld(&def);
    return index;
}

void Ph_PhysicsWorldStep(HPhysicsWorld hWorld, float timestep, int substepCount)
{
    b2World_Step(gWorldDefPool[hWorld].id, timestep, substepCount);
    //b2Body_SetLinearVelocity
}

float Ph_GetPixelsPerMeter(HPhysicsWorld world)
{
    return gWorldDefPool[world].pxlPerMeter;
}

void Ph_PixelCoords2PhysicsCoords(HPhysicsWorld world, vec2 inPixelCoords, vec2 outPhysicsCoords)
{
    struct Phys2dWorld* pPool = &gWorldDefPool[world];
    outPhysicsCoords[0] = inPixelCoords[0] / pPool->pxlPerMeter;
    outPhysicsCoords[1] = inPixelCoords[1] / pPool->pxlPerMeter;
}

void Ph_PhysicsCoords2PixelCoords(HPhysicsWorld world, vec2 inPhysicsCoords, vec2 outPixelCoords)
{
    struct Phys2dWorld* pPool = &gWorldDefPool[world];
    outPixelCoords[0] = inPhysicsCoords[0] * pPool->pxlPerMeter;
    outPixelCoords[1] = inPhysicsCoords[1] * pPool->pxlPerMeter;
}

static H2DBody GetBody(HPhysicsWorld world, struct PhysicsShape2D* pShape, struct Transform2D* pTransform, b2BodyType type)
{
    H2DBody hStatic = -1;
    g2DPhysBodyPool = GetObjectPoolIndex(g2DPhysBodyPool, &hStatic);
    g2DPhysBodyPool[hStatic].type = type;
    

    switch (pShape->type)
    {
    case PBT_Rect:
        {
            vec2 pixelsRectCenter = {
                pTransform->position[0] + pShape->data.rect.w / 2,
                pTransform->position[1] + pShape->data.rect.h / 2,
            };
            vec2 pixelDims = {
                pShape->data.rect.w,
                pShape->data.rect.h
            };
            vec2 physicsDims;
            vec2 physicsPos;
            Ph_PixelCoords2PhysicsCoords(world, pixelsRectCenter, physicsPos);
            Ph_PixelCoords2PhysicsCoords(world, pixelDims, physicsDims);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = type;
            bodyDef.position = (b2Vec2){physicsPos[0], physicsPos[1]};
            b2BodyId id = b2CreateBody(gWorldDefPool[world].id, &bodyDef);

            g2DPhysBodyPool[hStatic].bodyID = id;
            g2DPhysBodyPool[hStatic].shapedef = b2DefaultShapeDef();

            
            g2DPhysBodyPool[hStatic].shape.poly = b2MakeBox(physicsDims[0] / 2.0f, physicsDims[1] / 2.0f);
            g2DPhysBodyPool[hStatic].shapeID = b2CreatePolygonShape(id, &g2DPhysBodyPool[hStatic].shapedef, &g2DPhysBodyPool[hStatic].shape.poly);
        }
        break;
    case PBT_Circle:
        {
            vec2 physicsPos;
            float physicsRadius = pShape->data.circle.radius / Ph_GetPixelsPerMeter(world);
            Ph_PixelCoords2PhysicsCoords(world, pShape->data.circle.center, physicsPos);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = type;
            bodyDef.position = (b2Vec2){physicsPos[0], physicsPos[1]};
            b2BodyId id = b2CreateBody(gWorldDefPool[world].id, &bodyDef);

            g2DPhysBodyPool[hStatic].bodyID = id;
            g2DPhysBodyPool[hStatic].shapedef = b2DefaultShapeDef();

            
            g2DPhysBodyPool[hStatic].shape.circle.center.x = 0.0f;//physicsPos[0];
            g2DPhysBodyPool[hStatic].shape.circle.center.y = 0.0f;//physicsPos[1];
            g2DPhysBodyPool[hStatic].shape.circle.radius = physicsRadius;

            g2DPhysBodyPool[hStatic].shapeID = b2CreateCircleShape(id, &g2DPhysBodyPool[hStatic].shapedef, &g2DPhysBodyPool[hStatic].shape.circle);
        }
        break;
    case PBT_Ellipse:
        EASSERT(false);
        break;
    case PBT_Poly:
        EASSERT(false);
        break;
    default:
        break;
    }
    return hStatic;
}

H2DBody Ph_GetStaticBody2D(HPhysicsWorld world, struct PhysicsShape2D* pShape, struct Transform2D* pTransform)
{
    return GetBody(world, pShape, pTransform, b2_staticBody);
}

H2DBody Ph_GetKinematicBody(HPhysicsWorld world, struct PhysicsShape2D* pShape, struct KinematicBodyOptions* pOptions, struct Transform2D* pTransform)
{
    return GetBody(world, pShape, pTransform, b2_dynamicBody);
}

void Ph_SetDynamicBodyVelocity(H2DBody hBody, vec2 velocity)
{
    //EASSERT(g2DPhysBodyPool[hBody].type == b2_kinematicBody);
    b2Vec2 b2Vec = {.x = velocity[0], .y = velocity[1]};
    b2Body_SetLinearVelocity(g2DPhysBodyPool[hBody].bodyID, b2Vec);
}

void Ph_GetDynamicBodyVelocity(H2DBody hBody, vec2 outVelocity)
{
    EASSERT(g2DPhysBodyPool[hBody].type == b2_kinematicBody);
    b2Vec2 b2Vec = b2Body_GetLinearVelocity(g2DPhysBodyPool[hBody].bodyID);
    outVelocity[0] = b2Vec.x;
    outVelocity[1] = b2Vec.y;
}

void Ph_GetDymaicBodyPosition(H2DBody hBody, vec2 outPos)
{
    b2Vec2 b2Vec = b2Body_GetPosition(g2DPhysBodyPool[hBody].bodyID);
    outPos[0] = b2Vec.x;
    outPos[1] = b2Vec.y;
}
