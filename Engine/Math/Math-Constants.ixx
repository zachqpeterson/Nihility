module;

#include "Defines.hpp"
#include "TypeTraits.hpp"

export module Math:Constants;

export template<FloatingPoint Type> constexpr const Type E_T = Type(2.718281828459045L);
export template<FloatingPoint Type> constexpr const Type PI_T = Type(3.1415926535897932385L);
export template<FloatingPoint Type> constexpr const Type LOG_TWO_T = Type(0.693147180559945L);
export template<FloatingPoint Type> constexpr const Type LOG_TEN_T = Type(2.302585092994046L);
export template<FloatingPoint Type> constexpr const Type TWO_PI_T = PI_T<Type> *Type(2.0L);
export template<FloatingPoint Type> constexpr const Type HALF_PI_T = PI_T<Type> *Type(0.5L);
export template<FloatingPoint Type> constexpr const Type QUARTER_PI_T = PI_T<Type> *Type(0.25L);
export template<FloatingPoint Type> constexpr const Type PI_REC_T = Type(1.0L) / PI_T<Type>;
export template<FloatingPoint Type> constexpr const Type TWO_PI_REC_T = Type(1.0L) / TWO_PI_T<Type>;
export template<FloatingPoint Type> constexpr const Type SQRT_TWO_T = Type(1.414213562373095L);
export template<FloatingPoint Type> constexpr const Type SQRT_TWO_REC_T = Type(1.0L) / SQRT_TWO_T<Type>;
export template<FloatingPoint Type> constexpr const Type DEG_TO_RAD_T = PI_T<Type> / Type(180.0L);
export template<FloatingPoint Type> constexpr const Type RAD_TO_DEG_T = Type(180.0L) / PI_T<Type>;
export template<FloatingPoint Type> constexpr const Type ONE_THIRD_T = Type(1.0L) / Type(3.0L);
export template<FloatingPoint Type> constexpr const Type TWO_THIRDS_T = Type(2.0L) / Type(3.0L);

export constexpr inline F32 E_F = E_T<F32>;
export constexpr inline F64 E = E_T<F64>;
export constexpr inline F32 PI_F = PI_T<F32>;
export constexpr inline F64 PI = PI_T<F64>;
export constexpr inline F32 LOG_TWO_F = LOG_TWO_T<F32>;
export constexpr inline F64 LOG_TWO = LOG_TWO_T<F64>;
export constexpr inline F32 LOG_TEN_F = LOG_TEN_T<F32>;
export constexpr inline F64 LOG_TEN = LOG_TEN_T<F64>;
export constexpr inline F32 TWO_PI_F = TWO_PI_T<F32>;
export constexpr inline F64 TWO_PI = TWO_PI_T<F64>;
export constexpr inline F32 HALF_PI_F = HALF_PI_T<F32>;
export constexpr inline F64 HALF_PI = HALF_PI_T<F64>;
export constexpr inline F32 QUARTER_PI_F = QUARTER_PI_T<F32>;
export constexpr inline F64 QUARTER_PI = QUARTER_PI_T<F64>;
export constexpr inline F32 PI_REC_F = PI_REC_T<F32>;
export constexpr inline F64 PI_REC = PI_REC_T<F64>;
export constexpr inline F32 TWO_PI_REC_F = TWO_PI_REC_T<F32>;
export constexpr inline F64 TWO_PI_REC = TWO_PI_REC_T<F64>;
export constexpr inline F64 SQRT_TWO_F = SQRT_TWO_T<F32>;
export constexpr inline F64 SQRT_TWO = SQRT_TWO_T<F64>;
export constexpr inline F64 SQRT_TWO_REC_F = SQRT_TWO_REC_T<F32>;
export constexpr inline F64 SQRT_TWO_REC = SQRT_TWO_REC_T<F64>;
export constexpr inline F32 DEG_TO_RAD_F = DEG_TO_RAD_T<F32>;
export constexpr inline F64 DEG_TO_RAD = DEG_TO_RAD_T<F64>;
export constexpr inline F32 RAD_TO_DEG_F = RAD_TO_DEG_T<F32>;
export constexpr inline F64 RAD_TO_DEG = RAD_TO_DEG_T<F64>;
export constexpr inline F32 ONE_THIRD_F = ONE_THIRD_T<F32>;
export constexpr inline F64 ONE_THIRD = ONE_THIRD_T<F64>;
export constexpr inline F32 TWO_THIRDS_F = TWO_THIRDS_T<F32>;
export constexpr inline F64 TWO_THIRDS = TWO_THIRDS_T<F64>;