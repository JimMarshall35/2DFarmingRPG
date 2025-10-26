#include "StaticColliderEntity.h"
#include "BinarySerializer.h"
#include "Physics2D.h"
#include "AssertLib.h"
#include "GameFramework.h"
#include "DrawContext.h"
#include "InputContext.h"

void StaticColliderOnInitFn(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    Entity2DOnInit(pEnt, pLayer, pDrawCtx, pInputCtx);
}

void StaticColliderUpdate(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    Entity2DUpdate(pEnt, pLayer, deltaT);
}

void StaticColliderUpdatePostPhysics(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    Entity2DUpdatePostPhysics(pEnt, pLayer, deltaT);
}

void StaticColliderDrawFn(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex)
{
    Entity2DDraw(pEnt, pLayer, pCam, outVerts, outIndices, pNextIndex);
}

void StaticColliderInput(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    Entity2DInput(pEnt, pLayer, context);
}

void StaticColliderOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer)
{
    Entity2DOnDestroy(pEnt, pLayer);
}

void StaticCollider2DGetBoundingBox(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR)
{
    Entity2DGetBoundingBox(pEnt, pLayer, outTL, outBR);
}

void SetStaticColliderCallbacks(struct Entity2D* pOutEnt)
{
    pOutEnt->init = &StaticColliderOnInitFn;
    pOutEnt->update = &StaticColliderUpdate;
    pOutEnt->draw = &StaticColliderDrawFn;
    pOutEnt->postPhys = &StaticColliderUpdatePostPhysics;
    pOutEnt->input = &StaticColliderInput;
    pOutEnt->onDestroy = &StaticColliderOnDestroy;
    pOutEnt->getBB = &StaticCollider2DGetBoundingBox;
}

void DeSerialize2DRectStaticColliderEntityV1(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    float w, h;
    BS_DeSerializeFloat(&w, bs);
    BS_DeSerializeFloat(&h, bs);
    struct Component2D cmp = 
    {
        .type = ETE_StaticCollider,
        .data.staticCollider.id = NULL_HANDLE,
        .data.staticCollider.bIsSensor = false,
        .data.staticCollider.onSensorOverlapBegin = NULL,
        .data.staticCollider.onSensorOverlapEnd = NULL,
        .data.staticCollider.bGenerateSensorEvents = false,
        .data.staticCollider.shape = {
            .type = PBT_Rect,
            .data.rect = {
                .w = w,
                .h = h
            }
        }
    };
    pOutEnt->components[pOutEnt->numComponents++] = cmp;
    SetStaticColliderCallbacks(pOutEnt);
}

void DeSerialize2DRectStaticColliderEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version = 0;
    BS_DeSerializeU32(&version, bs);
    switch (version)
    {
    case 1:
        DeSerialize2DRectStaticColliderEntityV1(bs,pOutEnt, pData);
        break;
    default:
        break;
    }
    
}

void Serialize2DRectStaticColliderEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    if(pInEnt->numComponents != 1)
    {
        EASSERT(false);
        printf("SerializeStaticColliderEntity, collider doesn't have exactly 1 component\n");
        return;
    }
    if(pInEnt->components[0].type != ETE_StaticCollider)
    {
        EASSERT(false);
        printf("SerializeStaticColliderEntity, component isn't of type static collider\n");
        return;
    }
    //struct PhysicsShape2D* pShape = Ph_GetStaticBodyShape2D(pInEnt->components[0].data.staticCollider.id);
    BS_SerializeU32(1, bs);
    BS_SerializeFloat(pInEnt->components[0].data.staticCollider.shape.data.rect.w, bs);
    BS_SerializeFloat(pInEnt->components[0].data.staticCollider.shape.data.rect.h, bs);
}

static struct EntitySerializerPair gPairRect = 
{
    .deserialize = &DeSerialize2DRectStaticColliderEntity,
    .serialize = &Serialize2DRectStaticColliderEntity,
};

struct EntitySerializerPair Et2D_Get2DRectStaticColliderSerializerPair()
{
    return gPairRect;
}

void DeSerialize2DCircleStaticColliderEntityV1(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    float r;
    BS_DeSerializeFloat(&r, bs);
    struct Component2D cmp = 
    {
        .type = ETE_StaticCollider,
        .data.staticCollider.id = NULL_HANDLE,
        .data.staticCollider.bIsSensor = false,
        .data.staticCollider.onSensorOverlapBegin = NULL,
        .data.staticCollider.onSensorOverlapEnd = NULL,
        .data.staticCollider.bGenerateSensorEvents = false,
        .data.staticCollider.shape = {
            .type = PBT_Circle,
            .data.circle = {
                .center = { pOutEnt->transform.position[0], pOutEnt->transform.position[1] },
                .radius = r
            }
        }
    };
    pOutEnt->components[pOutEnt->numComponents++] = cmp;
    SetStaticColliderCallbacks(pOutEnt);
}

void DeSerialize2DCircleStaticColliderEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version = 0;
    BS_DeSerializeU32(&version, bs);
    switch(version)
    {
    case 1:
        DeSerialize2DCircleStaticColliderEntityV1(bs, pOutEnt, pData);
        break;
    default:
        EASSERT(false);
    }
    
}

void Serialize2DCircleStaticColliderEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    if(pInEnt->numComponents != 1)
    {
        EASSERT(false);
        printf("SerializeStaticColliderEntity, collider doesn't have exactly 1 component\n");
        return;
    }
    if(pInEnt->components[0].type != ETE_StaticCollider)
    {
        EASSERT(false);
        printf("SerializeStaticColliderEntity, component isn't of type static collider\n");
        return;
    }
    float r = pInEnt->components[0].data.staticCollider.shape.data.circle.radius;
    BS_SerializeU32(1, bs);   // version
    BS_SerializeFloat(r, bs);
}

static struct EntitySerializerPair gPairCircle = 
{
    .deserialize = &DeSerialize2DCircleStaticColliderEntity,
    .serialize = &Serialize2DCircleStaticColliderEntity,
};

struct EntitySerializerPair Et2D_Get2DCircleStaticColliderSerializerPair()
{
    return gPairCircle;
}


static void DeSerializeEllipseEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{

}

static void SerializeEllipseEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{

}

static struct EntitySerializerPair pPairEllipse = 
{
    .deserialize = &DeSerializeEllipseEntity,
    .serialize = &SerializeEllipseEntity,
};


struct EntitySerializerPair Et2D_Get2DEllipseStaticColliderSerializerPair()
{
    return pPairEllipse;
}


static void DeSerializePolyEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{

}

static void SerializePolyEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{

}

static struct EntitySerializerPair pPairPoly = 
{
    .deserialize = &DeSerializePolyEntity,
    .serialize = &SerializePolyEntity,
};


struct EntitySerializerPair Et2D_Get2DPolygonStaticColliderSerializerPair()
{
    return pPairPoly;
}

