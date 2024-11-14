#pragma once

#include "Defines.hpp"

#ifdef NH_SIMD_AVX

#include <immintrin.h>

typedef __m256 F256;
typedef __m256d D256;
typedef __m256i I256;
typedef __m256 FloatW;

inline static const F256 ZeroF256 = _mm256_setzero_ps();
inline static const D256 ZeroD256 = _mm256_setzero_pd();
inline static const D256 ZeroI256 = _mm256_setzero_si256();
inline static const FloatW ZeroFloatW = _mm256_setzero_ps();

static inline FloatW SplatW(F32 scalar) { return _mm256_set1_ps(scalar); }
static inline FloatW operator+(const FloatW& a, const FloatW& b) { return _mm256_add_ps(a, b); }
static inline FloatW operator-(const FloatW& a, const FloatW& b) { return _mm256_sub_ps(a, b); }
static inline FloatW operator*(const FloatW& a, const FloatW& b) { return _mm256_mul_ps(a, b); }
static inline FloatW operator/(const FloatW& a, const FloatW& b) { return _mm256_div_ps(a, b); }
static inline FloatW operator&(const FloatW& a, const FloatW& b) { return _mm256_and_ps(a, b); }
static inline FloatW operator|(const FloatW& a, const FloatW& b) { return _mm256_or_ps(a, b); }
static inline FloatW& operator+=(FloatW& a, const FloatW& b) { a = _mm256_add_ps(a, b); return a; }
static inline FloatW& operator-=(FloatW& a, const FloatW& b) { a = _mm256_sub_ps(a, b); return a; }
static inline FloatW& operator*=(FloatW& a, const FloatW& b) { a = _mm256_mul_ps(a, b); return a; }
static inline FloatW& operator/=(FloatW& a, const FloatW& b) { a = _mm256_div_ps(a, b); return a; }
static inline FloatW& operator&=(FloatW& a, const FloatW& b) { a = _mm256_and_ps(a, b); return a; }
static inline FloatW& operator|=(FloatW& a, const FloatW& b) { a = _mm256_or_ps(a, b); return a; }
static inline FloatW MulAddW(const FloatW& a, const FloatW& b, const FloatW& c) { return _mm256_add_ps(_mm256_mul_ps(b, c), a); }
static inline FloatW MulSubW(const FloatW& a, const FloatW& b, const FloatW& c) { return _mm256_sub_ps(a, _mm256_mul_ps(b, c)); }
static inline FloatW operator<(const FloatW& a, const FloatW& b) { return _mm256_cmp_ps(a, b, _CMP_LT_OQ); }
static inline FloatW operator>(const FloatW& a, const FloatW& b) { return _mm256_cmp_ps(a, b, _CMP_GT_OQ); }
static inline FloatW operator<=(const FloatW& b) { return _mm256_cmp_ps(a, b, _CMP_LE_OQ); }
static inline FloatW operator>=(const FloatW& b) { return _mm256_cmp_ps(a, b, _CMP_GE_OQ); }
static inline FloatW operator==(const FloatW& b) { return _mm256_cmp_ps(a, b, _CMP_EQ_OQ); }
static inline FloatW MinW(const FloatW& a, const FloatW& b) { return _mm256_min_ps(a, b); }
static inline FloatW MaxW(const FloatW& a, const FloatW& b) { return _mm256_max_ps(a, b); }
static inline FloatW BlendW(const FloatW& a, const FloatW& b, const FloatW& mask) { return _mm256_blendv_ps(a, b, mask); }

#elif defined NH_SIMD_SSE || defined NH_SIMD_SSE2

#include <xmmintrin.h>
#include <emmintrin.h>

typedef __m128 F128;
typedef __m128 FloatW;

static inline const F128 ZeroFloatW = _mm_setzero_ps();

static inline FloatW SplatW(F32 scalar) { return _mm_set1_ps(scalar); }
static inline FloatW SetW(F32 a, F32 b, F32 c, F32 d) { return _mm_setr_ps(a, b, c, d); }
static inline FloatW operator+(const FloatW& a, const FloatW& b) { return _mm_add_ps(a, b); }
static inline FloatW operator-(const FloatW& a, const FloatW& b) { return _mm_sub_ps(a, b); }
static inline FloatW operator*(const FloatW& a, const FloatW& b) { return _mm_mul_ps(a, b); }
static inline FloatW operator/(const FloatW& a, const FloatW& b) { return _mm_div_ps(a, b); }
static inline FloatW operator&(const FloatW& a, const FloatW& b) { return _mm_and_ps(a, b); }
static inline FloatW operator|(const FloatW& a, const FloatW& b) { return _mm_or_ps(a, b); }
static inline FloatW& operator+=(FloatW& a, const FloatW& b) { a = _mm_add_ps(a, b); return a; }
static inline FloatW& operator-=(FloatW& a, const FloatW& b) { a = _mm_sub_ps(a, b); return a; }
static inline FloatW& operator*=(FloatW& a, const FloatW& b) { a = _mm_mul_ps(a, b); return a; }
static inline FloatW& operator/=(FloatW& a, const FloatW& b) { a = _mm_div_ps(a, b); return a; }
static inline FloatW& operator&=(FloatW& a, const FloatW& b) { a = _mm_and_ps(a, b); return a; }
static inline FloatW& operator|=(FloatW& a, const FloatW& b) { a = _mm_or_ps(a, b); return a; }
static inline FloatW MulAddW(const FloatW& a, const FloatW& b, const FloatW& c) { return _mm_add_ps(a, _mm_mul_ps(b, c)); }
static inline FloatW MulSubW(const FloatW& a, const FloatW& b, const FloatW& c) { return _mm_sub_ps(a, _mm_mul_ps(b, c)); }
static inline FloatW operator>(const FloatW& a, const FloatW& b) { return _mm_cmpgt_ps(a, b); }
static inline FloatW operator<(const FloatW& a, const FloatW& b) { return _mm_cmplt_ps(a, b); }
static inline FloatW operator>=(const FloatW& a, const FloatW& b) { return _mm_cmpge_ps(a, b); }
static inline FloatW operator<=(const FloatW& a, const FloatW& b) { return _mm_cmple_ps(a, b); }
static inline FloatW operator==(const FloatW& a, const FloatW& b) { return _mm_cmpeq_ps(a, b); }
static inline FloatW MinW(const FloatW& a, const FloatW& b) { return _mm_min_ps(a, b); }
static inline FloatW MaxW(const FloatW& a, const FloatW& b) { return _mm_max_ps(a, b); }
static inline FloatW UnpackLoW(const FloatW& a, const FloatW& b) { return _mm_unpacklo_ps(a, b); }
static inline FloatW UnpackHiW(const FloatW& a, const FloatW& b) { return _mm_unpackhi_ps(a, b); }
static inline FloatW BlendW(const FloatW& a, const FloatW& b, const FloatW& mask) { return _mm_or_ps(_mm_and_ps(mask, b), _mm_andnot_ps(mask, a)); }
static inline FloatW LoadW(const F32* data) { return _mm_load_ps(data); }
static inline void StoreW(F32* data, const FloatW& a) { _mm_store_ps(data, a); }

#elif defined NH_SIMD_NEON

#include <arm_neon.h>

typedef float32x4_t F128;
typedef float32x4_t FloatW;

static inline const F128 ZeroFloatW = vdupq_n_f32(0.0f);

static inline FloatW b2SplatW(F32 scalar) { return vdupq_n_f32(scalar); }
static inline FloatW b2SetW(F32 a, F32 b, F32 c, F32 d) { return vld1q_f32({ a, b, c, d }); }
static inline FloatW operator+(const FloatW& a, const FloatW& b) { return vaddq_f32(a, b); }
static inline FloatW operator-(const FloatW& a, const FloatW& b) { return vsubq_f32(a, b); }
static inline FloatW operator*(const FloatW& a, const FloatW& b) { return vmulq_f32(a, b); }
static inline FloatW operator/(const FloatW& a, const FloatW& b) { return vdivq_f32(a, b); }
static inline FloatW operator&(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b))); }
static inline FloatW operator|(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b))); }
static inline FloatW& operator+=(FloatW& a, const FloatW& b) { a = vaddq_f32(a, b); return a; }
static inline FloatW& operator-=(FloatW& a, const FloatW& b) { a = vsubq_f32(a, b); return a; }
static inline FloatW& operator*=(FloatW& a, const FloatW& b) { a = vmulq_f32(a, b); return a; }
static inline FloatW& operator/=(FloatW& a, const FloatW& b) { a = vdivq_f32(a, b); return a; }
static inline FloatW& operator&=(FloatW& a, const FloatW& b) { a = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b))); return a; }
static inline FloatW& operator|=(FloatW& a, const FloatW& b) { a = vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b))); return a; }
static inline FloatW b2MulAddW(const FloatW& a, const FloatW& b, const FloatW& c) { return vmlaq_f32(a, b, c); }
static inline FloatW b2MulSubW(const FloatW& a, const FloatW& b, const FloatW& c) { return vmlsq_f32(a, b, c); }
static inline FloatW operator<(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vcltq_f32(a, b)); }
static inline FloatW operator>(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vcgtq_f32(a, b)); }
static inline FloatW operator<=(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vcleq_f32(a, b)); }
static inline FloatW operator>=(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vcgeq_f32(a, b)); }
static inline FloatW operator==(const FloatW& a, const FloatW& b) { return vreinterpretq_f32_u32(vceqq_f32(a, b)); }
static inline FloatW b2MinW(const FloatW& a, const FloatW& b) { return vminq_f32(a, b); }
static inline FloatW b2MaxW(const FloatW& a, const FloatW& b) { return vmaxq_f32(a, b); }
static inline FloatW b2UnpackLoW(const FloatW& a, const FloatW& b) { return vzip1q_f32(a, b); }
static inline FloatW b2UnpackHiW(const FloatW& a, const FloatW& b) { return vzip2q_f32(a, b); }
static inline FloatW b2BlendW(const FloatW& a, const FloatW& b, const FloatW& mask) { return vbslq_f32(vreinterpretq_u32_f32(mask), b, a); }
static inline FloatW b2LoadW(const float32_t* data) { return vld1q_f32(data); }
static inline void b2StoreW(float32_t* data, const FloatW& a) { return vst1q_f32(data, a); }

#else

struct FloatW
{
	F32 x, y, z, w;
};

static inline const FloatW ZeroFloatW = { 0.0f, 0.0f, 0.0f, 0.0f };

static inline FloatW SplatW(F32 scalar) { return { scalar, scalar, scalar, scalar }; }
static inline FloatW operator+(const FloatW& a, const FloatW& b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
static inline FloatW operator-(const FloatW& a, const FloatW& b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
static inline FloatW operator*(const FloatW& a, const FloatW& b) { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
static inline FloatW operator/(const FloatW& a, const FloatW& b) { return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }
static inline FloatW operator&(const FloatW& a, const FloatW& b) { return { (F32)(a.x && b.x), (F32)(a.y && b.y), (F32)(a.z && b.z), (F32)(a.w && b.w) }; }
static inline FloatW operator|(const FloatW& a, const FloatW& b) { return { (F32)(a.x || b.x), (F32)(a.y || b.y), (F32)(a.z || b.z), (F32)(a.w || b.w) }; }
static inline FloatW& operator+=(FloatW& a, const FloatW& b) { a = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; return a; }
static inline FloatW& operator-=(FloatW& a, const FloatW& b) { a = { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; return a; }
static inline FloatW& operator*=(FloatW& a, const FloatW& b) { a = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; return a; }
static inline FloatW& operator/=(FloatW& a, const FloatW& b) { a = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; return a; }
static inline FloatW& operator&=(FloatW& a, const FloatW& b) { a = { (F32)(a.x && b.x), (F32)(a.y && b.y), (F32)(a.z && b.z), (F32)(a.w && b.w) }; return a; }
static inline FloatW& operator|=(FloatW& a, const FloatW& b) { a = { (F32)(a.x || b.x), (F32)(a.y || b.y), (F32)(a.z || b.z), (F32)(a.w || b.w) }; return a; }
static inline FloatW MulAddW(const FloatW& a, const FloatW& b, const FloatW& c) { return { a.x + b.x * c.x, a.y + b.y * c.y, a.z + b.z * c.z, a.w + b.w * c.w }; }
static inline FloatW MulSubW(const FloatW& a, const FloatW& b, const FloatW& c) { return { a.x - b.x * c.x, a.y - b.y * c.y, a.z - b.z * c.z, a.w - b.w * c.w }; }
static inline FloatW operator<(const FloatW& a, const FloatW& b) { return { (F32)(a.x < b.x), (F32)(a.y < b.y), (F32)(a.z < b.z), (F32)(a.w < b.w) }; }
static inline FloatW operator>(const FloatW& a, const FloatW& b) { return { (F32)(a.x > b.x), (F32)(a.y > b.y), (F32)(a.z > b.z), (F32)(a.w > b.w) }; }
static inline FloatW operator<=(const FloatW& a, const FloatW& b) { return { (F32)(a.x <= b.x), (F32)(a.y <= b.y), (F32)(a.z <= b.z), (F32)(a.w <= b.w) }; }
static inline FloatW operator>=(const FloatW& a, const FloatW& b) { return { (F32)(a.x >= b.x), (F32)(a.y >= b.y), (F32)(a.z >= b.z), (F32)(a.w >= b.w) }; }
static inline FloatW operator==(const FloatW& a, const FloatW& b) { return { (F32)(a.x == b.x), (F32)(a.y == b.y), (F32)(a.z == b.z), (F32)(a.w == b.w) }; }
static inline FloatW MinW(const FloatW& a, const FloatW& b) { return { a.x <= b.x ? a.x : b.x, a.y <= b.y ? a.y : b.y, a.z <= b.z ? a.z : b.z, a.w <= b.w ? a.w : b.w }; }
static inline FloatW MaxW(const FloatW& a, const FloatW& b) { return { a.x >= b.x ? a.x : b.x, a.y >= b.y ? a.y : b.y, a.z >= b.z ? a.z : b.z, a.w >= b.w ? a.w : b.w }; }
static inline FloatW BlendW(const FloatW& a, const FloatW& b, const FloatW& mask) { return { mask.x ? b.x : a.x, mask.y ? b.y : a.y, mask.z ? b.z : a.z, mask.w ? b.w : a.w }; }

#endif

#if defined NH_COMPILER_MSVC && (defined _M_IX86 || defined _M_X64)
static inline void Pause() { _mm_pause(); }
#elif defined NH_COMPILER_MSVC && (defined _M_ARM || defined _M_ARM64)
static inline void Pause() { __yield(); }
#else
static inline void Pause() {}
#endif

struct Vector2W
{
	FloatW x, y;
};

struct Quaternion2W
{
	FloatW c, s;
};

static inline FloatW DotW(const Vector2W& a, const Vector2W& b)
{
	return a.x * b.x + a.y * b.y;
}

static inline FloatW CrossW(const Vector2W& a, const Vector2W& b)
{
	return a.x * b.y - a.y * b.x;
}

static inline Vector2W RotateVectorW(const Quaternion2W& q, const Vector2W& v)
{
	return { q.c * v.x - q.s * v.y, q.s * v.x + q.c * v.y };
}