#include "openAL.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "ObjectPool.h"
#include "ZzFX.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"

#ifndef ZeroMemory
#define ZeroMemory(dest, numBytes) memset(dest, 0, numBytes)
#endif


#define NULL_HANDLE -1

typedef int HSFXBuffer;

struct SFXBuffer
{
    /// @brief Length in samples
    float lengthSamples;

    /// @brief OpenAL source handle
    ALuint hALSource;

    /// @brief OpenAL buffer handle
    ALuint hALBuffer;

    /// @brief the time left to play for the sound effect
    float timeLeft;

    HSFXBuffer hThis;

    /// @brief next in the list, when the buffer has finished playing and can be reused
    HSFXBuffer hNext;

    /// @brief prev in the list, when the buffer has finished playing and can be reused
    HSFXBuffer hPrev;

};

static ALCint gDevRate = 0;
static ALuint gSFXSource = -1;
static HSFXBuffer gPlayingBuffersListHead = -1;
static int gPlayingBuffersListLen = 0;
static OBJECT_POOL(struct SFXBuffer) gBuffersPool = NULL;
float* gSfxBuffer = NULL;
size_t gSfxBufferCapacity = 0;
size_t gSfxBufferSize = 0;
float gMasterVolume = 0.2;


static void InsertBufferIntoPlayingList(HSFXBuffer hBuf)
{
    gPlayingBuffersListLen++;
    if(gPlayingBuffersListHead == NULL_HANDLE)
    {
        gPlayingBuffersListHead = hBuf;
        return;
    }

    gBuffersPool[hBuf].hNext = gPlayingBuffersListHead;
    gBuffersPool[gPlayingBuffersListHead].hPrev = hBuf;
    gPlayingBuffersListHead = hBuf;
}

static void RemoveBufferFromPlayingList(HSFXBuffer hBuf)
{
    gPlayingBuffersListLen--;

    HSFXBuffer hNext = gBuffersPool[hBuf].hNext;
    HSFXBuffer hPrev = gBuffersPool[hBuf].hPrev;

    if(hNext != NULL_HANDLE)
    {
        gBuffersPool[hNext].hPrev = hPrev;
    }
    
    if(hPrev != NULL_HANDLE)
    {
        gBuffersPool[hPrev].hNext = hNext;
    }
    
    if(gPlayingBuffersListHead == hBuf)
    {
        gPlayingBuffersListHead = hNext;
    }
}

static HSFXBuffer FindReusableBuffer()
{
    HSFXBuffer hBuf = NULL_HANDLE;

    hBuf = gPlayingBuffersListHead;
    while(hBuf != NULL_HANDLE)
    {
        if(gBuffersPool[hBuf].timeLeft <= 0)
        {
            return hBuf;
        }
        hBuf = gBuffersPool[hBuf].hNext;
    }
    return NULL_HANDLE;
}

static HSFXBuffer AquireSFXBuffer()
{
    HSFXBuffer hR = FindReusableBuffer();
    if(hR != NULL_HANDLE)
    {
        printf("reusing buffer\n");
        RemoveBufferFromPlayingList(hR);
        gBuffersPool[hR].timeLeft = 0;
        gBuffersPool[hR].lengthSamples = 0;
    }
    else
    {
        gBuffersPool = GetObjectPoolIndex(gBuffersPool, &hR);
        gBuffersPool[hR].hALBuffer = NULL_HANDLE;
        gBuffersPool[hR].hALSource = NULL_HANDLE;
        gBuffersPool[hR].hNext = NULL_HANDLE;
        gBuffersPool[hR].hPrev = NULL_HANDLE;
        gBuffersPool[hR].hThis = hR;
        gBuffersPool[hR].lengthSamples = 0;
        alGenSources(1, &gBuffersPool[hR].hALSource);
        
        ALfloat source_x = 0.0f;
        ALfloat source_y = 0.0f;
        ALfloat source_z = 0.0f;
        alSource3f(gBuffersPool[hR].hALSource, AL_POSITION, source_x, source_y, source_z);
    }
    return hR;
}

static void AllocateWorkingSampleBuffer()
{
    const float sfxBufferLenSeconds = 4;
    gSfxBufferCapacity = sizeof(float) * sfxBufferLenSeconds * gDevRate;
    gSfxBuffer = malloc(gSfxBufferCapacity); 
    ZeroMemory(gSfxBuffer, gSfxBufferCapacity);
}

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
)
{
    struct ZZFXSound sfx = {
        volume,
        randomness,
        frequency,
        attack,
        sustain,
        release,
        shape,
        shapeCurve,
        slide,
        deltaSlide,
        pitchJump,
        pitchJumpTime,
        repeatTime,
        noise,
        modulation,
        bitCrush,
        delay,
        sustainVolume,
        decay,
        tremolo,
        filter
    };
    return zzfx_struct(&sfx);
}


float zzfx_struct(struct ZZFXSound* pSound)
{
    float v = pSound->volume;
    pSound->volume *= gMasterVolume;
    int samples = zzfx_Generate(gSfxBuffer, gSfxBufferCapacity / sizeof(float), (float)gDevRate, pSound);
    pSound->volume = v;
    HSFXBuffer hBuf = AquireSFXBuffer();
    printf("Acquired buffer %i\n", hBuf);
    gBuffersPool[hBuf].timeLeft = (float)samples / (float)gDevRate;
    gBuffersPool[hBuf].hNext = NULL_HANDLE;
    gBuffersPool[hBuf].hPrev = NULL_HANDLE;
    alSourceStop(gBuffersPool[hBuf].hALSource);
    printf("Timeleft: %.3f\n", gBuffersPool[hBuf].timeLeft);
    if(gBuffersPool[hBuf].hALBuffer != NULL_HANDLE)
    {
        alDeleteBuffers(1, &gBuffersPool[hBuf].hALBuffer);
    }
    alGenBuffers(1, &gBuffersPool[hBuf].hALBuffer);
    alBufferData(gBuffersPool[hBuf].hALBuffer, AL_FORMAT_MONO_FLOAT32, gSfxBuffer, (ALsizei)samples * sizeof(float), (ALsizei)gDevRate);
    alSourcei(gBuffersPool[hBuf].hALSource, AL_BUFFER, (ALint)gBuffersPool[hBuf].hALBuffer);
    
    //assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");
    alSourcePlay(gBuffersPool[hBuf].hALSource);
    InsertBufferIntoPlayingList(hBuf);
    float seconds = (float)samples / (float)gDevRate;
    gSfxBufferSize = samples;
    return seconds;
}

int zzfx_InitBackend()
{

    const ALCchar *name;
    ALCdevice *device;
    ALCcontext *ctx;
    
    device = alcOpenDevice(NULL);
    if(!device)
    {
        printf("Could not open an OpenAL device!");
        return 1;
    }

    alcGetIntegerv(device, ALC_FREQUENCY, 1, &gDevRate);
    printf("audio device sample rate: %i", gDevRate);

    ctx = alcCreateContext(device, NULL);
    if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if(ctx != NULL)
            alcDestroyContext(ctx);
        alcCloseDevice(device);
        printf("Could not set an OpenAL context!");
        return 1;
    }

    name = NULL;
    if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if(!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);
    printf("OpenAL: Opened \"%s\"\n", name);

    gBuffersPool = NEW_OBJECT_POOL(struct SFXBuffer, 64);
    gPlayingBuffersListHead = NULL_HANDLE;
    gPlayingBuffersListLen = 0;
    AllocateWorkingSampleBuffer();
    return 0;
}

void zzfx_ShutdownBackend()
{
    ALCdevice *device;
    ALCcontext *ctx;

    ctx = alcGetCurrentContext();
    if(ctx == NULL)
        return;

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}

void zzfx_SetMasterVolume(float v)
{
    gMasterVolume = v;
}

float zzfx_GetMasterVolume()
{
    return gMasterVolume;
}

void zzfx_Update(float deltaT)
{
    HSFXBuffer hBuf = NULL_HANDLE;

    hBuf = gPlayingBuffersListHead;
    while(hBuf != NULL_HANDLE)
    {
        if(gBuffersPool[hBuf].timeLeft > 0.0f)
            gBuffersPool[hBuf].timeLeft -= deltaT;
        hBuf = gBuffersPool[hBuf].hNext;
    }
}

bool zzfx_IsSoundPlaying()
{
    HSFXBuffer hBuf = NULL_HANDLE;
    bool bAnyTimeLeft = false;
    if(gPlayingBuffersListLen == 0)
    {
        return false;
    }

    hBuf = gPlayingBuffersListHead;
    while(hBuf != NULL_HANDLE)
    {
        if(gBuffersPool[hBuf].timeLeft > 0.0f)
        {
            bAnyTimeLeft = true;
            break;
        }
            
        hBuf = gBuffersPool[hBuf].hNext;
    }
    return bAnyTimeLeft;
}

float* zzfx_GetSfxBuffer(int* pOutSize)
{   
    *pOutSize = gSfxBufferSize;
    return gSfxBuffer;
}
