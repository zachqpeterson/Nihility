#pragma once

#include "MathDefines.hpp"

class Math
{
public:
	//Comparisons
	template <typename Type> static Type Abs(const Type& n) { return n < 0 ? -n : n; }
	template <typename Type> static Type Min(const Type& a, const Type& b) { return a > b ? b : a; }
	template <typename Type> static Type Max(const Type& a, const Type& b) { return a < b ? b : a; }
	template <typename Type> static Type Clamp(const Type& n, const Type& min, const Type& max) { return n < min ? min : n > max ? max : n; }
	template <typename Type> static Type Sign(const Type& n) { return (Type)((n > 0) - (n < 0)); }
	template <FloatingPoint Type> static I64 Floor(const Type& n) { return n >= 0.0 ? (I64)n : (I64)n - 1; }
	template <FloatingPoint Type> static Type FloorF(const Type& n) { return n >= 0.0 ? n : n - 1; }
	template <FloatingPoint Type> static I64 Ceiling(const Type& n) { return (n - (I64)n) < 0.0 ? (I64)n : (I64)n + 1; }
	template <FloatingPoint Type> static Type CeilingF(const Type& n) { return (n - n) < 0.0 ? n : n + 1; }
	template <FloatingPoint Type> static Type Round(const Type& n) { return (Type)(I64)(n + 0.5); }
	template <FloatingPoint Type> static Type RoundI(const Type& n) { return (I64)(n + 0.5); }
	template <FloatingPoint Type> static Type Mod(const Type& n, const Type& d) { return n - d * FloorF(n / d); }

	template <FloatingPoint Type> static Type DegToRad(const Type& deg) { return (Type)(deg * DEG2RAD); }
	template <FloatingPoint Type> static Type RadToDeg(const Type& rad) { return (Type)(rad * RAD2DEG); }

	static bool Zero(F32 f) { return f < FLOAT_EPSILON && f > -FLOAT_EPSILON; }
	static bool Zero(F64 f) { return f < DOUBLE_EPSILON && f > -DOUBLE_EPSILON; }
	static bool NaN(F32 f);
	static bool NaN(F64 f);
	static bool Inf(F32 f);
	static bool Inf(F64 f);

	//Interpolation


private:

	STATIC_CLASS(Math);
};