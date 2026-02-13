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

#include <x86intrin.h>

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

__m256 fmodf256(__m256 _x, __m256 _y)
{
    //x - trunc(x / y) * y;
    __m256 d = _mm256_div_ps(_x, _y);
    __m256 trunc = _mm256_floor_ps(d);
    __m256 c = _mm256_mul_ps(trunc, _y);
    return _mm256_sub_ps(_x, c);
}

int zzfx_Generate_avx(float* buffer, int bufferSize, float sampleRate, struct ZZFXSound* inSfx) 
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
    float bitcrush100 = (int)(sfx->bitCrush * 100);
    for (int i = 0; i < length; i += sizeof(__m256) / sizeof(float))
    {
        int crushVals[sizeof(__m256) / sizeof(float)];
        int modVals[sizeof(__m256) / sizeof(float)];
        float tVals[sizeof(__m256) / sizeof(float)];
        for(int j=0; j<sizeof(__m256) / sizeof(float); j++)
        {
            crushVals[j] = ++crush;
            modVals[j] = SafeMod(crushVals[j], bitcrush100);

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
            tVals[j] = t;
        }
        
        
        __m256i lengthMask = _mm256_set_epi32(
            (i     < length ? 0xffffffff : 0),
            (i + 1 < length ? 0xffffffff : 0),
            (i + 2 < length ? 0xffffffff : 0),
            (i + 3 < length ? 0xffffffff : 0),
            (i + 4 < length ? 0xffffffff : 0),
            (i + 5 < length ? 0xffffffff : 0),
            (i + 6 < length ? 0xffffffff : 0),
            (i + 7 < length ? 0xffffffff : 0));

        __m256 modMask = _mm256_set_ps(
            (!modVals[0] ? 1.0f : 0),
            (!modVals[1] ? 1.0f : 0),
            (!modVals[2] ? 1.0f : 0),
            (!modVals[3] ? 1.0f : 0),
            (!modVals[4] ? 1.0f : 0),
            (!modVals[5] ? 1.0f : 0),
            (!modVals[6] ? 1.0f : 0),
            (!modVals[7] ? 1.0f : 0)); 
            
        
        __m256 samples;

        switch((int)sfx->shape)
        {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        }
        
        _mm256_maskstore_ps(buffer + i, lengthMask, samples);
    } 
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
            if (sfx->shape) 
            {
                if (sfx->shape > 1) 
                {
                    if (sfx->shape > 2) 
                    {
                        if (sfx->shape > 3) 
                        {
                            if (sfx->shape > 4) 
                            {
                                // 5: square (duty)
                                s = fmodf(t/PI2,1.0f) < sfx->shapeCurve/2.0f ? 1.0f : -1.0f;
                            } 
                            else 
                            {
                                // 4: noise
                                s = sinf(t*t*t); // noise-like
                            }
                        } 
                        else 
                        {
                            // 3: tan
                            s = fmaxf(fminf(tanf(t),1.0f),-1.0f);
                        }
                    } 
                    else 
                    {
                        // 2: saw
                        //s = 1 - (2 * t / PI2 % 2 + 2) % 2;
                        s = 1.0f - fmodf(2.0f * t / PI2 + 2.0f, 2.0f); // saw
                        float phase = t / PI2;
                        float doubled = 2 * phase;
                        float wrapped = fmodf(doubled, 2.0f);
                        float shifted = wrapped + 2;
                        float normalized = fmodf(shifted, 2);

                        s = 1 - normalized;

                    }
                } 
                else 
                {
                    // 1: triangle
                    s = 1.0f - 4.0f * fabsf(roundf(t/PI2) - t/PI2); // triangle
                }
            } 
            else 
            {
                // 0: sine
                s = sinf(t);
            }


            // tremolo
            if (repeatTime)
                s *= 1.0f - sfx->tremolo + sfx->tremolo * sinf(PI2 * i / repeatTime);

            // shape curve
            if (sfx->shape <= 4) s = signf(s) * powf(fabsf(s), sfx->shapeCurve);

            // envelope
            if (i < attack) 
            {
                s *= (float)i / attack;
            }
            else if (i < attack + decay)
            {
                s *= 1.0f - (((float)i - attack) / decay) * (1.0f - sfx->sustainVolume);
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
            /*
            

                s = delay ? s/2 + (delay > i ? 0 :           // delay
                    (i<length-delay? 1 : (length-i)/delay) * // release delay 
                    b[i-delay|0]/2/volume) : s;              // sample delay
            */
            if (delay > 0)
            {
                
                // int dIndex = i - (int)delay;
                // if (dIndex >= 0)
                // {
                //     s = s / 2.0f + buffer[dIndex] / 2.0f;
                // }
                // else
                // {
                //     s = s / 2.0f;
                // }
            }
            s = delay ? s/2.0f + (delay > i ? 0 : 
                    (i < length - delay ? 1 : (length-1) / delay) * 
                    (buffer[i-(int)delay] / 2.0) / sfx->volume) : s;

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