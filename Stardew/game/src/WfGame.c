#include "WfGame.h"
#include "DynArray.h"
#include "cwalk.h"
#include "WfWorld.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static VECTOR(struct WfGameSave) gSaves = NULL;

static void PopulateSavesList()
{
    FILE * fp;
    char line[256];
    size_t len = 0;
    size_t read;

    fp = fopen("./Assets/Saves/saves.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    memset(line, 0, sizeof(line));
    while (fgets(line, sizeof(line), fp))
    {
        int lenLine = strlen(line);
        if(line[lenLine - 1] == '\n')
            line[lenLine - 1] = '\0';
        struct  WfGameSave save = 
        {
            .folderPath = malloc(lenLine + 1)
        };
        strcpy(save.folderPath, line);
        size_t basenameLen = 0;
        cwk_path_get_basename(save.folderPath, &save.saveName, &basenameLen);
        gSaves = VectorPush(gSaves, &save);
        memset(line, 0, sizeof(line));
    }

    fclose(fp);
}

void WfGameInit()
{
    gSaves = NEW_VECTOR(struct WfGameSave);
    gSaves = VectorResize(gSaves, 8);
    gSaves = VectorClear(gSaves);
    PopulateSavesList();
}

VECTOR(struct WfGameSave) WfGameGetSaves()
{
    return gSaves;
}

void WfSetCurrentSaveGame(struct WfGameSave* pSave)
{
    WfWorld_ClearLocations();
    struct WfLocation locaton;

    cwk_path_join(pSave->folderPath, "Farm.tilemap", locaton.levelFilePath, 256);
    locaton.bIsInterior = false;
    WfWorld_AddLocation(&locaton, "Farm");

    cwk_path_join(pSave->folderPath, "House.tilemap", locaton.levelFilePath, 256);
    locaton.bIsInterior = false;
    WfWorld_AddLocation(&locaton, "House");

    cwk_path_join(pSave->folderPath, "RoadToTown.tilemap", locaton.levelFilePath, 256);
    locaton.bIsInterior = false;
    WfWorld_AddLocation(&locaton, "RoadToTown");

    WfWorld_SetCurrentLocationName("Bed");
}

