#include "main.h"
#include <stdio.h>
#include "DynArray.h"
#include "GameFramework.h"
#include "XMLUIGameLayer.h"
#include "ImageFileRegstry.h"
#include "Atlas.h"
#include "Widget.h"
#include "Scripting.h"
#include <string.h>
#include "PlatformDefs.h"
#include <libxml/parser.h>
#include "Log.h"
#include "AssertLib.h"
#include "Audio.h"
#include "cwalk.h"
#include "Platform.h"
#include "Profile.h"


InputContext gInputContext;
DrawContext gDrawContext;
STARDEW_API struct CommandLineArgs gCmdArgs;

DrawContext* GetDrawContext()
{
    return &gDrawContext;
}

InputContext* GetInputContext()
{
    return &gInputContext;
}

int Mn_GetScreenWidth()
{
    return gDrawContext.screenWidth;
}

int Mn_GetScreenHeight()
{
    return gDrawContext.screenHeight;
}


typedef void(*GameInitFn)(InputContext*,DrawContext*);


void Engine_ParseCmdArgs(int argc, char** argv, ArgHandlerFn handlerFn)
{
    gCmdArgs.bLogTextColoured = true;
    gCmdArgs.bIncludeLogTimeStamps = true;
    gCmdArgs.bLogTIDs = true;
    gCmdArgs.logfilePath = NULL;
    gCmdArgs.bLogToConsole = true;
    gCmdArgs.assetsDir = "./WfAssets";
    gCmdArgs.configDir = "./WfAssets";

    if(argc > 1)
    {
        for(int i=1; i <argc; i++)
        {
            Log_Info("Cmd arg %i: %s", i, argv[i]);

            if(strcmp(argv[i], "--log_level") == 0 || strcmp(argv[i], "-l") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                if(strcmp(argv[i], "verbose") == 0 || strcmp(argv[i], "v") == 0)
                {
                    Log_SetLevel(LogLvl_Verbose);
                }
                else if(strcmp(argv[i], "info") == 0 || strcmp(argv[i], "i") == 0)
                {
                    Log_SetLevel(LogLvl_Info);
                }
                else if(strcmp(argv[i], "warning") == 0 || strcmp(argv[i], "w") == 0)
                {
                    Log_SetLevel(LogLvl_Warning);
                }
                else if(strcmp(argv[i], "error") == 0 || strcmp(argv[i], "e") == 0)
                {
                    Log_SetLevel(LogLvl_Error);
                }
            }
            else if(strcmp(argv[i], "--disable_log_colour") == 0)
            {
                gCmdArgs.bLogTextColoured = false;
            }
            else if(strcmp(argv[i], "--disable_log_timestamp") == 0)
            {
                gCmdArgs.bIncludeLogTimeStamps = false;
            }
            else if(strcmp(argv[i], "--logfile") == 0 || strcmp(argv[i], "--lf") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.logfilePath = argv[i];
            }
            else if(strcmp(argv[i], "--disable_log_tid") == 0)
            {
                gCmdArgs.bLogTIDs = false;
            }
            else if(strcmp(argv[i], "--disable_console_log") == 0)
            {
                gCmdArgs.bLogToConsole = false;
            }
            else if(strcmp(argv[i], "--assetsDir") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.assetsDir = argv[i];
            }
            else if(strcmp(argv[i], "--configDir") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.configDir = argv[i];
            }
            else if(handlerFn)
            {
                handlerFn(argc, argv, &i);
            }
        }
    }
}


int EngineStart(int argc, char** argv, GameInitFn init, ArgHandlerFn argHandler)
{
    Engine_ParseCmdArgs(argc, argv, argHandler);
    Log_Init();
    int audioSystemInitCode = Au_Init();
    LIBXML_TEST_VERSION
   
    Platform_InitWindow();

    double accumulator = 0;
    double slice = 1.0 / TARGET_FPS;

    gDrawContext = Dr_InitDrawContext();
    Dr_OnScreenDimsChange(&gDrawContext, SCR_WIDTH, SCR_HEIGHT);
    gInputContext = In_InitInputContext();
    GF_InitGameFramework();
    char pathBuf[256];
    cwk_path_join(gCmdArgs.assetsDir, "ImageFiles.json", pathBuf, 256);
    IR_InitImageRegistry(pathBuf);
    At_Init();
    UI_Init();
    Sc_InitScripting();

    init(&gInputContext, &gDrawContext);
    
    double frameTimeTotal = 0.0;
    int onCount = 0;
    int numCounts = 60;
    double lastUpdate = Platform_GetElapsedSeconds();
    while (!Platform_ShouldWindowClose())
    {
        double time = Platform_GetElapsedSeconds();
        double delta = time - lastUpdate;
        lastUpdate = time;
        accumulator += delta;
        while (accumulator > slice)
        {
            FRAME_MARK
            Platform_PollEvents();
            GF_InputGameFramework(&gInputContext);
            GF_UpdateGameFramework((float)slice);
            In_EndFrame(&gInputContext);
            GF_EndFrame(&gDrawContext, &gInputContext);
            accumulator -= slice;
        }

        gDrawContext.ClearScreen();
        
        GF_DrawGameFramework(&gDrawContext);
        Platform_SwapBuffers();
        
        frameTimeTotal += delta;
        onCount++;
        if(onCount == numCounts)
        {
            onCount = 0;
            //printf("frame time: %f\n", 1.0 / (frameTimeTotal / (double)numCounts));
            frameTimeTotal = 0.0f;
        }
    }

    Sc_DeInitScripting();
    IR_DestroyImageRegistry();
    GF_DestroyGameFramework();

    Platform_DeInit();
    Log_DeInit();

    if(audioSystemInitCode == 0)
    {
        Au_DeInit();
    }
}



void GameInit(InputContext* pIC, DrawContext* pDC)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct XMLUIGameLayerOptions options;
    options.bLoadImmediately = true;
    char buf[256];
    cwk_path_join(buf, "test.xml", buf, 256);
    options.xmlPath = buf;
    options.pDc = pDC;
    Log_Verbose("making xml ui layer");
    XMLUIGameLayer_Get(&testLayer, &options);
    Log_Verbose("done");
    Log_Verbose("pushing framework layer");
    GF_PushGameFrameworkLayer(&testLayer);
    Log_Verbose("done");
}
