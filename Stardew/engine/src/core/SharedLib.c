#include "SharedLib.h"
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)

#include <dlfcn.h>

struct SharedLib
{
    void* handle;
};

struct SharedLib* SharedLib_LoadSharedLib(const char* path)
{
    struct SharedLib* pLib = malloc(sizeof(struct SharedLib));
    memset(pLib, 0, sizeof(struct SharedLib));
    pLib->handle = dlopen(path, 0);
    return pLib;
}

void* SharedLib_GetProc(struct SharedLib* pSharedLib, const char* procName)
{
    return dlsym(pSharedLib->handle, procName);
}


#else if defined(__WIN32__)

#include <libloaderapi.h>

struct SharedLib
{
    HMODULE handle;
};

struct SharedLib* SharedLib_LoadSharedLib(const char* path)
{
    struct SharedLib* pLib = malloc(sizeof(struct SharedLib));
    memset(pLib, 0, sizeof(struct SharedLib));
    pLib->handle = LoadLibraryA(path, NULL, 0);
    return pLib;
}

void* SharedLib_GetProc(struct SharedLib* pSharedLib, const char* procName)
{
    return GetProcAddress(pSharedLib->handle, procName);
}

#endif

