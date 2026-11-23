#ifndef STARDEW_LOG_H
#define STARDEW_LOG_H

#include <stdarg.h>

enum LogLvl
{
    LogLvl_Verbose,
    LogLvl_Info,
    LogLvl_Warning,
    LogLvl_Error,
    LogLvl_NumLevels
};

int vLog_Fmt(const char* fmt, enum LogLvl lvl, va_list args);

int Log_Fmt(const char* fmt, enum LogLvl lvl, ...);

int Log_Verbose(const char* fmt, ...);

int Log_Info(const char* fmt, ...);

int Log_Warning(const char* fmt, ...);

int Log_Error(const char* fmt, ...);

int Log_Init();

void Log_DeInit();

/** @brief logs with a level lower than this will be dropped */
void Log_SetLevel(enum LogLvl lvl);

#endif
