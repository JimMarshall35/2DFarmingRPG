# Entities

The Game2D layer has an entity system whereby entity types are defined by a numerical ID, a set of callbacks, an object layer index, a transform, a user data pointer (or handle) and a list of components.

The entity callbacks are:
    - Init
    - Draw
    - Input
    - Update
    - PostPhysics
    - Draw
    - OnDestroy
    - GetBoundingBox
    - GetSortPos (defines order in which entities within an object layer are drawn)

There are a fixed number of different types of components, as of writing these are:
    - Sprite
        - These have their own offset transform from the entities transform
    - StaticCollider
    - KinematicCollider
    - TextSprite
    - AnimatedSprite

This will be subject to change. The game won't be able to define new components so the engine should provide a good set that cover everything.

Entities can have more than one of a given component. In the case of sprites they're drawn in the order that they appear in the entity list.

Some kind of lua scripting system might be added.

Your game will define a list of entity serializers which can serialize a particular type of entity:

```c

#include "Entities.h"

enum WfEntityTypes
{
    // start the enum containing your entities at EBET_Last and let it auto increment
    WfEntityType_PlayerStart = EBET_Last, 
}
static void DeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    BS_SerializeU32(1, bs); // version
    /* 
        load data fromthe file here and populate pOutEnt with it.
        Data common to all entity types will already be loaded into pOutEnt,
        you'll want to create a user data struct with entity specific data in and and set the entities userdata pointer/handle to it..
        You'll also want to set any enitity callbacks you want to overrider here a well.
        In these callbacks you should always call the default callback, In Entities.h these are:

        void Entity2DOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer);
        void Entity2DUpdate(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
        void Entity2DUpdatePostPhysics(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT);
        void Entity2DDraw(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex);
        void Entity2DInput(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context);
        void Entity2DOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer);
        void Entity2DGetBoundingBox(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR);
        float Entity2DGetSortVal(struct Entity2D* pEnt);

        Deserialize data with the functions in BinarySerializer.h.

        YOU NEED TO LOAD THE DATA FOR AND CREATE THE COMPONENTS HERE.
        (This could well change in the future)

        Good practice is to serialize a version number first.
    */
}

static void SerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    
    u32 version = 0;
    BS_DeSerializeU32(&version, bs);
    switch (version)
    {
    case 1:
        /*
            save entity type specific data here in such a way that Deserialize2D can load it here

            YOU NEED TO SAVE DATA FOR THE COMPONENTS HERE
        */
        break;
    default:
        EASSERT(false);
        break;
    }
}

static struct EntitySerializerPair gPlayerSerializer      = { .serialize = &SerializePlayerEntity, .deserialize = &DeSerializePlayerEntity };

int main()
{
    /* call this somewhere near the start of the games main function */
    Et2D_RegisterEntityType(WfEntityType_PlayerStart, &gPlayerStartSerializer);
}


```

See AssetTools.md to see how you can create a level full of entities that the engine can load.

