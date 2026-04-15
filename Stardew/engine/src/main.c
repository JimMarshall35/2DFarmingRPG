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
#include "Network.h"
#include "AssertLib.h"
#include "Audio.h"
#include "cwalk.h"
#include "Platform.h"



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
    gCmdArgs.role = GR_Singleplayer;
    gCmdArgs.serverAddress = "127.0.0.1:40000";
    gCmdArgs.clientAddress = "0.0.0.0";
    gCmdArgs.matchmakingServerAddress = NULL;
    gCmdArgs.playerName = "Jim";
    gCmdArgs.bLogTextColoured = true;
    gCmdArgs.bIncludeLogTimeStamps = true;
    gCmdArgs.bLogTIDs = true;
    gCmdArgs.logfilePath = NULL;
    gCmdArgs.networkSimulatorConfigPath = NULL;
    gCmdArgs.bLogToConsole = true;
    gCmdArgs.assetsDir = "./WfAssets";
    gCmdArgs.configDir = "./WfAssets";

    if(argc > 1)
    {
        for(int i=1; i <argc; i++)
        {
            
            Log_Info("Cmd arg %i: %s", i, argv[i]);
            if(strcmp(argv[i], "--name") == 0 || strcmp(argv[i], "-n") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.playerName = argv[i];
            }
            if(strcmp(argv[i], "--role") == 0 || strcmp(argv[i], "-r") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                if(strcmp(argv[i], "server") == 0 || strcmp(argv[i], "s") == 0)
                {
                    gCmdArgs.role = GR_ClientServer;
                }
                else if(strcmp(argv[i], "client") == 0 || strcmp(argv[i], "c") == 0)
                {
                    gCmdArgs.role = GR_Client;
                }
            }
            else if(strcmp(argv[i], "--server_address") == 0 || strcmp(argv[i], "-s") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.serverAddress = argv[i];
            }
            else if(strcmp(argv[i], "--client_address") == 0 || strcmp(argv[i], "-c") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.clientAddress = argv[i];
            }
            else if(strcmp(argv[i], "--matchmaking_address") == 0 || strcmp(argv[i], "-m") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.matchmakingServerAddress = argv[i];
            }
            else if(strcmp(argv[i], "--log_level") == 0 || strcmp(argv[i], "-l") == 0)
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
            else if(strcmp(argv[i], "--network_sim_config") == 0)
            {
                EASSERT(i + 1 < argc);
                i++;
                Log_Info("Cmd arg %i: %s", i, argv[i]);
                gCmdArgs.networkSimulatorConfigPath = argv[i];
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

static void DoNetworkQueues()
{
    /* placeholder */
    struct NetworkConnectionEvent event; 
    while(NW_DequeueConnectionEvent(&event))
    {
        Log_Info("NETWORK EVENT: %s CLIENT: %i", event.type == NCE_ClientConnected ? "NCE_ClientConnected" : "NCE_ClientDisconnected", event.client);
    }
}

int EngineStart(int argc, char** argv, GameInitFn init, ArgHandlerFn argHandler)
{
    Engine_ParseCmdArgs(argc, argv, argHandler);
    Log_Init();
    NW_Init();
    int audioSystemInitCode = Au_Init();
    Log_Info("Initialized audio, return code: %i", audioSystemInitCode);

    Log_Verbose("testing libxml version...");
    LIBXML_TEST_VERSION
    Log_Verbose("hello world");
   
    Platform_InitWindow();

    
    Log_Verbose("done");

    double accumulator = 0;
    double lastUpdate = 0;
    double slice = 1.0 / TARGET_FPS;

    Log_Verbose("initialising draw context");
    gDrawContext = Dr_InitDrawContext();
    Log_Verbose("done");
    Log_Verbose("initial screen dims change");
    Dr_OnScreenDimsChange(&gDrawContext, SCR_WIDTH, SCR_HEIGHT);
    Log_Verbose("done");
    Log_Verbose("initialising input context");
    gInputContext = In_InitInputContext();
    Log_Verbose("done");
    Log_Verbose("Initialising game framework");
    GF_InitGameFramework();
    Log_Verbose("done");
    Log_Verbose("initialising image registry");

    char pathBuf[256];
    cwk_path_join(gCmdArgs.assetsDir, "ImageFiles.json", pathBuf, 256);
    IR_InitImageRegistry(pathBuf);
    Log_Verbose("done");
    Log_Verbose("initialising atlas");
    At_Init();
    Log_Verbose("done");
    Log_Verbose("initialising UI");
    UI_Init();
    Log_Verbose("done");
    Log_Verbose("initialising scripting");
    Sc_InitScripting();
    Log_Verbose("done");

    init(&gInputContext, &gDrawContext);
    
    double frameTimeTotal = 0.0;
    int onCount = 0;
    int numCounts = 60;
    while (!Platform_ShouldWindowClose())
    {
        double time = Platform_GetElapsedSeconds();
        double delta = time - lastUpdate;
        lastUpdate = time;
        accumulator += delta;
        while (accumulator > slice)
        {
            Platform_PollEvents();
            //DoNetworkQueues();
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
