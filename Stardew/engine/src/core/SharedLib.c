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

void* SharedLib_GetCurrentlyLoadedFn(const char* procName)
{
    void* pFn = dlsym(RTLD_DEFAULT, procName);

    return pFn;
}

#else if defined(__WIN32__)

#include <windows.h>
#include <psapi.h>
#include <processthreadsapi.h>

struct SharedLib
{
    HMODULE handle;
};

struct SharedLib* SharedLib_LoadSharedLib(const char* path)
{
    struct SharedLib* pLib = malloc(sizeof(struct SharedLib));
    memset(pLib, 0, sizeof(struct SharedLib));
    pLib->handle = LoadLibraryExA(path, NULL, 0);
    return pLib;
}

void* SharedLib_GetProc(struct SharedLib* pSharedLib, const char* procName)
{
    return GetProcAddress(pSharedLib->handle, procName);
}

void* SharedLib_GetCurrentlyLoadedFn(const char* procName)
{
    HMODULE modules[1024];
    DWORD needed;

    if (!EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed))
        return NULL;

    int count = needed / sizeof(HMODULE);

    for (int i = 0; i < count; i++) 
    {
        FARPROC proc = GetProcAddress(modules[i], procName);
        if (proc)
            return (void*)proc;
    }

    return NULL;
}

#endif

