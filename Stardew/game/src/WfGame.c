#include "WfGame.h"
#include "DynArray.h"
#include "cwalk.h"
#include "WfWorld.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "WfPersistantGameData.h"
#include "WfCmdArgs.h"
#include "FilesystemUtils.h"

static VECTOR(struct WfGameSave) gSaves = NULL;

char gTempFolderPath[512];

static void PopulateSavesList()
{
    FILE * fp;
    char line[256];
    char line2[256];

    size_t len = 0;
    size_t read;
    cwk_path_join(gGameArgs.savesDir, "saves.txt", line, 256);

    fp = fopen(line, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    memset(line, 0, sizeof(line));
    while (fgets(line, sizeof(line), fp))
    {
        cwk_path_join(gGameArgs.savesDir, line, line2, 256);
        int lenLine = strlen(line2);
        if(line2[lenLine - 1] == '\n')
            line2[lenLine - 1] = '\0';
        struct  WfGameSave save = 
        {
            .folderPath = malloc(lenLine + 1)
        };
        strcpy(save.folderPath, line2);
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

void WfCopySaveToTempFolder(struct WfGameSave* pSave)
{
    char bufSrc[512];
    char bufDst[512];
    cwk_path_join(pSave->folderPath, "Farm.tilemap", bufSrc, 512);
    cwk_path_join(gTempFolderPath, "Farm.tilemap", bufDst, 512);
    FS_CopyFile(bufSrc, bufDst);

    cwk_path_join(pSave->folderPath, "House.tilemap", bufSrc, 512);
    cwk_path_join(gTempFolderPath, "House.tilemap", bufDst, 512);
    FS_CopyFile(bufSrc, bufDst);

    cwk_path_join(pSave->folderPath, "RoadToTown.tilemap", bufSrc, 512);
    cwk_path_join(gTempFolderPath, "RoadToTown.tilemap", bufDst, 512);
    FS_CopyFile(bufSrc, bufDst);

    cwk_path_join(pSave->folderPath, "Town.tilemap", bufSrc, 512);
    cwk_path_join(gTempFolderPath, "Town.tilemap", bufDst, 512);
    FS_CopyFile(bufSrc, bufDst);


    cwk_path_join(pSave->folderPath, "Persistant.game", bufSrc, 512);
    cwk_path_join(gTempFolderPath, "Persistant.game", bufDst, 512);
    FS_CopyFile(bufSrc, bufDst);
}

void WfSetCurrentSaveGame(struct WfGameSave* pSave)
{
    WfCopySaveToTempFolder(pSave);

    WfWorld_ClearLocations();
    struct WfLocation location;

    cwk_path_join(gTempFolderPath, "Farm.tilemap", location.levelFilePath, 256);
    location.bIsInterior = false;
    WfWorld_AddLocation(&location, "Farm");

    cwk_path_join(gTempFolderPath, "House.tilemap", location.levelFilePath, 256);
    location.bIsInterior = false;
    WfWorld_AddLocation(&location, "House");

    cwk_path_join(gTempFolderPath, "RoadToTown.tilemap", location.levelFilePath, 256);
    location.bIsInterior = false;
    WfWorld_AddLocation(&location, "RoadToTown");

    cwk_path_join(gTempFolderPath, "Town.tilemap", location.levelFilePath, 256);
    location.bIsInterior = false;
    WfWorld_AddLocation(&location, "Town");

    char buf[256];
    cwk_path_join(gTempFolderPath, "Persistant.game", buf, 256);
    WfLoadPersistantDataFile(buf);

    WfWorld_SetCurrentLocationName("Bed");
}

void WfSetupTempFolder()
{
    FS_GetTempDir(gTempFolderPath, 512);
}

void WfTeardownTempFolder()
{
    FS_DeleteDirRecursive(gTempFolderPath);
}
