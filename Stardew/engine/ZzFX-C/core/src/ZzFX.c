#include "ZzFX.h"

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <immintrin.h>

#include <string.h>

#define __MATH__INTRINSICS__IMPLEMENTATION__
#include "Math_Intrinsics.h"
#include "stdio.h"

#define PI 3.14159265358979323846
#define PI2 (2.0 * PI)

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

__m256 mm256_tan_ps(__m256 val)
{
    __m256 cos, sin;
    mm256_sincos_ps(val, &sin, &cos);
    return _mm256_div_ps(sin, cos);
}

__m256 mm256_fmodf_ps(__m256 _x, __m256 _y)
{
    //x - trunc(x / y) * y;
    __m256 d = _mm256_div_ps(_x, _y);
    __m256 trunc = _mm256_floor_ps(d);
    __m256 c = _mm256_mul_ps(trunc, _y);
    return _mm256_sub_ps(_x, c);
}

// https://github.com/MonoS/RGVS/blob/master/Repair.cpp
__m256 mm256_abs_ps(__m256 x)
{
    __m256 Mask = _mm256_castsi256_ps(_mm256_set1_epi32(~0x80000000));

    __m256 abs = _mm256_and_ps(Mask, x);

    return abs;
}

__m256 mm256_sign_ps(__m256 x)
{
    __m256 zero = _mm256_setzero_ps();

    __m256 positive = _mm256_and_ps(_mm256_cmp_ps(x, zero, _CMP_GT_OS), _mm256_set1_ps(1.0f));
    __m256 negative = _mm256_and_ps(_mm256_cmp_ps(x, zero, _CMP_LT_OS), _mm256_set1_ps(-1.0f));

    return _mm256_or_ps(positive, negative);
}

// __m256 mm256_abs_ps(__m256 _x)
// {
//     __m256 _zero = _mm256_setzero_ps();
//     __m256 _minusOne = _mm256_set1_ps(-1.0f);
//     __m256 _ltMask = _mm256_cmp_ps(_x, _zero, _CMP_LT_OS);
//     __m256 _gtMask = _mm256_cmp_ps(_x, _zero, _CMP_GT_OS);
//     __m256 _flipped = _mm256_mul_ps(_minusOne, _x);
//     __m256 _lt = _mm256_and_ps(_ltMask, _flipped); 
//     __m256 _gt = _mm256_and_ps(_gtMask, _x);
//     //__m256 negative = _mm256_and_ps(_mm256_cmp_ps(x, zero, _CMP_GT_OS), _mm256_set1_ps(-1.0f));

//     return _mm256_or_ps(_lt, _gt);
// }



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

    __m256 _pi2 = _mm256_set1_ps(PI2);

    __m256 _samples = _mm256_set1_ps(0.0f);
    __m256 _repeatTime = _mm256_set1_ps((float)repeatTime);

    __m256 _tremelo = _mm256_set1_ps(sfx->tremolo);
    __m256 _length = _mm256_set1_ps(length);

    for (int i = 0; i < length; i += sizeof(__m256) / sizeof(float))
    {
        int crushVals[sizeof(__m256) / sizeof(float)];
        int modVals[sizeof(__m256) / sizeof(float)];
        float tVals[sizeof(__m256) / sizeof(float)];
        float delayVals[sizeof(__m256) / sizeof(float)];
        for(int j=0; j<sizeof(__m256) / sizeof(float); j++)
        {
            crushVals[j] = ++crush;
            modVals[j] = SafeMod(crushVals[j], bitcrush100);

            // frequency + modulation + noise
            f = (frequency += slide += deltaSlide) * cosf(modulation * modOffset++);
            t += f + f * sfx->noise * sinf(powf((float)i + j, 5.0f));

            // pitch jump
            if (jump && (++jump > pitchJumpTime)) 
            {
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
        
        
        __m256i _lengthMask = _mm256_set_epi32(
            (i     < length ? 0xffffffff : 0),
            (i + 1 < length ? 0xffffffff : 0),
            (i + 2 < length ? 0xffffffff : 0),
            (i + 3 < length ? 0xffffffff : 0),
            (i + 4 < length ? 0xffffffff : 0),
            (i + 5 < length ? 0xffffffff : 0),
            (i + 6 < length ? 0xffffffff : 0),
            (i + 7 < length ? 0xffffffff : 0));


        __m256 _i = _mm256_set_ps(
            ((float)i + 0),
            ((float)i + 1),
            ((float)i + 2),
            ((float)i + 3),
            ((float)i + 4),
            ((float)i + 5),
            ((float)i + 6),
            ((float)i + 7)
        );
        
        //__m256 _lengthMask = _mm256_cmp_ps(_i, _length, _CMP_LT_OS);

        __m256 _modMask = _mm256_set_ps(
            (!modVals[0] ? 1.0f : 0),
            (!modVals[1] ? 1.0f : 0),
            (!modVals[2] ? 1.0f : 0),
            (!modVals[3] ? 1.0f : 0),
            (!modVals[4] ? 1.0f : 0),
            (!modVals[5] ? 1.0f : 0),
            (!modVals[6] ? 1.0f : 0),
            (!modVals[7] ? 1.0f : 0)); 
            
        __m256 _t = _mm256_set_ps(
            tVals[0],
            tVals[1],
            tVals[2],
            tVals[3],
            tVals[4],
            tVals[5],
            tVals[6],
            tVals[7]
        );
        
        

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
                            __m256 _one = _mm256_set1_ps(1.0f);
                            __m256 _minusOne = _mm256_set1_ps(-1.0f);
                            __m256 _two = _mm256_set1_ps(2.0f);
                            __m256 _tOverPI2 = _mm256_div_ps(_t, _pi2);
                            __m256 _fmod = mm256_fmodf_ps(_tOverPI2, _one);
                            __m256 _shapeCurveOver2 = _mm256_set1_ps(sfx->shapeCurve/2.0f);
                            __m256 _ternaryMask = _mm256_cmp_ps(_fmod, _shapeCurveOver2, _CMP_LT_OS);
                            __m256 _toAdd = _mm256_and_ps(_two, _ternaryMask); /* add two to the values where the fmod result was < shapeCurve / 2.0f to give values of either 1 or -1 */
                            _samples = _mm256_add_ps(_minusOne, _toAdd);
                            //s = fmodf(t/PI2,1.0f) < sfx->shapeCurve/2.0f ? 1.0f : -1.0f;
                        } 
                        else 
                        {
                            // 4: noise
                            __m256 _t2 = _mm256_mul_ps(_t, _t);
                            __m256 _t3 = _mm256_mul_ps(_t, _t);
                            _samples = mm256_sin_ps(_t3);
                            //s = sinf(t*t*t); // noise-like
                        }
                    } 
                    else 
                    {
                        // 3: tan
                        __m256 _one = _mm256_set1_ps(1.0f);
                        __m256 _minusOne = _mm256_set1_ps(1.0f);
                        __m256 _tan = mm256_tan_ps(_t);
                        __m256 _min = _mm256_min_ps(_tan, _one);
                        _samples = _mm256_max_ps(_min, _minusOne);
                        //s = fmaxf(fminf(tanf(t),1.0f),-1.0f);
                    }
                } 
                else 
                {
                    // 2: saw
                    //s = 1 - (2 * t / PI2 % 2 + 2) % 2;
                    __m256 _1 = _mm256_set1_ps(1.0f);
                    __m256 _2 = _mm256_set1_ps(2.0f);
                    __m256 _2t = _mm256_mul_ps(_2, _t);
                    __m256 _2toverpi2 = _mm256_div_ps(_2t, _pi2);
                    __m256 _fmod1 = mm256_fmodf_ps(_2toverpi2, _2);
                    _fmod1 = _mm256_add_ps(_fmod1, _2);
                    __m256 _fmod2 = mm256_fmodf_ps(_fmod1, _2);
                    _samples = _mm256_sub_ps(_fmod2, _1);
                    //s = 1.0f - fmodf(fmodf(2.0f * t / PI2, 2.0f) + 2.0f, 2.0); // saw
                }
            } 
            else 
            {
                // 1: triangle
                //s = 1.0f - 4.0f * fabsf(roundf(t/PI2) - t/PI2); // triangle
                __m256 _1 = _mm256_set1_ps(1.0f);
                __m256 _4 = _mm256_set1_ps(4.0f);
                __m256 _tOverPI2 = _mm256_div_ps(_t, _pi2);
                __m256 _r = _mm256_round_ps(_tOverPI2, _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC);
                _r = _mm256_sub_ps(_r, _tOverPI2);
                __m256 _abs = mm256_abs_ps(_r);
                _abs = _mm256_mul_ps(_abs, _4);
                _samples = _mm256_sub_ps(_1, _abs); 
            }
        } 
        else 
        {
            // 0: sine
            //s = sinf(t);
            _samples = mm256_sin_ps(_t);
        }
        //printf("generated wave\n");
        
        // tremolo
        // if (repeatTime)
        //     s *= 1.0f - sfx->tremolo + sfx->tremolo * sinf(PI2 * i / repeatTime);
        if(repeatTime)
        {
            __m256 _1 = _mm256_set1_ps(1.0f);
            __m256 _piTimesI = _mm256_mul_ps(_pi2, _i);
            __m256 _sinArg = _mm256_div_ps(_piTimesI, _repeatTime);
            __m256 _sinRes = mm256_sin_ps(_sinArg);
            __m256 _sinTremelo = _mm256_mul_ps(_tremelo, _sinRes);
            __m256 _oneMinusTremelo = _mm256_sub_ps(_1, _tremelo);
            __m256 _multiplier = _mm256_add_ps(_oneMinusTremelo, _sinTremelo);
            _samples = _mm256_mul_ps(_samples, _multiplier);
        }
        
        // if (sfx->shape <= 4) s = signf(s) * powf(fabsf(s), sfx->shapeCurve);
        if(sfx->shape <= 4)
        {
            __m256 _sign = mm256_sign_ps(_samples);
            __m256 _abs = mm256_abs_ps(_samples);
            __m256 _shapeCurve = _mm256_set1_ps(sfx->shapeCurve);
            __m256 _powf = mm256_pow_ps(_abs, _shapeCurve);
            _samples = _mm256_mul_ps(_sign, _powf);
        }
        /*
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
        */
        {
            __m256 _1 = _mm256_set1_ps(1.0f);
            __m256 lengthMinusDelay = _mm256_set1_ps(length - delay);
            __m256 _adsr = _mm256_set1_ps(length - delay);
            __m256 _ads = _mm256_set1_ps(attack + decay + sustain);
            __m256 _ad = _mm256_set1_ps(attack + decay);
            __m256 _a = _mm256_set1_ps(attack);
            __m256 _d = _mm256_set1_ps(decay);
            __m256 _mask_a = _mm256_cmp_ps(_i, _a, _CMP_LT_OS);
            __m256 _mask_d = _mm256_cmp_ps(_i, _ad, _CMP_LT_OS);
            __m256 _mask_s = _mm256_cmp_ps(_i, _ads, _CMP_LT_OS);
            __m256 _mask_r = _mm256_cmp_ps(_i, _adsr, _CMP_LT_OS);

            _mask_r = _mm256_andnot_ps(_mask_s, _mask_r);
            _mask_s = _mm256_andnot_ps(_mask_d, _mask_s);
            _mask_d = _mm256_andnot_ps(_mask_a, _mask_s);

            /* attack value */
            __m256 _valAttack = _mm256_div_ps(_i, _a);

            /* decay value */
            // 1.0f - (((float)i - attack) / decay) * (1.0f - sfx->sustainVolume);
            __m256 _iminusAttack = _mm256_sub_ps(_i, _a);
            _iminusAttack = _mm256_div_ps(_iminusAttack, _d);
            __m256 _oneMinusSus = _mm256_set1_ps(1.0f - sfx->sustainVolume);
            _iminusAttack = _mm256_mul_ps(_iminusAttack, _oneMinusSus);
            __m256 _valDecay = _mm256_sub_ps(_1, _iminusAttack);

            /* sustain value */
            __m256 _valSustain = _mm256_set1_ps(sfx->sustainVolume);

            /* release value */
            // (length - i - delay) / release * sfx->sustainVolume;
            __m256 _len = _mm256_set1_ps(length);
            __m256 _del = _mm256_set1_ps(delay);
            __m256 _relTimesSusVol = _mm256_set1_ps(release * sfx->sustainVolume);
            _len = _mm256_sub_ps(_len, _i);
            _len = _mm256_sub_ps(_len, _del);
            __m256 _valRelease = _mm256_div_ps(_len, _relTimesSusVol);

            __m256 _multiplierA = _mm256_and_ps(_valAttack, _mask_a);
            __m256 _multiplierD = _mm256_and_ps(_valDecay, _mask_d);
            __m256 _multiplierS = _mm256_and_ps(_valSustain, _mask_s);
            __m256 _multiplierR = _mm256_and_ps(_valRelease, _mask_r);
            __m256 _envelope = _mm256_setzero_ps();
            _envelope = _mm256_or_ps(_envelope, _multiplierA);
            _envelope = _mm256_or_ps(_envelope, _multiplierD);
            _envelope = _mm256_or_ps(_envelope, _multiplierS);
            _envelope = _mm256_or_ps(_envelope, _multiplierR);

            _samples = _mm256_mul_ps(_samples, _envelope);
        }
        
        {
            /*
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
            */
           if(delay > 0)
           {
                /* TEMPORARILY COMMENTED OUT: _mm256_mask_i32gather_ps throws error on game CI, param 3 should be __m256i  */
                // __m256 _delay = _mm256_set1_ps(delay);
                // __m256i _delayInt = _mm256_cvtps_epi32(_delay);
                // __m256 _dIndex = _mm256_sub_ps(_i, _delay);
                // __m256 _0 = _mm256_setzero_ps();
                // __m256 _2 = _mm256_set1_ps(2.0f);
                // __m256 _ltZeroMask = _mm256_cmp_ps(_dIndex, _0, _CMP_LT_OS);
                // __m256 _geZeroMask = _mm256_cmp_ps(_dIndex, _0, _CMP_GE_OS);                          /* mask of all the packed singles >= 0 */
                // __m256 _delaySamples = _mm256_mask_i32gather_ps(_0, buffer, _dIndex, _geZeroMask, 4); /* load from the buffer packed singles at indexes from _dIndex */
                
                // __m256 _sOver2 = _mm256_div_ps(_samples, _2);
                // __m256 _delaySampleOver2 = _mm256_div_ps(_delaySamples, _2);

                // __m256 _validIndexVal = _mm256_add_ps(_sOver2, _delaySampleOver2);
                // __m256 _validPortion = _mm256_and_ps(_geZeroMask, _validIndexVal);
                // __m256 _invalidPortion = _mm256_and_ps(_ltZeroMask, _sOver2);
                // _samples = _mm256_or_ps(_validPortion, _invalidPortion);
            }
        }

        __m256 _vol = _mm256_set1_ps(volume);
        _samples = _mm256_mul_ps(_samples, _vol);

        _mm256_maskstore_ps(buffer + i, _lengthMask, _samples);
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
    //return zzfx_Generate_avx(buffer, bufferSize, sampleRate, inSfx);
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
                        s = 1.0f - fmodf(fmodf(2.0f * t / PI2, 2.0f) + 2.0f, 2.0); // saw
                        

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
            // s = delay ? s/2.0f + (delay > i ? 0 : 
            //         (i < length - delay ? 1 : (length-1) / delay) * 
            //         (buffer[i-(int)delay] / 2.0) / sfx->volume) : s;

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
        if (jump && (++jump > pitchJumpTime))
        {
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
    //Log_Info("min: %.2f max: %.2f", minS, maxS);
    return length;
}