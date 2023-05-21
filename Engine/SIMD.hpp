#pragma once

#include "Defines.hpp"

#if defined _MSC_VER
#	if (defined _M_AMD64 || defined _M_X64) || _M_IX86_FP == 2
#		define NH_SSE2
#	elif _M_IX86_FP == 1
#		define NH_SSE
#	endif
#endif



#if defined __AVX__

#include <immintrin.h>
#define NH_AVX

typedef __m256 F256;
typedef __m256d D256;
typedef __m256i I256;

inline static const F256 ZeroF256 = _mm256_setzero_ps();
inline static const D256 ZeroD256 = _mm256_setzero_pd();
inline static const D256 ZeroI256 = _mm256_setzero_si256();

#elif defined NH_SSE || defined NH_SSE2

#include <xmmintrin.h>
#include <emmintrin.h>

typedef __m128 M128;

inline static const M128 Zero128 = _mm_setzero_ps();

#elif defined __ARM_NEON

#include <arm_neon.h>
#define NH_ARM_NEON

#else

#endif

