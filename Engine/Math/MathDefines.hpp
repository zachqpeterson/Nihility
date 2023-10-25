#pragma once

#include "Defines.hpp"

template<FloatingPoint Type> static inline constexpr Type E_T = 2.718281828459045;
template<FloatingPoint Type> static inline constexpr Type PI_T = 3.141592653589793;
template<FloatingPoint Type> static inline constexpr Type LOG_TWO_T = 0.693147180559945;
template<FloatingPoint Type> static inline constexpr Type LOG_TEN_T = 2.302585092994046;
template<FloatingPoint Type> static inline constexpr Type TWO_PI_T = PI_T<Type> * 2.0;
template<FloatingPoint Type> static inline constexpr Type HALF_PI_T = PI_T<Type> * 0.5;
template<FloatingPoint Type> static inline constexpr Type QUARTER_PI_T = PI_T<Type> * 0.25;
template<FloatingPoint Type> static inline constexpr Type PI_REC_T = 1.0 / PI_T<Type>;
template<FloatingPoint Type> static inline constexpr Type TWO_PI_REC_T = 1.0 / TWO_PI_T<Type>;
template<FloatingPoint Type> static inline constexpr Type SQRT_TWO_T = 1.414213562373095;
template<FloatingPoint Type> static inline constexpr Type DEG_TO_RAD_T = PI_T<Type> / 180.0;
template<FloatingPoint Type> static inline constexpr Type RAD_TO_DEG_T = 180.0 / PI_T<Type>;
template<FloatingPoint Type> static inline constexpr Type ONE_THIRD_T = 1.0 / 3.0;
template<FloatingPoint Type> static inline constexpr Type TWO_THIRDS_T = 2.0 / 3.0;

static inline constexpr F32 E_F = E_T<F32>;
static inline constexpr F64 E = E_T<F64>;
static inline constexpr F32 PI_F = PI_T<F32>;
static inline constexpr F64 PI = PI_T<F64>;
static inline constexpr F32 LOG_TWO_F = LOG_TWO_T<F32>;
static inline constexpr F64 LOG_TWO = LOG_TWO_T<F64>;
static inline constexpr F32 LOG_TEN_F = LOG_TEN_T<F32>;
static inline constexpr F64 LOG_TEN = LOG_TEN_T<F64>;
static inline constexpr F32 TWO_PI_F = TWO_PI_T<F32>;
static inline constexpr F64 TWO_PI = TWO_PI_T<F64>;
static inline constexpr F32 HALF_PI_F = HALF_PI_T<F32>;
static inline constexpr F64 HALF_PI = HALF_PI_T<F64>;
static inline constexpr F32 QUARTER_PI_F = QUARTER_PI_T<F32>;
static inline constexpr F64 QUARTER_PI = QUARTER_PI_T<F64>;
static inline constexpr F32 PI_REC_F = PI_REC_T<F32>;
static inline constexpr F64 PI_REC = PI_REC_T<F64>;
static inline constexpr F32 TWO_PI_REC_F = TWO_PI_REC_T<F32>;
static inline constexpr F64 TWO_PI_REC = TWO_PI_REC_T<F64>;
static inline constexpr F64 SQRT_TWO_F = SQRT_TWO_T<F32>;
static inline constexpr F64 SQRT_TWO = SQRT_TWO_T<F64>;
static inline constexpr F32 DEG_TO_RAD_F = DEG_TO_RAD_T<F32>;
static inline constexpr F64 DEG_TO_RAD = DEG_TO_RAD_T<F64>;
static inline constexpr F32 RAD_TO_DEG_F = RAD_TO_DEG_T<F32>;
static inline constexpr F64 RAD_TO_DEG = RAD_TO_DEG_T<F64>;

static inline constexpr F32 ONE_THIRD_F = ONE_THIRD_T<F32>;
static inline constexpr F64 ONE_THIRD = ONE_THIRD_T<F64>;
static inline constexpr F32 TWO_THIRDS_F = TWO_THIRDS_T<F32>;
static inline constexpr F64 TWO_THIRDS = TWO_THIRDS_T<F64>;

