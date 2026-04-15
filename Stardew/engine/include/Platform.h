#ifndef STARDEW_PLATFORM_H
#define STARDEW_PLATFORM_H

#define STARDEW_PLATFORM_GLFW3 1
#define STARDEW_PLATFORM_SDL2  2

#ifndef STARDEW_PLATFORM
#define STARDEW_PLATFORM STARDEW_PLATFORM_GLFW3
#endif

#include <stdbool.h>

int Platform_InitWindow();

bool Platform_ShouldWindowClose();

void Platform_SwapBuffers();

double Platform_GetElapsedSeconds();

void Platform_DeInit();

void Platform_PollEvents();

#endif
