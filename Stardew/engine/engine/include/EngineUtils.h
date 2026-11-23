#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <string.h>

/**
    @file EngineUtils.h
    @brief
    Miscellaneous utility macros and functions that don't fit in other headers
*/

#ifndef ZeroMemory

#define ZeroMemory(dest, numBytes) memset(dest, 0, numBytes)

#endif


#endif
