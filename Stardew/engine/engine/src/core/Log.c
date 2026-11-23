#include "Log.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Thread.h"
#include "ANSIColourCodes.h"
#include "main.h"

static CrossPlatformMutex gLogMtx;
static enum LogLvl gLogLevel = LogLvl_Verbose;
static FILE* gLogFile = NULL;

char* gLogLevelNames[LogLvl_NumLevels] = 
{
    "[Verbose] ",
    "[Info]    ",
    "[Warning] ",
    "[Error]   ",
};

char* gColouredLogLevelNames[LogLvl_NumLevels] = 
{
    BHCYN"[Verbose]"CRESET" ",
    BHGRN"[Info]"CRESET"    ",
    BHYEL"[Warning]"CRESET" ",
    BHRED"[Error]"CRESET"   ",
};


void Log_SetLevel(enum LogLvl lvl)
{
    gLogLevel = lvl;
}

int Log_Init()
{
    InitMutex(&gLogMtx);
    if(gCmdArgs.logfilePath)
    {
        gLogFile = fopen(gCmdArgs.logfilePath, "w");
    }
}

void Log_DeInit()
{
    DestroyMutex(&gLogMtx);
    if(gLogFile)
    {
        fclose(gLogFile);
    }
}

int vLog_Fmt(const char* fmt, enum LogLvl lvl, va_list args)
{
    if(lvl < gLogLevel)
    {
        return 0;
    }
    char gLogBuffer[1024];
    char** levelNames = gCmdArgs.bLogTextColoured ? gColouredLogLevelNames : gLogLevelNames;
    snprintf(gLogBuffer, 512, "%s", levelNames[lvl]);
    int namelen = strlen(levelNames[lvl]);
    char* start = gLogBuffer + namelen;
    if(gCmdArgs.bIncludeLogTimeStamps)
    {
        const char* timeFmtString = gCmdArgs.bLogTextColoured ? UWHT"%02d:%02d:%02d"CRESET" " : "%02d:%02d:%02d ";
        time_t rawtime;
        struct tm *info;
        time( &rawtime );
        info = localtime( &rawtime );
        snprintf(start, 512 - namelen, timeFmtString, info->tm_hour, info->tm_min, info->tm_sec);
        namelen = strlen(gLogBuffer);
        start = gLogBuffer + namelen;
    }
    if(gCmdArgs.bLogTIDs)
    {
        //static_assert(sizeof(CrossPlatformThreadID) == sizeof(u32));
        CrossPlatformThreadID tid = GetThisThreadsID();
        const char* tidFmt = gCmdArgs.bLogTextColoured ? " "BHMAG"TID: 0x%08x"CRESET" " : "TID: %08x ";
        snprintf(start, 512 - namelen, tidFmt, (u32)tid);

        namelen = strlen(gLogBuffer);
        start = gLogBuffer + namelen;
    }

    vsnprintf(start, 512 - namelen, fmt, args);
    int r = 0;
    LockMutex(&gLogMtx);
    if(gCmdArgs.bLogToConsole)
    {
        r = printf("%s\n", gLogBuffer);
    }
    if(gLogFile)
    {
        fprintf(gLogFile, "%s\n", gLogBuffer);
    }
    UnlockMutex(&gLogMtx);
    return r;
}

int Log_Verbose(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Verbose, args);
    va_end(args);
    return r;
}

int Log_Fmt(const char* fmt, enum LogLvl lvl, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, lvl, args);
    va_end(args);
    return r;
}

int Log_Info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Info, args);
    va_end(args);
    return r;
}

int Log_Warning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Warning, args);
    va_end(args);
    return r;
}

int Log_Error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Error, args);
    va_end(args);
    return r;
}