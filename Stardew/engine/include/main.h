#ifndef  MAIN_H
#define MAIN_H

#include "DrawContext.h"
#include "InputContext.h"
#include "Network.h"
#include <stdbool.h>

int Mn_GetScreenWidth();
int Mn_GetScreenHeight();

typedef void(*GameInitFn)(InputContext*,DrawContext*);
typedef void(*ArgHandlerFn)(int argc, char** argv, int onArg);

int EngineStart(int argc, char** argv, GameInitFn init);


/*
    TODO:
    Change functions which pass pointers to these to just use these global getters
*/
DrawContext* GetDrawContext();
InputContext* GetInputContext();

void Engine_ParseCmdArgs(int argc, char** argv, ArgHandlerFn handlerFn);

struct CommandLineArgs
{
    enum GameRole role;
    char* serverAddress;
    char* clientAddress;
    char* matchmakingServerAddress;
    char* playerName;
    bool bLogTextColoured;
    bool bIncludeLogTimeStamps;
    bool bLogTIDs;
    bool bLogToConsole;
    const char* logfilePath;
    const char* networkSimulatorConfigPath;
};

extern struct CommandLineArgs gCmdArgs;

#endif // ! MAIN_H
