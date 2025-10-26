#include "main.h"
#include "GameFramework.h"
#include <string.h>
#include "Game2DLayer.h"
#include "XMLUIGameLayer.h"
#include "DynArray.h"
#include "Entities.h"
#include "EntityQuadTree.h"
#include "WfEntities.h"
#include "Physics2D.h"
#include "WfInit.h"
#include "Random.h"
#include "WfGameLayerData.h"
#include "WfGameLayer.h"
#include "WfWorld.h"
#include "WfGame.h"

void WfEngineInit()
{
    unsigned int seed = Ra_SeedFromTime();
    printf("seed: %u\n", seed);
    Ph_Init();
    InitEntity2DQuadtreeSystem();
    Et2D_Init(&WfRegisterEntityTypes);
}

void GameInit(InputContext* pIC, DrawContext* pDC)
{
    WfGameInit();
    WfEngineInit();
    WfInit();
    //WfInitWorldLevels(); /* temporary - a world will be loaded as part of a game file, to be implemented in WfGame.c */
    VECTOR(struct WfGameSave) pSaves = WfGameGetSaves();
    WfSetCurrentSaveGame(&pSaves[0]);
    WfWorld_LoadLocation("House", pDC);
    printf("done\n");
}

int main(int argc, char** argv)
{

    EngineStart(argc, argv, &GameInit);
}