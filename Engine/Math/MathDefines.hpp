#pragma once

#include "Defines.hpp"

static inline constexpr F32 PI_F = 3.141592653589793F;
static inline constexpr F64 PI = 3.141592653589793;
static inline constexpr F32 TWO_PI_F = PI_F * 2.0F;
static inline constexpr F64 TWO_PI = PI * 2.0;
static inline constexpr F32 HALF_PI_F = PI_F * 0.5F;
static inline constexpr F64 HALF_PI = PI * 0.5;
static inline constexpr F32 QUARTER_PI_F = PI_F * 0.25F;
static inline constexpr F64 QUARTER_PI = PI * 0.25;
static inline constexpr F32 ONE_OVER_PI_F = 1.0F / PI_F;
static inline constexpr F64 ONE_OVER_PI = 1.0 / PI;
static inline constexpr F32 ONE_OVER_TWO_PI_F = 1.0F / TWO_PI_F;
static inline constexpr F64 ONE_OVER_TWO_PI = 1.0 / TWO_PI;
static inline constexpr F64 SQRT_TWO_F = 1.414213562373095f;
static inline constexpr F64 SQRT_TWO = 1.414213562373095;
static inline constexpr F32 DEG_TO_RAD_F = PI_F / 180.0F;
static inline constexpr F64 DEG_TO_RAD = PI / 180.0;
static inline constexpr F32 RAD_TO_DEG_F = 180.0F / PI_F;
static inline constexpr F64 RAD_TO_DEG = 180.0 / PI;
static inline constexpr F32 F32_EPSILON = 1.192092896e-06F;
static inline constexpr F64 F64_EPSILON = 2.22045e-16;
static inline constexpr F32 F32_INFINITY = Traits<F32>::Infinity;
static inline constexpr F64 F64_INFINITY = Traits<F64>::Infinity;

static inline constexpr F32 ONE_THIRD_F = 1.0f / 3.0f;
static inline constexpr F64 ONE_THIRD = 1.0 / 3.0;
static inline constexpr F32 TWO_THIRDS_F = 2.0f / 3.0f;
static inline constexpr F64 TWO_THIRDS = 2.0 / 3.0;