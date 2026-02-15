#ifndef STARDEW_AUDIO_H
#define STARDEW_AUDIO_H
#include "ZzFX.h"

void Au_PlayZzFX(const struct ZZFXSound* pSound);

int Au_Init();

void Au_DeInit();

#endif