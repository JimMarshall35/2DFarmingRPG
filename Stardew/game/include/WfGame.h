#ifndef WFGAME_H
#define WFGAME_H

/*
    Game save file handling
*/

#define VECTOR(a) a*

/* 
    a very important struct - the data for a save game that is not saved by serializing the tilemap and object layers.
    Stuff like:
        - The in-game date: season, day of season, year
        - Player inventory
        - Player stats
        - Story progression
*/
struct WfNonEntityGameData
{
    int f;
};


struct WfGameSave
{
    const char* folderPath;
    const char* saveName;
};

VECTOR(struct WfGameSave) WfGameGetSaves();
void WfSetCurrentSaveGame(struct WfGameSave* pSave);

void WfGameInit();

#endif