#include "main.h"
#include "Log.h"
#include "Network.h"
#include "FileHelpers.h"
#include "AssertLib.h"
#include "SharedPtr.h"
#include "cwalk.h"
#include "netcode.h"
#include "ANSIColourCodes.h"
#include <string.h>
#include <stdio.h>
/*
    extends the engines cmd line args.
    An end to end test written in python will spawn a server process and then a series of short lived clients.
    For each client the script will generate a packet file of random bytes known to the test script and the client process will send it 
    to the server, which save a copy to disk and send another copy (possibly altered) back to the client, which will save that to disk.
    
    The script then compares that saved copies sent to and from the server are the same in contents to the one it generated.

    So this C program needs to :
        if client:
            - accept a file as a command line argument, load it into memory and send it to the server as a reliable packet
             once connected (monitored through connection queue). The test script will enforce a timeout. 
            - listen for a response from the server and save to a file specified on the command line
        if server:
            - listen for an incoming packet from connected clients. It will save these to a file path specified by cmd arg appended with the client number
            - send the packet back to the client
*/



struct NetTestCmdArgs
{
    char* inputFilePath;
    char* outputFilePath;
    double timeoutSeconds;
};

enum GameRole gRole;

static struct  NetTestCmdArgs gTestArgs = {
    .inputFilePath = NULL,
    .outputFilePath = NULL,
    .timeoutSeconds = -1.0 /* < 0 means no timeout */
};

void HandleNetTestArgs(int argc, char** argv, int* pOnArg)
{
    int onArg = *pOnArg;
    char* arg = argv[onArg];
    
    if(strcmp(arg, "--input_file_path") == 0)
    {
        /* 
            client:
            file containing data to send to the server
            server:
            no effect for server 
        */
        onArg++;
        char* file = argv[onArg];
        gTestArgs.inputFilePath = file;
    }
    else if(strcmp(arg, "--output_file_path") == 0)
    {
        /* 
            client:
            file that client will dump the servers response to.  
            server:
            file path to dump recieved packet to, will be appended with client index
        */
        onArg++;
        char* file = argv[onArg];
        gTestArgs.outputFilePath = file;
    }
    else if(strcmp(arg, "--server_timeout") == 0)
    {
        /*
            client:
            no effect
            server:
            process will terminate after this many seconds
        */
        onArg++;
        gTestArgs.timeoutSeconds = atof(argv[onArg]);
    }
    *pOnArg = onArg;
}

static char* GetFileOutputPath(int clientID)
{
    static char sPathBuf[256];
    char extensionBuf[16];
    strcpy(sPathBuf, gTestArgs.outputFilePath);
    return sPathBuf;
}

static int NetTestServer()
{
    double time = 0;
    double delta_time = 1.0 / 60.0;
    while(true)
    {
        struct NetworkQueueItem nci;
        /* 1.) recieve packet */
        while(NW_DequeueData(&nci))
        {
            char* path = GetFileOutputPath(nci.client);
            Log_Info(CYNHB"[Server]"CRESET" recieved packet. client: %i, size: %i. Saving to path %s", nci.client, nci.pDataSize, path);
            /* 2.) dump to file */
            FILE* pFile = fopen(path, "w");
            fwrite(nci.pData, 1, nci.pDataSize, pFile);
            fclose(pFile);

            struct NetworkQueueItem response = 
            {
                .bReliable = true,
                .client = nci.client,
                .pData = Sptr_New(nci.pDataSize, NULL),
                .pDataSize = nci.pDataSize
            };

            /* 3.) echo back to client */
            memcpy(response.pData, nci.pData, nci.pDataSize);
            Log_Info(CYNHB"[Server]"CRESET" sending response to client %i, response size %i", response.client, response.pDataSize);
            NW_EnqueueData(&response);
        }

        netcode_sleep( delta_time );

        time += delta_time;
        if(gTestArgs.timeoutSeconds > 0 && time >= gTestArgs.timeoutSeconds)
        {
            Log_Info(CYNHB"[Server]"CRESET" Timing out after %.2f seconds", gTestArgs.timeoutSeconds);
            break;
        }
    }
}


#define get_tae goto
static int NetTestClient()
{
    int size = 0;
    char* fileContents = LoadFile(gTestArgs.inputFilePath, &size);
    EASSERT(fileContents && size > 0);
    struct NetworkConnectionEvent nce;

    /* wait to connect */
    while(true)
        while(NW_DequeueConnectionEvent(&nce))
        {
            if(nce.type == NCE_ClientConnected)
            {
                get_tae connected;
            }    
        }
connected:

    /* now connected */
    Log_Info(YELHB"[Client]"CRESET" I've connected!");
    struct NetworkQueueItem item = 
    {
        .bReliable = true,
        .client = -1,
        .pData = Sptr_New(size, NULL),
        .pDataSize = size
    };
    memcpy(item.pData, fileContents, size);
    NW_EnqueueData(&item);
    Log_Info(YELHB"[Client]"CRESET" I've sent my packet to the server");
    
    while(true)
        while(NW_DequeueData(&item))
        {
            Log_Info(YELHB"[Client]"CRESET" I've recieved a response from the server. Size %i, Saving to path %s", item.pDataSize, gTestArgs.outputFilePath);
            FILE* pFile = fopen(gTestArgs.outputFilePath, "w");
            fwrite(item.pData, 1, item.pDataSize, pFile);
            fclose(pFile);
            /* don't like gotos? then you can... */
            get_tae fuck;
        }
fuck:

    return 0;
}

int main(int argc, char** argv)
{
    https://www.youtube.com/watch?v=TtYWcA6NnRU

    Engine_ParseCmdArgs(argc, argv, &HandleNetTestArgs);
    Log_Init();
    NW_Init();
    gRole = gCmdArgs.role;
    switch (gCmdArgs.role)
    {
    case GR_Client:
        return NetTestClient();
        break;
    case GR_ClientServer:
        return NetTestServer();
        break;
    default:
        Log_Error("[Net Test] INVALID ROLE %i", gCmdArgs.role);
        break;
    }
    return 0;
}
