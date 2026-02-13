#ifndef STARDEW_AUDIO_H
#define STARDEW_AUDIO_H

struct ZZFXSound
{
    float volume;
    float randomness;
    float frequency;
    float attack;
    float sustain;
    float release;
    float shape;
    float shapeCurve;
    float slide;
    float deltaSlide;
    float pitchJump;
    float pitchJumpTime;
    float repeatTime;
    float noise;
    float modulation;
    float bitCrush;
    float delay;
    float sustainVolume;
    float decay;
    float tremolo;
    float filter;
};

int Au_Init();

void Au_DeInit();

#endif