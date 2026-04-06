#ifndef STARDEWSHAREDLIB_H
#define STARDEWSHAREDLIB_H

/* Cross playform abstraction for explicitly loading functions from shared libraries */

struct SharedLib;

struct SharedLib* SharedLib_LoadSharedLib(const char* path);

void* SharedLib_GetProc(struct SharedLib* pSharedLib, const char* procName);

/// @brief hook a function by name that's currently loaded
/// @param procName 
/// @return function pointer
void* SharedLib_GetCurrentlyLoadedFn(const char* procName);

// we build with CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS but this doesn't work properly for global variables
// such as gCmdArgs, so sometimes we need to manualy add this in like the primitive windows programmers that we are
#ifdef WIN32
#ifdef STARDEW_EXPORTS
    #define STARDEW_API __declspec(dllexport)
#else
    #define STARDEW_API __declspec(dllimport)
#endif
#else 
#define STARDEW_API __attribute__((visibility("default")))
#endif

#endif