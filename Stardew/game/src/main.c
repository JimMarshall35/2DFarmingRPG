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
#include "WfItem.h"
#include "XMLUIGameLayer.h"
#include "WfHUD.h"


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
    WfRegisterItemScriptFunctions();
    //WfInitWorldLevels(); /* temporary - a world will be loaded as part of a game file, to be implemented in WfGame.c */
    VECTOR(struct WfGameSave) pSaves = WfGameGetSaves();
    WfSetCurrentSaveGame(&pSaves[0]);
    WfWorld_LoadLocation("House", pDC);
    WfPushHUD(pDC);

    printf("done\n");
}

enum WfExeMode
{
    NormalGame,
    CreatePersistantDataFile
};

struct CmdLineArgs
{
    enum WfExeMode mode;
    const char* outPersistantDataFilePath;
};

struct CmdLineArgs ParseCmdLineArgs(int argc, char** argv)
{
    struct CmdLineArgs args;
    args.mode = NormalGame;
    if(argc == 3)
    {
        if(strcmp(argv[1], "--outPersistantFile") == 0)
        {
            args.mode = CreatePersistantDataFile;
            args.outPersistantDataFilePath = argv[2];
        }
    }
    return args;
}

int main(int argc, char** argv)
{
    struct CmdLineArgs args = ParseCmdLineArgs(argc, argv);
    switch (args.mode)
    {
    case NormalGame:
        EngineStart(argc, argv, &GameInit);
        break;
    case CreatePersistantDataFile:
        {
            WfPersistantDataInit();
            WfNewSavePersistantData();
            WfSavePersistantDataFile(args.outPersistantDataFilePath);
        }
        break;
    default:
        break;
    }
    
}