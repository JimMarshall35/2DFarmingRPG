#ifndef STARDEWSHAREDLIB_H
#define STARDEWSHAREDLIB_H

/* Cross playform abstraction for explicitly loading functions from shared libraries */

struct SharedLib;

struct SharedLib* SharedLib_LoadSharedLib(const char* path);

void* SharedLib_GetProc(struct SharedLib* pSharedLib, const char* procName);


#endif