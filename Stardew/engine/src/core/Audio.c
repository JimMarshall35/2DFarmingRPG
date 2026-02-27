#include "Audio.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <float.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "Log.h"

#include <stdlib.h>
#include <math.h>
#include "EngineUtils.h"
#include "AssertLib.h"

#include "Thread.h"
#include "ThreadSafeQueue.h"
#include "ObjectPool.h"
#include <cglm/cglm.h>
#include "HandleDefs.h"

#define PI 3.14159265358979323846
#define PI2 (2.0 * PI)

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


static ALuint gSFXSource = -1;
static HSFXBuffer gPlayingBuffersListHead = -1;
static int gPlayingBuffersListLen = 0;
static OBJECT_POOL(struct SFXBuffer) gBuffersPool = NULL;
float* gSfxBuffer = NULL;
size_t gSfxBufferSize = 0;
float gMasterVolume = 0.2;

typedef i32 HWavSFX;

enum StreamedSoundPlayerCommandType
{
    SSPC_Play,
    SSPC_Pause,
    SSPC_Stop,
    SSPC_SkipTrack,
    SSPC_QueueFile,
    SSPC_ClearQueue,
    SSPC_SetVolume,
    SSPC_SetCurrentFileLooping,
    SSPC_SetQueueLooping,
};

struct StreamedSoundPlayerCommand
{
    enum StreamedSoundPlayerCommandType type;
    union
    {
        float fVal;
        char* fileName;
        bool bVal;
    }data;
    
};

enum SoundEffectType
{
    SET_ZzFX,
    SET_PreloadedWav,
};

struct SoundEffect
{
    enum SoundEffectType type;
    vec2 pos;
    union
    {
        struct ZZFXSound zzfx;
        HWavSFX hWav;
    }data;
    
};

enum AudioQueueItemType
{
    AQI_PlaySFX,
    AQI_StreamPlayerCommand,
    AQI_ShutdownAudioThread,
};

struct AudioQueueItem
{
    enum AudioQueueItemType type;

    union 
    {
        struct SoundEffect sfx;
        struct StreamedSoundPlayerCommand streamCmd;
    }data;
};

ALCint gDevRate = 0;
ALuint gBasicSource = -1;
CrossPlatformThread gAudioThread;

struct ThreadSafeQueue gAudioTxQueue;

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
    gSfxBufferSize = sizeof(float) * sfxBufferLenSeconds * gDevRate;
    gSfxBuffer = malloc(gSfxBufferSize); 
    ZeroMemory(gSfxBuffer, gSfxBufferSize);
}

float zzfx_struct(struct ZZFXSound* pSound)
{
    float v = pSound->volume;
    pSound->volume *= gMasterVolume;
    int samples = zzfx_Generate(gSfxBuffer, gSfxBufferSize / sizeof(float), (float)gDevRate, pSound);
    pSound->volume = v;
    HSFXBuffer hBuf = AquireSFXBuffer();
    gBuffersPool[hBuf].timeLeft = (float)samples / (float)gDevRate;
    gBuffersPool[hBuf].hNext = NULL_HANDLE;
    gBuffersPool[hBuf].hPrev = NULL_HANDLE;
    alSourceStop(gBuffersPool[hBuf].hALSource);
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
    return seconds;
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

DECLARE_THREAD_PROC(AudioThread, arg)
{
    const ALCchar *name;
    ALCdevice *device;
    ALCcontext *ctx;
    struct ThreadSafeQueue* pQueue = arg;
    
    device = alcOpenDevice(NULL);
    if(!device)
    {
        Log_Error("Could not open an OpenAL device!");
        return (void*)1;
    }

    alcGetIntegerv(device, ALC_FREQUENCY, 1, &gDevRate);
    Log_Info("audio device sample rate: %i", gDevRate);

    ctx = alcCreateContext(device, NULL);
    if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if(ctx != NULL)
            alcDestroyContext(ctx);
        alcCloseDevice(device);
        Log_Error("Could not set an OpenAL context!");
        return (void*)1;
    }

    name = NULL;
    if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if(!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);
    Log_Info("OpenAL: Opened \"%s\"\n", name);

    gBuffersPool = NEW_OBJECT_POOL(struct SFXBuffer, 64);
    gPlayingBuffersListHead = NULL_HANDLE;
    gPlayingBuffersListLen = 0;
    AllocateWorkingSampleBuffer();
    
    bool bQuit = false;
    while(!bQuit)
    {
        struct AudioQueueItem item;
        while(TSQ_Dequeue(pQueue, &item))
        {
            
            switch(item.type)
            {
            case AQI_ShutdownAudioThread:
                bQuit = true;
                break;
            case AQI_PlaySFX:
                switch(item.data.sfx.type)
                {
                case SET_ZzFX:
                    zzfx_struct(&item.data.sfx.data.zzfx);
                    break;
                case SET_PreloadedWav:
                    break;
                }
                break;
            }
            SleepForMS(5);
            zzfx_Update(5.0f / 1000.0f);
        }
    }

    ctx = alcGetCurrentContext();
    if(ctx == NULL)
        return NULL;

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);

    free(gSfxBuffer);
    FreeObjectPool(gBuffersPool);

    Log_Info("shutting down audio thread");
    return NULL;
}

void OnAudioThreadQueueWrapped(void* pItemToBeLost)
{
    Log_Warning("Network thread Connection event queue wrapped around, packets lost. It must not have been emptied quick enough");
}


int Au_Init(char ***argv, int *argc)
{
    TSQ_Init(&gAudioTxQueue, sizeof(struct AudioQueueItem), 32, &OnAudioThreadQueueWrapped);
    gAudioThread = StartThread(&AudioThread, &gAudioTxQueue);
    return 0;
}

void Au_DeInit()
{
    struct AudioQueueItem aqi = 
    {
        .type = AQI_ShutdownAudioThread
    };
    TSQ_Enqueue(&gAudioTxQueue, &aqi);
    JoinThread(gAudioThread);
}


void Au_PlayZzFX(const struct ZZFXSound* pSound)
{
    struct AudioQueueItem aqi = 
    {
        .type = AQI_PlaySFX,
        .data.sfx.type = SET_ZzFX,
        .data.sfx.data.zzfx = *pSound
    };
    TSQ_Enqueue(&gAudioTxQueue, &aqi);
}
