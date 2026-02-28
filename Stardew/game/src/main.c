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
#include "WfUI.h"
#include "WfScriptFunctions.h"
#include "ANSIColourCodes.h"
#include "Log.h"
#include "WfVersion.h"
#include "WfPersistantGameData.h"
#include "WfCmdArgs.h"

enum WfExeMode
{
    NormalGame,
    CreatePersistantDataFile
};

/// @brief parsed before the engine is initialized
struct PreEngineCmdLineArgs
{
    enum WfExeMode mode;
    const char* outPersistantDataFilePath;
};


struct WfGameCmdArgs gGameArgs = 
{
    .savesDir = "./WfAssets/Saves"
};

/* A policy of containment means we must keep this function static, lest the spectre of communism haunt other translation units. */
static void KarlMarx()
{
    const char* marx = RED
    "\"Die Proletarier haben nichts zu verlieren als ihre Ketten.\n"
    "Sie haben eine Welt zu gewinnen.\n"
    "Proletarier aller Länder, vereinigt Euch!\"\n"
    "        -Karl Marx\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣀⣀⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⣠⣟⣋⣩⣩⣤⣄⣀⣀⣀⣉⣉⠙⠛⠒⠦⢤⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠈⠋⠉⠉⠉⠉⠉⠉⠉⠛⠛⠻⠭⢿⣲⣤⣄⣀⠉⠓⠦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠓⠻⢽⣲⣄⡀⠉⠳⢤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣾⣿⣷⡄⠀⠈⠑⠿⡲⣄⠀⠙⣷⣄⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣾⣿⣿⣿⣿⡿⠁⠀⠀⠀⠀⠈⠻⣷⣄⠈⠻⡳⣄⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣾⣿⣿⣿⣿⣿⣿⡟⠁⠀⠀⠀⠀⠀⠀⠀⠈⢿⣣⡀⠀⠙⣦⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣾⣿⣿⣿⣿⣿⣿⣿⡿⠛⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢳⣵⡀⠀⠘⣇⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⢀⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣷⡀⠀⢸⠀⠀⠀\n"
    "⠀⠀⠀⢴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⠋⠎⢈⢣⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⣿⡇⠀⢸⡧⠀⠀\n"
    "⠀⠀⠀⠀⠹⣿⣿⣿⣿⣿⡿⠟⠉⢿⣆⠘⢧⡹⣽⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⡿⠇⠀⢸⠁⠀⠀\n"
    "⠀⠀⠀⠀⠀⠈⢿⣿⠟⠃⠀⠀⠀⠀⠹⣧⡄⢽⢮⡻⣷⡀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣿⡿⠀⠀⠀⡼⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⠘⢳⣄⣷⣝⠪⣝⣆⠀⠀⠀⠀⠀⢀⣴⣏⣼⠁⠀⢀⣾⠃⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⠤⣤⢤⣤⣽⣷⣛⣷⣌⠙⢷⣤⣤⣤⣾⡽⠛⠉⠁⠀⣀⣾⠃⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣾⠇⠀⢈⢽⣿⣋⠉⢿⣮⣻⢷⡂⠙⢆⠀⠀⠀⢀⢠⠀⣩⠟⠁⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⣠⣶⣿⢥⣠⣔⡿⠛⠛⠷⠧⠤⣿⣿⣷⣝⠳⠬⣳⣤⣖⡾⠴⠋⠁⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⣠⡞⠓⠦⣤⣿⣿⣭⣿⣿⣿⠅⠀⠀⠀⠀⠀⠀⠙⢷⣝⢳⣄⠀⠹⣦⡀⣠⠤⢤⣄⠀⠀⠀⠀⠀⠀\n"
    "⠀⢀⣤⠿⣼⣦⣴⣿⠿⠛⠛⡿⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣶⣿⠛⠊⠛⠳⣧⣀⣘⡽⢦⡄⠀⠀⠀⠀\n"
    "⠀⣿⣿⠀⠘⣿⡞⢊⠠⢾⣇⡄⠀⠱⣄⠀⠀⠀⠀⠀⠀⠀⠀⢀⡞⠁⠀⣼⣶⡄⠙⢺⡿⢀⡘⣿⠀⠀⠀⠀\n"
    "⡾⡍⠙⢾⡀⠈⣹⡳⣾⣿⣿⡿⡄⠀⢸⢧⠀⠀⠀⠀⠀⠀⢠⡏⠀⠀⣠⣻⡋⠹⣿⡉⠀⣠⠾⠛⣷⠀⠀⠀\n"
    "⣻⢯⡦⡀⠙⣿⣧⣿⣿⡟⠯⠀⠀⠀⠘⢸⡀⠀⠀⠀⠀⠀⡟⠀⠀⠀⣸⣿⣷⣤⣷⢥⠞⡁⡠⣤⣿⡀⠀⠀\n"
    "⡇⢀⣙⠺⣾⣽⣿⡿⡿⣇⠉⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⠀⡇⠀⠀⠀⠁⠠⠿⣿⣟⢳⣄⡽⠟⠉⣉⣿⠀⠀\n"
    "⣽⣷⣿⣄⣿⣿⣿⡟⠀⢹⠀⠀⠀⠀⠀⢨⣧⡀⠀⠀⠀⠀⢻⣀⠀⠀⠀⠀⢺⠋⡿⣦⣯⣂⣴⣮⣟⣧⡀⠀\n"
    "⠹⣿⣿⣿⣿⣿⢏⠀⡀⠀⠀⠀⠀⠀⠐⠻⠍⠳⡄⠀⠀⢠⡾⠾⡀⠀⠀⠀⠀⠀⠈⠻⡄⠹⣥⡹⣿⠹⣆ \n"
    "⠀⠈⠉⠛⠋⠉⠓⠾⠤⣤⣶⡃⠀⠀⠀⠀⠀⠀⠘⢦⣠⠏⠀⠀⠸⣷⣄⡀⠀⡠⠲⣀⡴⠛⢧⠹⠜⢾⣷⠿\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢳⡁⢀⣀⠀⠀⠀⠀⢀⡴⠃⠀⠀⠀⠀⠀⠀⠀⢛⡷⠛⠁⠀⠀⠀⠳⠖⠛⠁⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣆⢐⠀⠀⠀⣠⠞⠁⠀⠀⠀⠀⠀⢠⠃⢠⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢦⠀⢀⡔⠁⠀⠀⠀⠀⠀⠀⠀⢰⣠⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣳⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⡼⠁⠙⣦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠁⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    CRESET;
    printf(marx);
}

void WfPrintVersion()
{
    Log_Info(WfGetVersionString());
}


void WfEngineInit()
{
    unsigned int seed = Ra_SeedFromTime();
    Log_Info("seed: %u", seed);
    Ph_Init();
    InitEntity2DQuadtreeSystem();
    Et2D_Init(&WfRegisterEntityTypes);
}

void GameInit(InputContext* pIC, DrawContext* pDC)
{
    //KarlMarx();
    WfPrintVersion();
    WfGameInit();
    WfEngineInit();
    WfInit();
    WfRegisterScriptFunctions();
    //WfInitWorldLevels(); /* temporary - a world will be loaded as part of a game file, to be implemented in WfGame.c */
    VECTOR(struct WfGameSave) pSaves = WfGameGetSaves();
    WfSetCurrentSaveGame(&pSaves[0]);
    WfWorld_LoadLocation("House", pDC);
    WfPushHUD(pDC);
}

/// @brief parse args outside of the game engine mechanism, before the engine is initialized
static struct PreEngineCmdLineArgs ParsePreCmdLineArgs(int argc, char** argv)
{
    struct PreEngineCmdLineArgs args;
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
void ParseGameCmdLineArgs(int argc, char** argv, int* onArg)
{
    if(strcmp(argv[*onArg], "--savesDir") == 0)
    {
        int arg = *onArg;
        arg++;
        gGameArgs.savesDir = argv[arg];
        *onArg = arg;
    }
}

int main(int argc, char** argv)
{
    struct PreEngineCmdLineArgs args = ParsePreCmdLineArgs(argc, argv);
    switch (args.mode)
    {
    case NormalGame:
        EngineStart(argc, argv, &GameInit, &ParseGameCmdLineArgs);
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