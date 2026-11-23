/**
    @file openAL.h
    @brief provides an implementation of "zzfx" using openAL
*/

#ifndef ZZFX_OPENAL_H
#define ZZFX_OPENAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct ZZFXSound;

/// @brief play a sound - version with individual params to match the original API
/// @param volume 
/// @param randomness 
/// @param frequency 
/// @param attack 
/// @param sustain 
/// @param release 
/// @param shape 
/// @param shapeCurve 
/// @param slide 
/// @param deltaSlide 
/// @param pitchJump 
/// @param pitchJumpTime 
/// @param repeatTime 
/// @param noise 
/// @param modulation 
/// @param bitCrush 
/// @param delay 
/// @param sustainVolume 
/// @param decay 
/// @param tremolo 
/// @param filter 
/// @return 
float zzfx(
    float volume,
    float randomness,
    float frequency,
    float attack,
    float sustain,
    float release,
    float shape,
    float shapeCurve,
    float slide,
    float deltaSlide,
    float pitchJump,
    float pitchJumpTime,
    float repeatTime,
    float noise,
    float modulation,
    float bitCrush,
    float delay,
    float sustainVolume,
    float decay,
    float tremolo,
    float filter
);

/// @brief play a sound immediately
/// @param pSound the sound struct
/// @return length of sound playing in seconds
float zzfx_struct(struct ZZFXSound* pSound);

/// @brief initialize the backend - call this once before zzfx
int zzfx_InitBackend();

/// @brief destroy the backend when done
void zzfx_ShutdownBackend();

/// @brief master volume applied on top of volume from the zzfx call args
/// @param v 
void zzfx_SetMasterVolume(float v);

/// @brief get the master volue set with zzfx_SetMasterVolume
/// @return 
float zzfx_GetMasterVolume();

/// @brief tell the backend that time has passed, call regularly
/// @param deltaT seconds passed
/// @return 
void zzfx_Update(float deltaT);

/// @brief returns whether sound is playing or not
/// @return 
bool zzfx_IsSoundPlaying();

/// @brief 
/// @param pOutSize 
/// @return 
float* zzfx_GetSfxBuffer(int* pOutSize);

#ifdef __cplusplus
}
#endif

#endif