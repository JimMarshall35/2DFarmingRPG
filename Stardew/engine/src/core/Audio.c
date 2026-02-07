#include "Audio.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef NO_OPENAL
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#endif

#include "Log.h"

int Au_Init(char ***argv, int *argc)
{

#ifndef NO_OPENAL
    const ALCchar *name;
    ALCdevice *device;
    ALCcontext *ctx;

    /* Open and initialize a device */
    // device = NULL;
    // if(argc && argv && *argc > 1 && strcmp((*argv)[0], "-device") == 0)
    // {
    //     device = alcOpenDevice((*argv)[1]);
    //     if(!device)
    //         fprintf(stderr, "Failed to open \"%s\", trying default\n", (*argv)[1]);
    //     (*argv) += 2;
    //     (*argc) -= 2;
    // }
    // if(!device)
    device = alcOpenDevice(NULL);
    if(!device)
    {
        Log_Error("Could not open an OpenAL device!");
        return 1;
    }

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
#endif
    return 0;
}

void Au_DeInit()
{
#ifndef NO_OPENAL
    ALCdevice *device;
    ALCcontext *ctx;

    ctx = alcGetCurrentContext();
    if(ctx == NULL)
        return;

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
#endif
}

