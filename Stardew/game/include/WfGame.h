#ifndef WFGAME_H
#define WFGAME_H

/*
    Game save file handling
*/

#define VECTOR(a) a*

struct WfGameSave
{
    const char* folderPath;
    const char* saveName;
};

VECTOR(struct WfGameSave) WfGameGetSaves();
void WfSetCurrentSaveGame(struct WfGameSave* pSave);

void WfGameInit();

#endif