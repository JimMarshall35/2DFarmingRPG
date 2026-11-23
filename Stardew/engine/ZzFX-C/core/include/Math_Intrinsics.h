/*

MIT License

Copyright (c) 2025 Geolm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*/


#ifndef MATH__INTRINSICS__H
#define MATH__INTRINSICS__H

/*

    NEON/AVX trascendental math functions

    Documentation can be found https://github.com/Geolm/math_intrinsics/

*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__ARM_NEON) && defined(__ARM_NEON__)
#include <arm_neon.h>

    // max error : 5.960464478e-08
    float32x4_t vcosq_f32(float32x4_t a);

    // max error : 5.960464478e-08
    float32x4_t vsinq_f32(float32x4_t a);

    // max error : 5.960464478e-08
    void vsincosq_f32(float32x4_t a, float32x4_t *s, float32x4_t *c);

    // max error : 2.384185791e-07
    float32x4_t vacosq_f32(float32x4_t a);

    // max error : 1.192092896e-07
    float32x4_t vasinq_f32(float32x4_t a);

    // max error : 6.699562073e-05
    float32x4_t vatanq_f32(float32x4_t a);

    // max error : 2.384185791e-07
    float32x4_t vatan2q_f32(float32x4_t x, float32x4_t y);

    // max error : 4.768371582e-07
    float32x4_t vlogq_f32(float32x4_t a);

    // max error : 2.349663504e-07
    float32x4_t vlog2q_f32(float32x4_t x);

    // max error : 1.108270880e-07
    float32x4_t vexpq_f32(float32x4_t a);

    // max error : 1.042427087e-07
    float32x4_t vexp2q_f32(float32x4_t a);

    // max error : 4.768371582e-07
    float32x4_t vcbrtq_f32(float32x4_t a);

    // max error : 9.768706377e-07
    float32x4_t vpowq_f32(float32x4_t x, float32x4_t y);

    #define __MATH__INTRINSICS__NEON__

#else
#include <immintrin.h>

    // max error : 5.960464478e-08
    __m256 mm256_cos_ps(__m256 a);

    // max error : 5.960464478e-08
    __m256 mm256_sin_ps(__m256 a);

    // max error : 5.960464478e-08
    void mm256_sincos_ps(__m256 a, __m256 *s, __m256 *c);

    // max error : 2.384185791e-07
    __m256 mm256_acos_ps(__m256 a);

    // max error : 1.192092896e-07
    __m256 mm256_asin_ps(__m256 a);

    // max error : 6.699562073e-05
    __m256 mm256_atan_ps(__m256 a);

    // max error : 2.384185791e-07
    __m256 mm256_atan2_ps(__m256 x, __m256 y);

    // max error : 4.768371582e-07
    __m256 mm256_log_ps(__m256 a);

    // max error : 2.349663504e-07
    __m256 mm256_log2_ps(__m256 x);

    // max error : 1.108270880e-07
    __m256 mm256_exp_ps(__m256 a);

    // max error : 1.042427087e-07
    __m256 mm256_exp2_ps(__m256 x);

    // max error : 4.768371582e-07
    __m256 mm256_cbrt_ps(__m256 a);

    // max error : 9.768706377e-07
    __m256 mm256_pow_ps(__m256 x, __m256 y);

    #define __MATH__INTRINSICS__AVX__

#endif

#ifdef __cplusplus
}
#endif

#endif


#ifdef __MATH__INTRINSICS__IMPLEMENTATION__

#define SIMD_MATH_TAU (6.28318530f)
#define SIMD_MATH_PI  (3.14159265f)
#define SIMD_MATH_PI2 (1.57079632f)
#define SIMD_MATH_PI4 (0.78539816f)

#if defined(__ARM_NEON) && defined(__ARM_NEON__)
    typedef float32x4_t simd_vector;

    static inline simd_vector simd_add(simd_vector a, simd_vector b) {return vaddq_f32(a, b);}
    static inline simd_vector simd_sub(simd_vector a, simd_vector b) {return vsubq_f32(a, b);}
    static inline simd_vector simd_mul(simd_vector a, simd_vector b) {return vmulq_f32(a, b);}
    static inline simd_vector simd_div(simd_vector a, simd_vector b) {return vdivq_f32(a, b);}
    static inline simd_vector simd_abs(simd_vector a) {return vabsq_f32(a);}
    static inline simd_vector simd_fmad(simd_vector a, simd_vector b, simd_vector c) {return vfmaq_f32(c, a, b);}
    static inline simd_vector simd_or(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));}
    static inline simd_vector simd_xor(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));}
    static inline simd_vector simd_and(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));}
    static inline simd_vector simd_andnot(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));}
    static inline simd_vector simd_min(simd_vector a, simd_vector b) {return vminq_f32(a, b);}
    static inline simd_vector simd_max(simd_vector a, simd_vector b) {return vmaxq_f32(a, b);}
    static inline simd_vector simd_cmp_gt(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vcgtq_f32(a, b));}
    static inline simd_vector simd_cmp_ge(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vcgeq_f32(a, b));}
    static inline simd_vector simd_cmp_lt(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vcltq_f32(a, b));}
    static inline simd_vector simd_cmp_le(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vcleq_f32(a, b));}
    static inline simd_vector simd_cmp_eq(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vceqq_f32(a, b));}
    static inline simd_vector simd_cmp_neq(simd_vector a, simd_vector b) {return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(a, b)));}
    static inline simd_vector simd_isnan(simd_vector a) {return simd_cmp_neq(a, a);}
    static inline simd_vector simd_select(simd_vector a, simd_vector b, simd_vector mask) {return vbslq_f32(vreinterpretq_u32_f32(mask), b, a);}
    static inline simd_vector simd_splat(float value) {return vdupq_n_f32(value);}
    static inline simd_vector simd_splat_zero(void) {return vdupq_n_f32(0);}
    static inline simd_vector simd_splat_positive_infinity(void) {return vreinterpretq_u32_f32(vdupq_n_u32(0x7f800000));}
    static inline simd_vector simd_splat_negative_infinity(void) {return vreinterpretq_u32_f32(vdupq_n_u32(0xff800000));}
    static inline simd_vector simd_sign_mask(void) {return vreinterpretq_u32_f32(vdupq_n_u32(0x80000000));}
    static inline simd_vector simd_inv_sign_mask(void) {return vreinterpretq_u32_f32(vdupq_n_u32(~0x80000000));}
    static inline simd_vector simd_abs_mask(void) {return vreinterpretq_u32_f32(vdupq_n_u32(0x7FFFFFFF));}
    static inline simd_vector simd_min_normalized(void) {return vreinterpretq_u32_f32(vdupq_n_u32(0x00800000));} // the smallest non denormalized float number
    static inline simd_vector simd_inv_mant_mask(void){return vreinterpretq_u32_f32(vdupq_n_u32(~0x7f800000));}
    static inline simd_vector simd_floor(simd_vector a) {return vrndmq_f32(a);}
    static inline simd_vector simd_round(simd_vector a) {return vrndnq_f32(a);}
    static inline simd_vector simd_neg(simd_vector a) {return vnegq_f32(a);}
    static inline simd_vector simd_sqrt(simd_vector a) {return vsqrtq_f32(a);}

    typedef int32x4_t simd_vectori;
    static inline simd_vectori simd_convert_from_float(simd_vector a) {return vcvtq_s32_f32(a);}
    static inline simd_vectori simd_cast_from_float(simd_vector a) {return vreinterpretq_s32_f32(a);}
    static inline simd_vector simd_convert_from_int(simd_vectori a) {return vcvtq_f32_s32(a);}
    static inline simd_vector simd_cast_from_int(simd_vectori a) {return vreinterpretq_f32_s32(a);}
    static inline simd_vectori simd_add_i(simd_vectori a, simd_vectori b) {return vaddq_s32(a, b);}
    static inline simd_vectori simd_sub_i(simd_vectori a, simd_vectori b) {return vsubq_s32(a, b);}
    static inline simd_vector simd_mul_i(simd_vector a, simd_vector b) {return vmulq_s32(a, b);}
    static inline simd_vectori simd_splat_i(int i) {return vdupq_n_s32(i);}
    static inline simd_vectori simd_splat_zero_i(void) {return vdupq_n_s32(0);}
    static inline simd_vectori simd_shift_left_i(simd_vectori a, int i) {return vshlq_s32(a, vdupq_n_s32(i));}
    static inline simd_vectori simd_shift_right_i(simd_vectori a, int i) {return vshlq_s32(a, vdupq_n_s32(-i));}
    static inline simd_vectori simd_and_i(simd_vectori a, simd_vectori b) {return vandq_s32(a, b);}
    static inline simd_vectori simd_or_i(simd_vectori a, simd_vectori b) {return vorrq_s32(a, b);}
    static inline simd_vectori simd_andnot_i(simd_vectori a, simd_vectori b) {return vbicq_s32(a, b);}
    static inline simd_vectori simd_cmp_eq_i(simd_vectori a, simd_vectori b) {return vceqq_s32(a, b);}
    static inline simd_vectori simd_cmp_gt_i(simd_vectori a, simd_vectori b) {return vcgtq_s32(a, b);}
    static inline simd_vectori simd_abs_i(simd_vectori a) {return vabsq_s32(a);}

    #define simd_asin vasinq_f32
    #define simd_atan vatanq_f32
    #define simd_sincos vsincosq_f32
    #define simd_sin vsinq_f32
    #define simd_log vlogq_f32
    #define simd_exp vexpq_f32
    #define simd_log2 vlog2q_f32
    #define simd_exp2 vexp2q_f32

#else
    typedef __m256 simd_vector;

    static inline simd_vector simd_add(simd_vector a, simd_vector b) {return _mm256_add_ps(a, b);}
    static inline simd_vector simd_sub(simd_vector a, simd_vector b) {return _mm256_sub_ps(a, b);}
    static inline simd_vector simd_mul(simd_vector a, simd_vector b) {return _mm256_mul_ps(a, b);}
    static inline simd_vector simd_div(simd_vector a, simd_vector b) {return _mm256_div_ps(a, b);}
    static inline simd_vector simd_abs_mask(void) {return _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));}
    static inline simd_vector simd_abs(simd_vector a) {return _mm256_and_ps(a, simd_abs_mask());}
    static inline simd_vector simd_fmad(simd_vector a, simd_vector b, simd_vector c)
    {
    #ifdef __FMA__
        return _mm256_fmadd_ps(a, b, c);
    #else
        return _mm256_add_ps(_mm256_mul_ps(a, b), c);
    #endif
    }
    static inline simd_vector simd_or(simd_vector a, simd_vector b) {return _mm256_or_ps(a, b);}
    static inline simd_vector simd_and(simd_vector a, simd_vector b) {return _mm256_and_ps(a, b);}
    static inline simd_vector simd_andnot(simd_vector a, simd_vector b) {return _mm256_andnot_ps(b, a);}
    static inline simd_vector simd_xor(simd_vector a, simd_vector b) {return _mm256_xor_ps(a, b);}
    static inline simd_vector simd_min(simd_vector a, simd_vector b) {return _mm256_min_ps(a, b);}
    static inline simd_vector simd_max(simd_vector a, simd_vector b) {return _mm256_max_ps(a, b);}
    static inline simd_vector simd_select(simd_vector a, simd_vector b, simd_vector mask) {return _mm256_blendv_ps(a, b, mask);}
    static inline simd_vector simd_splat(float value) {return _mm256_set1_ps(value);}
    static inline simd_vector simd_splat_zero(void) {return _mm256_setzero_ps();}
    static inline simd_vector simd_splat_positive_infinity(void) {return _mm256_castsi256_ps(_mm256_set1_epi32(0x7f800000));}
    static inline simd_vector simd_splat_negative_infinity(void) {return _mm256_castsi256_ps(_mm256_set1_epi32(0xff800000));}
    static inline simd_vector simd_sign_mask(void) {return _mm256_castsi256_ps(_mm256_set1_epi32(0x80000000));}
    static inline simd_vector simd_inv_sign_mask(void) {return _mm256_castsi256_ps(_mm256_set1_epi32(~0x80000000));}
    static inline simd_vector simd_min_normalized(void) {return _mm256_castsi256_ps(_mm256_set1_epi32(0x00800000));} // the smallest non denormalized float number
    static inline simd_vector simd_inv_mant_mask(void){return _mm256_castsi256_ps(_mm256_set1_epi32(~0x7f800000));}
    static inline simd_vector simd_floor(simd_vector a) {return _mm256_floor_ps(a);}
    static inline simd_vector simd_round(simd_vector a) {return _mm256_round_ps(a, _MM_FROUND_NINT);}
    static inline simd_vector simd_cmp_gt(simd_vector a, simd_vector b) {return _mm256_cmp_ps(a, b, _CMP_GT_OQ);}
    static inline simd_vector simd_cmp_ge(simd_vector a, simd_vector b) {return _mm256_cmp_ps(a, b, _CMP_GE_OQ);}
    static inline simd_vector simd_cmp_lt(simd_vector a, simd_vector b) {return _mm256_cmp_ps(a, b, _CMP_LT_OQ);}
    static inline simd_vector simd_cmp_le(simd_vector a, simd_vector b) {return _mm256_cmp_ps(a, b, _CMP_LE_OQ);}
    static inline simd_vector simd_cmp_eq(simd_vector a, simd_vector b) {return _mm256_cmp_ps(a, b, _CMP_EQ_OQ);}
    static inline simd_vector simd_cmp_neq(simd_vector a, simd_vector b) {return _mm256_cmp_ps(a, b, _CMP_NEQ_OQ);}
    static inline simd_vector simd_isnan(simd_vector a) {return _mm256_cmp_ps(a, a, _CMP_NEQ_UQ);}
    static inline simd_vector simd_sqrt(simd_vector a) {return _mm256_sqrt_ps(a);}
    static inline simd_vector simd_neg(simd_vector a) {return _mm256_xor_ps(a, simd_sign_mask());}

    typedef __m256i simd_vectori;
    static inline simd_vectori simd_convert_from_float(simd_vector a) {return _mm256_cvttps_epi32(a);}
    static inline simd_vectori simd_cast_from_float(simd_vector a) {return _mm256_castps_si256(a);}
    static inline simd_vector simd_convert_from_int(simd_vectori a) {return _mm256_cvtepi32_ps(a);}
    static inline simd_vector simd_cast_from_int(simd_vectori a) {return _mm256_castsi256_ps(a);}
    static inline simd_vectori simd_add_i(simd_vectori a, simd_vectori b) {return _mm256_add_epi32(a, b);}
    static inline simd_vectori simd_sub_i(simd_vectori a, simd_vectori b) {return _mm256_sub_epi32(a, b);}
    static inline simd_vectori simd_mul_i(simd_vectori a, simd_vectori b) {return _mm256_mullo_epi32(a, b);}
    static inline simd_vectori simd_splat_i(int i) {return _mm256_set1_epi32(i);}
    static inline simd_vectori simd_splat_zero_i(void) {return _mm256_setzero_si256();}
    static inline simd_vectori simd_shift_left_i(simd_vectori a, int i) {return _mm256_slli_epi32(a, i);}
    static inline simd_vectori simd_shift_right_i(simd_vectori a, int i) {return _mm256_srai_epi32(a, i);}
    static inline simd_vectori simd_and_i(simd_vectori a, simd_vectori b) {return _mm256_and_si256(a, b);}
    static inline simd_vectori simd_or_i(simd_vectori a, simd_vectori b) {return _mm256_or_si256(a, b);}
    static inline simd_vectori simd_abs_i(simd_vectori a) {return _mm256_abs_epi32(a);}
    static inline simd_vectori simd_andnot_i(simd_vectori a, simd_vectori b) {return _mm256_andnot_si256(b, a);}
    static inline simd_vectori simd_cmp_eq_i(simd_vectori a, simd_vectori b) {return _mm256_cmpeq_epi32(a, b);}
    static inline simd_vectori simd_cmp_gt_i(simd_vectori a, simd_vectori b) {return _mm256_cmpgt_epi32(a, b);}


    #define simd_asin mm256_asin_ps
    #define simd_atan mm256_atan_ps
    #define simd_sincos mm256_sincos_ps
    #define simd_sin mm256_sin_ps
    #define simd_exp mm256_exp_ps
    #define simd_log mm256_log_ps
    #define simd_exp2 mm256_exp2_ps
    #define simd_log2 mm256_log2_ps

#endif

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_frexp(simd_vector x, simd_vectori* exponent)
{
    simd_vectori cast_float = simd_cast_from_float(x);
    simd_vectori e = simd_and_i(simd_shift_right_i(cast_float, 23), simd_splat_i(0xff));;
    simd_vectori equal_to_zero = simd_and_i(simd_cmp_eq_i(e, simd_splat_zero_i()), simd_cast_from_float(simd_cmp_eq(x, simd_splat_zero())));
    *exponent = simd_andnot_i(simd_sub_i(e, simd_splat_i(0x7e)), equal_to_zero);
    cast_float = simd_and_i(cast_float, simd_splat_i(0x807fffff));
    cast_float = simd_or_i(cast_float, simd_splat_i(0x3f000000));
    return simd_select(simd_cast_from_int(cast_float), x, simd_cast_from_int(equal_to_zero));
}

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_ldexp(simd_vector x, simd_vectori pw2)
{
    simd_vectori fl = simd_cast_from_float(x);
    simd_vectori e = simd_and_i(simd_shift_right_i(fl, 23), simd_splat_i(0xff));
    e = simd_and_i(simd_add_i(e, pw2), simd_splat_i(0xff));
    simd_vectori is_infinity = simd_cmp_eq_i(e, simd_splat_i(0xff));
    fl = simd_or_i(simd_andnot_i(fl, is_infinity), simd_and_i(fl, simd_splat_i(0xFF800000)));
    fl = simd_or_i(simd_shift_left_i(e, 23), simd_and_i(fl, simd_splat_i(0x807fffff)));
    simd_vector equal_to_zero = simd_cmp_eq(x, simd_splat_zero());
    return simd_andnot(simd_cast_from_int(fl), equal_to_zero);
}

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_polynomial4(simd_vector x, float* coefficients)
{
    simd_vector result = simd_fmad(x, simd_splat(coefficients[0]), simd_splat(coefficients[1]));
    result = simd_fmad(x, result, simd_splat(coefficients[2]));
    result = simd_fmad(x, result, simd_splat(coefficients[3]));
    return result;
}

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_polynomial5(simd_vector x, float* coefficients)
{
    simd_vector result = simd_polynomial4(x, coefficients);
    result = simd_fmad(x, result, simd_splat(coefficients[4]));
    return result;
}

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_polynomial6(simd_vector x, float* coefficients)
{
    simd_vector result = simd_polynomial5(x, coefficients);
    result = simd_fmad(x, result, simd_splat(coefficients[5]));
    return result;
}

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_clamp(simd_vector a, simd_vector range_min, simd_vector range_max) 
{
    return simd_max(simd_min(a, range_max), range_min);
}

//----------------------------------------------------------------------------------------------------------------------
static inline simd_vector simd_sign(simd_vector a)
{
    simd_vector result = simd_select(simd_splat_zero(), simd_splat(-1.f), simd_cmp_lt(a, simd_splat_zero()));
    return simd_select(result, simd_splat( 1.f), simd_cmp_gt(a, simd_splat_zero()));
}

static inline simd_vectori simd_select_i(simd_vectori a, simd_vectori b, simd_vectori mask) { return simd_or_i(simd_andnot_i(a, mask), simd_and_i(b, mask));}
static inline simd_vectori simd_neg_i(simd_vectori a){return simd_sub_i(simd_splat_zero_i(), a);}


//----------------------------------------------------------------------------------------------------------------------
// based on http://gruntthepeon.free.fr/ssemath/
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vlogq_f32(float32x4_t x)
#else
    __m256 mm256_log_ps(__m256 x)
#endif
{
    simd_vector one = simd_splat(1.f);
    simd_vector invalid_mask = simd_cmp_le(x, simd_splat_zero());
    invalid_mask = simd_or(invalid_mask, simd_isnan(x));
    simd_vector input_is_zero = simd_cmp_eq(x, simd_splat_zero());
    simd_vector input_is_infinity = simd_cmp_eq(x, simd_splat_positive_infinity());

    x = simd_max(x, simd_min_normalized());  // cut off denormalized stuff

    simd_vectori emm0 = simd_shift_right_i(simd_cast_from_float(x), 23);
    emm0 = simd_sub_i(emm0, simd_splat_i(0x7f));
    simd_vector e = simd_convert_from_int(emm0);
    
    // keep only the fractional part
    x = simd_and(x, simd_inv_mant_mask());
    x = simd_or(x, simd_splat(0.5f));
    
    e = simd_add(e, one);
    simd_vector mask = simd_cmp_lt(x, simd_splat(0.707106781186547524f));
    simd_vector tmp = simd_and(x, mask);
    x = simd_sub(x, one);
    e = simd_sub(e, simd_and(one, mask));
    x = simd_add(x, tmp);

    simd_vector z = simd_mul(x,x);
    simd_vector y = simd_splat(7.0376836292E-2f);
    y = simd_fmad(y, x, simd_splat(-1.1514610310E-1f));
    y = simd_fmad(y, x, simd_splat(1.1676998740E-1f));
    y = simd_fmad(y, x, simd_splat(-1.2420140846E-1f));
    y = simd_fmad(y, x, simd_splat(+1.4249322787E-1f));
    y = simd_fmad(y, x, simd_splat(-1.6668057665E-1f));
    y = simd_fmad(y, x, simd_splat(+2.0000714765E-1f));
    y = simd_fmad(y, x, simd_splat(-2.4999993993E-1f));
    y = simd_fmad(y, x, simd_splat(+3.3333331174E-1f));
    y = simd_mul(y, x);
    y = simd_mul(y, z);

    tmp = simd_mul(e, simd_splat(-2.12194440e-4f));
    y = simd_add(y, tmp);

    tmp = simd_mul(z, simd_splat(0.5f));
    y = simd_sub(y, tmp);

    tmp = simd_mul(e, simd_splat(0.693359375f));
    x = simd_add(x, y);
    x = simd_add(x, tmp);
    x = simd_or(x, invalid_mask); // NAN/negative arg will be NAN
    x = simd_select(x, simd_splat_negative_infinity(), input_is_zero); // zero arg will be -inf
    x = simd_select(x, simd_splat_positive_infinity(), input_is_infinity); // +inf arg will be +inf

    return x;
}

//----------------------------------------------------------------------------------------------------------------------
// based on https://github.com/redorav/hlslpp/blob/master/include/hlsl%2B%2B_vector_float8.h
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vlog2q_f32(float32x4_t x)
#else
    __m256 mm256_log2_ps(__m256 x)
#endif
{
    simd_vector invalid_mask = simd_cmp_le(x, simd_splat_zero());
    invalid_mask = simd_or(invalid_mask, simd_isnan(x));
    simd_vector input_is_zero = simd_cmp_eq(x, simd_splat_zero());
    simd_vector input_is_infinity = simd_cmp_eq(x, simd_splat_positive_infinity());
    simd_vector one = simd_splat(1.f);
    simd_vectori exp = simd_splat_i(0x7f800000);
    simd_vectori mant = simd_splat_i(0x007fffff);
    simd_vectori i = simd_cast_from_float(x);
    simd_vector e = simd_convert_from_int(simd_sub_i(simd_shift_right_i(simd_and_i(i, exp), 23), simd_splat_i(127)));
    simd_vector m = simd_or(simd_cast_from_int(simd_and_i(i, mant)), one);

    // minimax polynomial fit of log2(x)/(x - 1), for x in range [1, 2[
    simd_vector p = simd_polynomial6(m, (float[]){-3.4436006e-2f, 3.1821337e-1f, -1.2315303f, 2.5988452f, -3.3241990f, 3.1157899f});

    // this effectively increases the polynomial degree by one, but ensures that log2(1) == 0
    p = simd_mul(p, simd_sub(m, one));
    simd_vector result = simd_add(p, e);

    result = simd_or(result, invalid_mask); // NAN/negative arg will be NAN
    result = simd_select(result, simd_splat_negative_infinity(), input_is_zero); // zero arg will be -inf
    result = simd_select(result, simd_splat_positive_infinity(), input_is_infinity); // +inf arg will be +inf

    return result;
}

//----------------------------------------------------------------------------------------------------------------------
// based on http://gruntthepeon.free.fr/ssemath/
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vexpq_f32(float32x4_t x)
#else
    __m256 mm256_exp_ps(__m256 x)
#endif
{
    simd_vector invalid_mask = simd_isnan(x);
    simd_vector input_is_infinity = simd_cmp_eq(x, simd_splat_positive_infinity());
    simd_vector tmp = simd_splat_zero();
    simd_vector fx;
    simd_vector one = simd_splat(1.f);

    x = simd_min(x, simd_splat(88.3762626647949f));
    x = simd_max(x, simd_splat(-88.3762626647949f));

    // express exp(x) as exp(g + n*log(2))
    fx = simd_fmad(x, simd_splat(1.44269504088896341f), simd_splat(0.5f));
    tmp = simd_floor(fx);

    // if greater, substract 1
    simd_vector mask = simd_cmp_gt(tmp, fx);
    mask = simd_and(mask, one);
    fx = simd_sub(tmp, mask);

    tmp = simd_mul(fx, simd_splat(0.693359375f));
    simd_vector z = simd_mul(fx, simd_splat(-2.12194440e-4f));
    x = simd_sub(x, tmp);
    x = simd_sub(x, z);
    z = simd_mul(x, x);
    simd_vector y = simd_polynomial6(x, (float[]) {1.9875691500E-4f, 1.3981999507E-3f, 8.3334519073E-3f,
                                                   4.1665795894E-2f, 1.6666665459E-1f, 5.0000001201E-1f});
    y = simd_fmad(y, z, x);
    y = simd_add(y, one);

    simd_vectori emm0 = simd_convert_from_float(fx);
    emm0 = simd_add_i(emm0, simd_splat_i(0x7f));
    emm0 = simd_shift_left_i(emm0, 23);
    simd_vector pow2n = simd_cast_from_int(emm0);

    simd_vector result = simd_mul(y, pow2n);
    result = simd_or(result, invalid_mask);
    result = simd_select(result, simd_splat_positive_infinity(), input_is_infinity); // +inf arg will be +inf

    return result;
}

//----------------------------------------------------------------------------------------------------------------------
// based on https://github.com/jeremybarnes/cephes/blob/master/single/exp2f.c
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vexp2q_f32(float32x4_t x)
#else
    __m256 mm256_exp2_ps(__m256 x)
#endif
{
    simd_vector invalid_mask = simd_isnan(x);
    simd_vector input_is_infinity = simd_cmp_eq(x, simd_splat_positive_infinity());
    simd_vector equal_to_zero = simd_cmp_eq(x, simd_splat_zero());
    simd_vector one = simd_splat(1.f);

    // clamp values
    x = simd_clamp(x, simd_splat(-127.f), simd_splat(127.f));

#ifdef __MATH_INTRINSINCS_FAST__
    simd_vector ipart = simd_floor(x);
    simd_vector fpart = simd_sub(x, ipart);

    simd_vectori i = simd_shift_left_i(simd_add_i(simd_convert_from_float(ipart), simd_splat_i(127)), 23);
    simd_vector expipart = simd_cast_from_int(i);

    // minimax polynomial fit of 2^x, in range [-0.5, 0.5[
    simd_vector expfpart = simd_polynomial5(fpart, (float[]) {1.3534167e-2f, 5.2011464e-2f, 2.4144275e-1f, 6.9300383e-1f, 1.0000026f});
    simd_vector result = simd_mul(expipart, expfpart);
#else
    simd_vector px = simd_floor(x);
    simd_vectori i0 = simd_convert_from_float(px);
    x = simd_sub(x, px);

    simd_vector above_half = simd_cmp_gt(x, simd_splat(.5f));
    i0 = simd_select_i(i0, simd_add_i(i0, simd_splat_i(1)), simd_cast_from_float(above_half));
    x = simd_select(x, simd_sub(x, one), above_half);

    px = simd_polynomial6(x, (float[]) {1.535336188319500E-004f, 1.339887440266574E-003f, 9.618437357674640E-003f,
                                        5.550332471162809E-002f, 2.402264791363012E-001f, 6.931472028550421E-001f});
    px = simd_fmad(px, x,  one);
    simd_vector result = simd_ldexp(px, i0);
#endif

    result = simd_select(result, one, equal_to_zero);
    result = simd_or(result, invalid_mask);
    result = simd_select(result, simd_splat_positive_infinity(), input_is_infinity); // +inf arg will be +inf
    return result;
}

//----------------------------------------------------------------------------------------------------------------------
// based on http://gruntthepeon.free.fr/ssemath/
#ifdef __MATH__INTRINSICS__NEON__
    void vsincosq_f32(float32x4_t x, float32x4_t* s, float32x4_t* c)
#else
    void mm256_sincos_ps(__m256 x, __m256* s, __m256* c)
#endif
{
    simd_vector xmm1, xmm2, xmm3 = simd_splat_zero(), sign_bit_sin, y;

    sign_bit_sin = x;

    // take the absolute value
    x = simd_and(x, simd_inv_sign_mask());
    // extract the sign bit (upper one)
    sign_bit_sin = simd_and(sign_bit_sin, simd_sign_mask());

    // scale by 4/Pi
    y = simd_mul(x, simd_splat(1.27323954473516f));

    // store the integer part of y in emm2 
    simd_vectori emm2 = simd_convert_from_float(y);

    // j=(j+1) & (~1) (see the cephes sources)
    emm2 = simd_add_i(emm2, simd_splat_i(1));
    emm2 = simd_and_i(emm2, simd_splat_i(~1));
    y = simd_convert_from_int(emm2);

    simd_vectori emm4 = emm2;

    // get the swap sign flag for the sine
    simd_vectori emm0 = simd_and_i(emm2, simd_splat_i(4));
    emm0 = simd_shift_left_i(emm0, 29);
    simd_vector swap_sign_bit_sin = simd_cast_from_int(emm0);

    // get the polynom selection mask for the sine
    emm2 = simd_and_i(emm2, simd_splat_i(2));
    emm2 = simd_cmp_eq_i(emm2, simd_splat_zero_i());
    simd_vector poly_mask = simd_cast_from_int(emm2); 

    // The magic pass: "Extended precision modular arithmetic" 
    //  x = ((x - y * DP1) - y * DP2) - y * DP3; 
    x = simd_fmad(y, simd_splat(-0.78515625f), x);
    x = simd_fmad(y, simd_splat(-2.4187564849853515625e-4f), x);
    x = simd_fmad(y, simd_splat(-3.77489497744594108e-8f), x);

    emm4 = simd_sub_i(emm4, simd_splat_i(2));
    emm4 = simd_andnot_i(simd_splat_i(4), emm4);
    emm4 = simd_shift_left_i(emm4, 29);
    simd_vector sign_bit_cos = simd_cast_from_int(emm4); 

    sign_bit_sin = simd_xor(sign_bit_sin, swap_sign_bit_sin);
    
    // Evaluate the first polynom  (0 <= x <= Pi/4)
    simd_vector z = simd_mul(x,x);
    y = simd_splat(2.443315711809948E-005f);
    y = simd_fmad(y, z, simd_splat(-1.388731625493765E-003f));
    y = simd_fmad(y, z, simd_splat(4.166664568298827E-002f));
    y = simd_mul(y, z);
    y = simd_mul(y, z);
    simd_vector tmp = simd_mul(z, simd_splat(.5f));
    y = simd_sub(y, tmp);
    y = simd_add(y, simd_splat(1.f));

    // Evaluate the second polynom  (Pi/4 <= x <= 0)
    simd_vector y2 = simd_splat(-1.9515295891E-4f);
    y2 = simd_fmad(y2, z, simd_splat(8.3321608736E-3f));
    y2 = simd_fmad(y2, z, simd_splat(-1.6666654611E-1f));
    y2 = simd_mul(y2, z);
    y2 = simd_fmad(y2, x, x);

    // select the correct result from the two polynoms
    xmm3 = poly_mask;
    simd_vector ysin2 = simd_and(y2, xmm3);
    simd_vector ysin1 = simd_andnot(y, xmm3);
    y2 = simd_sub(y2,ysin2);
    y = simd_sub(y, ysin1);

    xmm1 = simd_add(ysin1,ysin2);
    xmm2 = simd_add(y,y2);

    // update the sign
    *s = simd_xor(xmm1, sign_bit_sin);
    *c = simd_xor(xmm2, sign_bit_cos);
}

//----------------------------------------------------------------------------------------------------------------------
#ifdef __MATH__INTRINSICS__NEON__
float32x4_t vsinq_f32(float32x4_t x)
#else
__m256 mm256_sin_ps(__m256 x)
#endif
{
#ifdef __MATH_INTRINSINCS_FAST__
    // range reduction from hlslpp, polynomial computed by lolremez
    simd_vector invtau = simd_splat(1.f/SIMD_MATH_TAU);
    simd_vector tau = simd_splat(SIMD_MATH_TAU);
    simd_vector pi2 = simd_splat(SIMD_MATH_PI2);

    // Range reduction (into [-pi, pi] range)
    // Formula is x = x - round(x / 2pi) * 2pi
    x = simd_sub(x, simd_mul(simd_round(simd_mul(x, invtau)), tau));

    simd_vector gt_pi2 = simd_cmp_gt(x, pi2);
    simd_vector lt_minus_pi2 = simd_cmp_lt(x, simd_neg(pi2));
    simd_vector ox = x;

    // Use identities/mirroring to remap into the range of the minimax polynomial
    simd_vector pi = simd_splat(SIMD_MATH_PI);
    x = simd_select(x, simd_sub(pi, ox), gt_pi2);
    x = simd_select(x, simd_sub(simd_neg(pi), ox), lt_minus_pi2);

    simd_vector x_squared = simd_mul(x, x);
    simd_vector result = simd_polynomial4(x_squared, (float[]){2.6000548e-6f, -1.9806615e-4f, 8.3330173e-3f, -1.6666657e-1f});
    result = simd_mul(result, x_squared);
    result = simd_fmad(result, x, x);

    return result;
#else
    simd_vector sinus, cosinus;
    simd_sincos(x, &sinus, &cosinus);
    return sinus;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
#ifdef __MATH__INTRINSICS__NEON__
float32x4_t vcosq_f32(float32x4_t x)
#else
__m256 mm256_cos_ps(__m256 x)
#endif
{
#ifdef __MATH_INTRINSINCS_FAST__
    return simd_sin(simd_sub(simd_splat(SIMD_MATH_PI2), x));
#else
    simd_vector sinus, cosinus;
    simd_sincos(x, &sinus, &cosinus);
    return cosinus;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vasinq_f32(float32x4_t xx)
#else
    __m256 mm256_asin_ps(__m256 xx)
#endif
{
    simd_vector output_nan = simd_cmp_gt(simd_abs(xx), simd_splat(1.f));
    simd_vector small_value = simd_cmp_lt(simd_abs(xx), simd_splat(1.0e-4f));
    simd_vector a  = simd_abs(xx);
#ifdef __MATH_INTRINSINCS_FAST__
    // based on https://developer.download.nvidia.com/cg/asin.html
    simd_vector negate = simd_select(simd_splat_zero(), simd_splat(1.f), simd_cmp_lt(xx, simd_splat_zero()));
    simd_vector x = a;
    simd_vector result = simd_polynomial4(x, (float[]){-0.0187293f, 0.0742610f, -0.2121144f, 1.5707288f});
    result = simd_sub(simd_splat(SIMD_MATH_PI2), simd_mul(simd_sqrt(simd_sub(simd_splat(1.f), x)), result));
    result = simd_sub(result, simd_mul(simd_mul(simd_splat(2.f), result), negate));
#else
    // based on https://github.com/jeremybarnes/cephes/blob/master/single/asinf.c
    simd_vector x = xx;
    simd_vector sign = simd_sign(xx);
    simd_vector z1 = simd_mul(simd_splat(.5f), simd_sub(simd_splat(1.f), a));
    simd_vector z2 = simd_mul(a, a);
    simd_vector flag = simd_cmp_gt(a, simd_splat(.5f));
    simd_vector z = simd_select(z2, z1, flag);

    x = simd_select(a, simd_sqrt(z), flag);

    simd_vector tmp = simd_polynomial5(z, (float[]) {4.2163199048E-2f, 2.4181311049E-2f, 4.5470025998E-2f, 
                                                    7.4953002686E-2f, 1.6666752422E-1f});
    tmp = simd_mul(tmp, z);
    z = simd_fmad(tmp, x, x);

    tmp = simd_add(z, z);
    tmp = simd_sub(simd_splat(SIMD_MATH_PI2), tmp);
    z = simd_select(z, tmp, flag);
    simd_vector result = simd_mul(z, sign);
#endif
    result = simd_or(result, output_nan);
    result = simd_select(result, xx, small_value);
    return result;
}

//----------------------------------------------------------------------------------------------------------------------
// acos(x) = pi/2 - asin(x)
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vacosq_f32(float32x4_t x)
#else
    __m256 mm256_acos_ps(__m256 x)
#endif
{
#ifdef __MATH_INTRINSINCS_FAST__
    simd_vector negate = simd_select(simd_splat_zero(), simd_splat(1.f), simd_cmp_lt(x, simd_splat_zero()));
    x = simd_abs(x);
    simd_vector result = simd_polynomial4(x, (float[]){-0.0187293f, 0.0742610f, -0.2121144f, 1.5707288f});
    result = simd_mul(result, simd_sqrt(simd_sub(simd_splat(1.f), x)));
    result = simd_sub(result, simd_mul(simd_mul(simd_splat(2.f), negate), result));
    return simd_fmad(negate, simd_splat(SIMD_MATH_PI), result);
#else
    simd_vector out_of_bound = simd_cmp_gt(simd_abs(x), simd_splat(1.f));
    simd_vector result = simd_sub(simd_splat(SIMD_MATH_PI2), simd_asin(x));
    result = simd_or(result, out_of_bound); // out of bound outputs NAN
    return result;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
// based on https://github.com/jeremybarnes/cephes/blob/master/single/atanf.c
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vatanq_f32(float32x4_t xx)
#else
    __m256 mm256_atan_ps(__m256 xx)
#endif
{
    simd_vector sign = simd_sign(xx);
    simd_vector x = simd_abs(xx);
    simd_vector one = simd_splat(1.f);

    // range reduction
    simd_vector above_3pi8 = simd_cmp_gt(x, simd_splat(2.414213562373095f));
    simd_vector above_pi8 = simd_andnot(simd_cmp_gt(x, simd_splat(0.4142135623730950f)), above_3pi8);
    simd_vector y = simd_splat_zero();

    x = simd_select(x, simd_neg(simd_div(one, x)), above_3pi8);
    x = simd_select(x, simd_div(simd_sub(x, one), simd_add(x, one)), above_pi8);
    y = simd_select(y, simd_splat(SIMD_MATH_PI2), above_3pi8);
    y = simd_select(y, simd_splat(SIMD_MATH_PI4), above_pi8);

    // minimax polynomial
    simd_vector z = simd_mul(x, x);
    simd_vector tmp = simd_polynomial4(z, (float[]) {8.05374449538e-2f, -1.38776856032E-1f, 1.99777106478E-1f, -3.33329491539E-1f});
    tmp = simd_mul(tmp, z);
    tmp = simd_fmad(tmp, x, x);
    y = simd_add(tmp, y);
    y = simd_mul(y, sign);

    return y;	
}

//----------------------------------------------------------------------------------------------------------------------
// based on https://mazzo.li/posts/vectorized-atan2.html
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vatan2q_f32(float32x4_t x, float32x4_t y)
#else
    __m256 mm256_atan2_ps(__m256 x, __m256 y)
#endif
{
    simd_vector swap = simd_cmp_lt(simd_abs(x), simd_abs(y));
    simd_vector x_equals_zero = simd_cmp_eq(x, simd_splat_zero());
    simd_vector y_equals_zero = simd_cmp_eq(y, simd_splat_zero());
    simd_vector x_over_y = simd_div(x, y);
    simd_vector y_over_x = simd_div(y, x);
    simd_vector atan_input = simd_select(y_over_x, x_over_y, swap);
    simd_vector result = simd_atan(atan_input);

    simd_vector adjust = simd_select(simd_splat(-SIMD_MATH_PI2), simd_splat(SIMD_MATH_PI2), simd_cmp_ge(atan_input, simd_splat_zero()));
    result = simd_select(result, simd_sub(adjust, result), swap);

    simd_vector x_sign_mask = simd_cmp_lt(x, simd_splat_zero());
    result = simd_add( simd_and(simd_xor(simd_splat(SIMD_MATH_PI), simd_and(simd_sign_mask(), y)), x_sign_mask), result);
    result = simd_select(result, simd_mul(simd_sign(x), simd_splat_zero()), y_equals_zero);
    result = simd_select(result, simd_mul(simd_sign(y), simd_splat(SIMD_MATH_PI2)), x_equals_zero);
    return result;
}

//----------------------------------------------------------------------------------------------------------------------
// based on https://github.com/jeremybarnes/cephes/blob/master/single/cbrtf.c
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vcbrtq_f32(float32x4_t xx)
#else
    __m256 mm256_cbrt_ps(__m256 xx)
#endif
{
    simd_vector one_over_three = simd_splat(0.333333333333f);
    simd_vector sign = simd_sign(xx);
    simd_vector x = simd_abs(xx);
    simd_vector z = x;

    // extract power of 2, leaving mantissa between 0.5 and 1
    simd_vectori exponent;
    x = simd_frexp(x, &exponent);

    // Approximate cube root of number between .5 and 1
    x = simd_polynomial5(x, (float[]) {-0.1346611047335f, 0.5466460136639f, -0.954382247715f, 1.13999833547f, 0.40238979564f});

    // exponent divided by 3
    simd_vectori exponent_is_negative = simd_cmp_gt_i(simd_splat_zero_i(), exponent);
    
    exponent = simd_abs_i(exponent);
    simd_vectori rem = exponent;
    exponent = simd_convert_from_float((simd_mul(simd_convert_from_int(exponent), one_over_three)));
    rem = simd_sub_i(rem, simd_mul_i(exponent, simd_splat_i(3)));

    simd_vector cbrt2 = simd_splat(1.25992104989487316477f);
    simd_vector cbrt4 = simd_splat(1.58740105196819947475f);

    simd_vector rem_equals_1 = simd_cast_from_int(simd_cmp_eq_i(rem, simd_splat_i(1)));
    simd_vector rem_equals_2 = simd_cast_from_int(simd_cmp_eq_i(rem, simd_splat_i(2)));
    simd_vector x1 = simd_mul(x, simd_select(cbrt4, cbrt2, rem_equals_1));
    simd_vector x2 = simd_div(x, simd_select(cbrt4, cbrt2, rem_equals_1));
    x = simd_select(x, simd_select(x1, x2, simd_cast_from_int(exponent_is_negative)), simd_or(rem_equals_1, rem_equals_2));
    exponent = simd_mul_i(exponent, simd_select_i(simd_splat_i(1), simd_splat_i(-1), exponent_is_negative));

    // multiply by power of 2
    x = simd_ldexp(x, exponent);

    // Newton iteration, x -= ( x - (z/(x*x)) ) * 0.333333333333;
    x = simd_sub(x, simd_mul(simd_sub(x, simd_div(z, simd_mul(x, x))), one_over_three));
    x = simd_mul(x, sign);  // if input is zero, sign is also zero

    return x;
}

//----------------------------------------------------------------------------------------------------------------------
// the implementation based https://github.com/jeremybarnes/cephes/blob/master/single/powf.c is **too** slow
// so we use the classic exp(y * log(x))
#ifdef __MATH__INTRINSICS__NEON__
    float32x4_t vpowq_f32(float32x4_t x, float32x4_t y)
#else
    __m256 mm256_pow_ps(__m256 x, __m256 y)
#endif
{
    simd_vector x_equals_zero = simd_cmp_eq(x, simd_splat_zero());
    simd_vector y_equals_zero = simd_cmp_eq(y, simd_splat_zero());
    simd_vector non_integer_power = simd_cmp_neq(y, simd_floor(y));
    simd_vector return_zero = simd_andnot(x_equals_zero, y_equals_zero);
    simd_vector return_one = simd_and(x_equals_zero, y_equals_zero);
    simd_vector return_nan = simd_and(simd_cmp_lt(x, simd_splat_zero()), non_integer_power);

#ifdef __MATH_INTRINSINCS_FAST__
    simd_vector result = simd_exp2(simd_mul(y, simd_log2(x)));
#else
    simd_vector result = simd_exp(simd_mul(y, simd_log(x)));
#endif

    result = simd_andnot(result, return_zero);
    result = simd_select(result, simd_splat(1.f), return_one);
    result = simd_or(result, return_nan);

    return result;
}

#endif  // __MATH__INTRINSICS__IMPLEMENTATION__


