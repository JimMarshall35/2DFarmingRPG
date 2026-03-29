#include "WfEntities.h"

/* Player Start */
#include "WfPlayerStart.h"

/* Exit */
#include "WfExit.h"

/* Wooded Area */
#include "WfWoodedArea.h"

/* Debris Field */
#include "WfDebrisField.h"

/* Tree */
#include "WfTree.h"

/* Debris */
#include "WfDebris.h"

/* Player */
#include "WfPlayer.h"

/* Pickup */
#include "WfItemPickup.h"

static struct EntitySerializerPair gPlayerStartSerializer = { .serialize = &WfSerializePlayerStartEntity, .deserialize = &WfDeSerializePlayerStartEntity };
static struct EntitySerializerPair gExitSerializer        = { .serialize = &WfSerializeExitEntity,        .deserialize = &WfDeSerializeExitEntity };
static struct EntitySerializerPair gWoodedAreaSerializer  = { .serialize = &WfSerializeWoodedAreaEntity,  .deserialize = &WfDeSerializeWoodedAreaEntity };
static struct EntitySerializerPair gDebrisFieldSerializer = { .serialize = &WfSerializeDebrisFieldEntity, .deserialize = &WfDeSerializeDebrisFieldEntity };

static struct EntitySerializerPair gTreeSerializer        = { .serialize = &WfSerializeTreeEntity,         .deserialize = &WfDeSerializeTreeEntity };
static struct EntitySerializerPair gRockSerializer        = { .serialize = &WfSerializeDebrisEntity,         .deserialize = &WfDeSerializeDebrisEntity }; 
static struct EntitySerializerPair gPlayerSerializer      = { .serialize = &WfSerializePlayerEntity,       .deserialize = &WfDeSerializePlayerEntity };
static struct EntitySerializerPair gPickupSerializer      = { .serialize = &WfDeSerializeItemPickupEntity, .deserialize = &WfDeSerializeItemPickupEntity };

void WfRegisterEntityTypes()
{
    Et2D_RegisterEntityType(WfEntityType_PlayerStart, &gPlayerStartSerializer);
    Et2D_RegisterEntityType(WfEntityType_Exit, &gExitSerializer);
    Et2D_RegisterEntityType(WfEntityType_WoodedArea, &gWoodedAreaSerializer);
    Et2D_RegisterEntityType(WfEntityType_DebrisField, &gDebrisFieldSerializer);
    Et2D_RegisterEntityType(WfEntityType_Tree, &gTreeSerializer);
    Et2D_RegisterEntityType(WfEntityType_Rock, &gRockSerializer);
    Et2D_RegisterEntityType(WfEntityType_Player, &gPlayerSerializer);
    Et2D_RegisterEntityType(WfEntityType_ItemPickup, &gPickupSerializer);
}