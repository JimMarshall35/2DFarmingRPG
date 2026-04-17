#pragma once

#include "Platform.h"

#ifdef _WIN64 
#define GAME_PLATFORM_WINDOWS_64
#define GAME_PTR_SIZE 8
#elif _WIN32
#define GAME_PLATFORM_WINDOWS_32
#define GAME_PTR_SIZE 4
#elif __GNUC__
#define GAME_PLATFORM_LINUX
#define GAME_PTR_SIZE __SIZEOF_POINTER__
#endif

#define GAME_GL_API_TYPE_ES 1
#define GAME_GL_API_TYPE_CORE 2

//#define GAME_GL_API_TYPE "OPEN_GL"

#ifndef GAME_GL_API_TYPE
    #define GAME_GL_API_TYPE GAME_GL_API_TYPE_CORE
#endif

