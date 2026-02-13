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

#define PI 3.14159265358979323846
#define PI2 (2.0 * PI)

ALCint gDevRate = 0;
ALuint gBasicSource = -1;

int Au_Init(char ***argv, int *argc)
{
    const ALCchar *name;
    ALCdevice *device;
    ALCcontext *ctx;
    
    device = alcOpenDevice(NULL);
    if(!device)
    {
        Log_Error("Could not open an OpenAL device!");
        return 1;
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
        return 1;
    }

    name = NULL;
    if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if(!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);
    Log_Info("OpenAL: Opened \"%s\"\n", name);

    return 0;
}

void Au_DeInit()
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
