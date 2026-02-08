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

int zzfx_Generate(float* buffer, int bufferSize, float sampleRate, struct ZZFXSound* sfx) ;

float Au_PlaySoundTest(struct ZZFXSound* sound, float masterVol)
{
    const float sfxBufferLenSeconds = 4;
    size_t bufferSize = sizeof(float) * sfxBufferLenSeconds * gDevRate;
    float* sfxBuffer = malloc(bufferSize); 
    ZeroMemory(sfxBuffer, bufferSize);
    float v = sound->volume;
    sound->volume *= masterVol;
    int samples = zzfx_Generate(sfxBuffer, bufferSize / sizeof(float), (float)gDevRate, sound);
    sound->volume = v;
    Log_Info("num samples %i", samples);
    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO_FLOAT32, sfxBuffer, (ALsizei)samples * sizeof(float), (ALsizei)gDevRate);
    free(sfxBuffer);

    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        Log_Error("OpenAL Error: %s\n", alGetString(err));
        if(alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return;
    }

    /* Create the source to play the sound with. */
    ALuint source = 0;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, (ALint)buffer);
    ALfloat source_x = 0.0f;
    ALfloat source_y = 0.0f;
    ALfloat source_z = 0.0f;
    alSource3f(source, AL_POSITION, source_x, source_y, source_z);
    EASSERT(alGetError()==AL_NO_ERROR && "Failed to setup sound source");
    alSourcePlay(source);

    alDeleteBuffers(1, &buffer);
    return (float)samples / (float)gDevRate;
}

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


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


float signf(float v) { return v < 0 ? -1.0f : 1.0f; }

float RandFloat01()
{
    double r = rand() / (RAND_MAX + 1.0);
    return r;
}

/*

*/
int SafeMod(int a, int b)
{
    if(b == 0)
    {
        return 0;
    }
    return a % b;
}

/// @brief a port of https://github.com/KilledByAPixel/ZzFX - far from perfect
/// @param buffer buffer to populate
/// @param bufferSize max size of buffer
/// @param sampleRate sample rate
/// @param inSfx parameters to generate sound effect
/// @return number of samples output
int zzfx_Generate(float* buffer, int bufferSize, float sampleRate, struct ZZFXSound* inSfx) 
{
    struct ZZFXSound cpy;
    memcpy(&cpy, inSfx, sizeof(struct ZZFXSound));
    struct ZZFXSound* sfx = &cpy;
    float startSlide = sfx->slide * 500.0f * PI2 / (sampleRate * sampleRate);
    float startFrequency = sfx->frequency * 
        (1.0f + sfx->randomness * 2.0f * (RandFloat01() - sfx->randomness))
        * PI2 / sampleRate;
    
    float slide = startSlide;
    float frequency = startFrequency;
    float modOffset = 0.0f;
    int repeat = 0;
    int crush = 0;
    int jump = 1;
    
    float t = 0.0f;
    float s = 0.0f;
    float f;

    // biquad filter coefficients
    float quality = 2.0f;
    float w = PI2 * fabsf(sfx->filter) * 2.0f / sampleRate;
    float cos_w = cosf(w);
    float alpha = sinf(w) / 2.0f / quality;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos_w / a0;
    float a2 = (1.0f - alpha) / a0;
    float b0 = (1.0f + signf(sfx->filter) * cos_w) / 2.0f / a0;
    float b1 = -(signf(sfx->filter) + cos_w) / a0;
    float b2 = b0;
    float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

    // envelope scaling
    float minAttack = 9.0f;
    float attack = sfx->attack * sampleRate;
    if (attack <= 0)
    {
        attack = minAttack;
    }
    float decay = sfx->decay * sampleRate;
    float sustain = sfx->sustain * sampleRate;
    float release = sfx->release * sampleRate;
    float delay = sfx->delay * sampleRate;
    float deltaSlide = sfx->deltaSlide * 500.0f * PI2 / (sampleRate * sampleRate * sampleRate);
    float modulation = sfx->modulation * PI2 / sampleRate;
    float pitchJump = sfx->pitchJump * PI2 / sampleRate;
    int pitchJumpTime = (int)(sfx->pitchJumpTime * sampleRate);
    int repeatTime = (int)(sfx->repeatTime * sampleRate);
    float volume = sfx->volume;

    int length = (int)(attack + decay + sustain + release + delay);
    if (length > bufferSize) length = bufferSize;

    float minS = FLT_MAX;
    float maxS = FLT_MIN;

    for (int i = 0; i < length; ++i) 
    {
        // Bit crush
        if (!(SafeMod(++crush , (int)(sfx->bitCrush * 100)))) 
        {
            // waveform generation
            if ((int)sfx->shape == 0)
            {
                s = sinf(t);
            } 
            else if ((int)sfx->shape == 1) 
            {
                s = 1.0f - 4.0f * fabsf(roundf(t/PI2) - t/PI2); // triangle
            } 
            else if ((int)sfx->shape == 2) 
            {
                s = 1.0f - fmodf(2.0f * t / PI2 + 2.0f, 2.0f); // saw
            } 
            else if ((int)sfx->shape == 3) 
            {
                s = fmaxf(fminf(tanf(t),1.0f),-1.0f); // tan
            }
            else if ((int)sfx->shape == 4)
            {
                s = sinf(t*t*t); // noise-like
            }
            else if ((int)sfx->shape == 5)
            {
                s = fmodf(t/PI2,1.0f) < sfx->shapeCurve/2.0f ? 1.0f : -1.0f; // square duty
            }

            // tremolo
            if (repeatTime)
                s *= 1.0f - sfx->tremolo + sfx->tremolo * sinf(PI2 * i / repeatTime);

            // shape curve
            if (sfx->shape <= 4) s = signf(s) * powf(fabsf(s), sfx->shapeCurve);

            // envelope
            if (i < attack) 
            {
                s *= i / attack;
            }
            else if (i < attack + decay)
            {
                s *= 1.0f - ((i - attack) / decay) * (1.0f - sfx->sustainVolume);
            }
            else if (i < attack + decay + sustain)
            {
                s *= sfx->sustainVolume;
            }
            else if (i < length - delay)
            {
                s *= (length - i - delay) / release * sfx->sustainVolume;
            }
            else
            {
                s = 0.0f;
            }

            // delay
            if (delay > 0)
            {
                int dIndex = i - (int)delay;
                if (dIndex >= 0)
                {
                    s = s / 2.0f + buffer[dIndex] / 2.0f;
                }
                else
                {
                    s = s / 2.0f;
                }
            }

            // filter
            if (sfx->filter != 0)
            {
                y1 = s;
                s = b2*x2 + b1*x1 + b0*s - a2*y2 - a1*y1;
                x2 = x1; x1 = y1;
                y2 = y1;
            }
        }

        // frequency + modulation + noise
        f = (frequency += slide += deltaSlide) * cosf(modulation * modOffset++);
        t += f + f * sfx->noise * sinf(powf((float)i, 5.0f));

        // pitch jump
        if (jump && (++jump > pitchJumpTime)) {
            frequency += pitchJump;
            startFrequency += pitchJump;
            jump = 0;
        }

        // repeat
        if (repeatTime && !(++repeat % repeatTime))
        {
            frequency = startFrequency;
            slide = startSlide;
            if (!jump) 
                jump = 1;
        }

        if(s * volume > maxS)
        {
            maxS = s * volume;
        }
        if(s * volume < minS)
        {
            minS = s * volume;
        }

        // write sample
        buffer[i] = s * volume;
    }
    Log_Info("min: %.2f max: %.2f", minS, maxS);
    return length;
}