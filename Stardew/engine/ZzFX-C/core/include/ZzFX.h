/**
    @file Network.h
    @brief a port of https://github.com/KilledByAPixel/ZzFX - far from perfect
    
*/

#ifndef ZZFX_H
#define ZZFX_H

#ifdef __cplusplus
extern "C" {
#endif

/// @brief A struct whos members match the arguments used by ZzFX
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


/// @brief generate a samples for a sound effect into the pre allocated buffer 
/// @param buffer buffer to populate
/// @param bufferSize max size of buffer
/// @param sampleRate sample rate
/// @param inSfx parameters to generate sound effect
/// @return number of samples output
int zzfx_Generate(float* buffer, int bufferSize, float sampleRate, struct ZZFXSound* inSfx);

#ifdef __cplusplus
}
#endif

#endif