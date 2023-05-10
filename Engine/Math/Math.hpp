#pragma once

#include "MathDefines.hpp"

#include "Containers\String.hpp"
#include <math.h>

struct Vector2;
struct Vector3;
struct Vector4;
struct Vector2Int;
struct Vector3Int;
struct Vector4Int;

struct Matrix2;
struct Matrix3;
struct Matrix4;

struct Quaternion2;
struct Quaternion3;

template <class Type> concept VectorType = AnyOf<RemovedQuals<Type>, Vector2, Vector3, Vector4, Vector2Int, Vector3Int, Vector4Int>;

class Math
{
public:
	//TRIGONOMETRY
	template <FloatingPoint Type> static Type Sin(Type f) { if constexpr (IsSame<Type, F32>) { return sinf(f); } else { return sin(f); } }
	template <FloatingPoint Type> static Type Cos(Type f) { if constexpr (IsSame<Type, F32>) { return cosf(f); } else { return cos(f); } }
	template <FloatingPoint Type> static Type Tan(Type f) { if constexpr (IsSame<Type, F32>) { return tanf(f); } else { return tan(f); } }
	template <FloatingPoint Type> static Type SinH(Type f) { if constexpr (IsSame<Type, F32>) { return sinfh(f); } else { return sinh(f); } }
	template <FloatingPoint Type> static Type CosH(Type f) { if constexpr (IsSame<Type, F32>) { return cosfh(f); } else { return cosh(f); } }
	template <FloatingPoint Type> static Type TanH(Type f) { if constexpr (IsSame<Type, F32>) { return tanfh(f); } else { return tanh(f); } }

	template <FloatingPoint Type> static Type Asin(Type f) { if constexpr (IsSame<Type, F32>) { return asinf(f); } else { return asin(f); } }
	template <FloatingPoint Type> static Type Acos(Type f) { if constexpr (IsSame<Type, F32>) { return acosf(f); } else { return acos(f); } }
	template <FloatingPoint Type> static Type Atan(Type f) { if constexpr (IsSame<Type, F32>) { return atanf(f); } else { return atan(f); } }
	template <FloatingPoint Type> static Type AsinH(Type f) { if constexpr (IsSame<Type, F32>) { return asinfh(f); } else { return asinh(f); } }
	template <FloatingPoint Type> static Type AcosH(Type f) { if constexpr (IsSame<Type, F32>) { return acosfh(f); } else { return acosh(f); } }
	template <FloatingPoint Type> static Type AtanH(Type f) { if constexpr (IsSame<Type, F32>) { return atanfh(f); } else { return atanh(f); } }

	template <FloatingPoint Type> static Type LogE(Type f) { if constexpr (IsSame<Type, F32>) { return logf(f); } else { return log(f); } }
	template <FloatingPoint Type> static Type Log2(Type f) { if constexpr (IsSame<Type, F32>) { return log2f(f); } else { return log2(f); } }
	template <FloatingPoint Type> static Type Log10(Type f) { if constexpr (IsSame<Type, F32>) { return log10f(f); } else { return log10(f); } }
	template <FloatingPoint Type> static Type LogN(Type f, Type b) { if constexpr (IsSame<Type, F32>) { return log2f(f) / log2f(b); } else { return log2(f) / log2(b); } }

	template <FloatingPoint Type> static Type Sqrt(Type f) { if constexpr (IsSame<Type, F32>) { return sqrtf(f); } else { return sqrt(f); } }
	template <FloatingPoint Type> static Type Cbrt(Type f) { if constexpr (IsSame<Type, F32>) { return cbrtf(f); } else { return cbrt(f); } }
	template <FloatingPoint Type> static Type InvSqrt(Type f) { if constexpr (IsSame<Type, F32>) { return 1.0f / sqrtf(f); } else { return 1.0 / sqrt(f); } }

	template <FloatingPoint Type> static Type DegToRad(Type deg) { if constexpr (IsSame<Type, F32>) { return deg * DEG_TO_RAD_F; } else { return deg * DEG_TO_RAD; } }
	template <FloatingPoint Type> static Type RadToDeg(Type rad) { if constexpr (IsSame<Type, F32>) { return rad * RAD_TO_DEG_F; } else { return rad * RAD_TO_DEG; } }

	//COMPARISONS
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
	template <FloatingPoint Type> static Type Closest(Type n, Type a, Type b) { return n < (b + a) * 0.5f ? a : b; }
	template <Integer Type> static Type Closest(Type n, Type a, Type b) { return n < (b + a) >> 1 ? a : b; }

	template <FloatingPoint Type> static bool Zero(Type f) { if constexpr (IsSame<Type, F32>) { return f < FLOAT_EPSILON&& f > -FLOAT_EPSILON; }
		else { return f < DOUBLE_EPSILON&& f > -DOUBLE_EPSILON; } }
	template <FloatingPoint Type> static bool NaN(Type f) { return isnan(f); }
	template <FloatingPoint Type> static bool Inf(Type f) { return isinf(f); }

	//INTERPOLATION
	template <FloatingPoint Type> static Type Lerp(Type a, Type b, Type t) { return a + (b - a) * t; }
	template <FloatingPoint Type> static Type InvLerp(Type a, Type b, Type t) { return (t - a) / (b - a); }
	template <VectorType Type> static Type Lerp(const Type& a, const Type& b, F32 t); //TODO:
	template <VectorType Type> static Type InvLerp(const Type& a, const Type& b, F32 t); //TODO:
	template <FloatingPoint Type> static Type MoveTowards(Type a, Type b, Type t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }

	//EXPONENTS
	template <FloatingPoint Type> static Type Pow(Type b, Type e) { if constexpr (IsSame<Type, F32>) { return powf(b, e); } else { return pow(b, e); } }

private:

	STATIC_CLASS(Math);
};

struct Vector2
{
public:
	Vector2();
	Vector2(F32 f);
	Vector2(F32 x, F32 y);
	Vector2(const Vector2& v);
	Vector2(Vector2&& v) noexcept;

	Vector2& operator=(F32 f);
	Vector2& operator=(const Vector2& v);
	Vector2& operator=(Vector2&& v) noexcept;

	Vector2& operator+=(F32 f);
	Vector2& operator-=(F32 f);
	Vector2& operator*=(F32 f);
	Vector2& operator/=(F32 f);
	Vector2& operator%=(F32 f);
	Vector2& operator+=(const Vector2& v);
	Vector2& operator-=(const Vector2& v);
	Vector2& operator*=(const Vector2& v);
	Vector2& operator/=(const Vector2& v);
	Vector2& operator%=(const Vector2& v);

	Vector2 operator+(F32 f) const;
	Vector2 operator-(F32 f) const;
	Vector2 operator*(F32 f) const;
	Vector2 operator/(F32 f) const;
	Vector2 operator%(F32 f) const;
	Vector2 operator+(const Vector2& v) const;
	Vector2 operator-(const Vector2& v) const;
	Vector2 operator*(const Vector2& v) const;
	Vector2 operator/(const Vector2& v) const;
	Vector2 operator%(const Vector2& v) const;

	bool operator==(const Vector2& v) const;
	bool operator!=(const Vector2& v) const;
	bool operator>(const Vector2& v) const;
	bool operator<(const Vector2& v) const;
	bool operator>=(const Vector2& v) const;
	bool operator<=(const Vector2& v) const;  
	bool IsZero() const;

	Vector2 operator-() const;
	Vector2 operator~() const;
	Vector2 operator!() const;

	F32 SqrMagnitude() const;
	F32 Magnitude() const;
	F32 Dot(const Vector2& v) const;
	Vector2& Normalize();
	Vector2 Normalized() const;
	F32 AngleBetween(const Vector2& v) const;
	Vector2 Projection(const Vector2& v) const;
	Vector2 OrthoProjection(const Vector2& v) const;
	F32 Cross(const Vector2& v) const;
	Vector2 Cross(const F32 f) const;
	Vector2 Normal(const Vector2& v) const;
	Vector2& Rotate(const Vector2& center, F32 angle);
	Vector2 Rotated(const Vector2& center, F32 angle) const;
	Vector2& Rotate(const Vector2& center, const Quaternion2& quat);
	Vector2 Rotated(const Vector2& center, const Quaternion2& quat) const;
	Vector2& Clamp(const Vector2& min, const Vector2& max);
	Vector2 Clamped(const Vector2& min, const Vector2& max) const;
	Vector2& SetClosest(const Vector2& min, const Vector2& max);
	Vector2 Closest(const Vector2& min, const Vector2& max) const;

	F32& operator[] (U64 i);
	const F32& operator[] (U64 i) const;
	F32* Data();
	const F32* Data() const;

	operator String() const;
	operator String16() const;
	operator String32() const;

public:
	F32 x, y;

	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 Left;
	static const Vector2 Right;
	static const Vector2 Up;
	static const Vector2 Down;
};

struct Vector3
{
public:
	Vector3();
	Vector3(F32 f);
	Vector3(F32 x, F32 y, F32 z);
	Vector3(const Vector3& v);
	Vector3(Vector3&& v) noexcept;

	Vector3& operator=(F32 f);
	Vector3& operator=(const Vector3& v);
	Vector3& operator=(Vector3&& v) noexcept;

	Vector3& operator+=(F32 f);
	Vector3& operator-=(F32 f);
	Vector3& operator*=(F32 f);
	Vector3& operator/=(F32 f);
	Vector3& operator%=(F32 f);
	Vector3& operator+=(const Vector3& v);
	Vector3& operator-=(const Vector3& v);
	Vector3& operator*=(const Vector3& v);
	Vector3& operator/=(const Vector3& v);
	Vector3& operator%=(const Vector3& v);

	Vector3 operator+(F32 f) const;
	Vector3 operator-(F32 f) const;
	Vector3 operator*(F32 f) const;
	Vector3 operator/(F32 f) const;
	Vector3 operator%(F32 f) const;
	Vector3 operator+(const Vector3& v) const;
	Vector3 operator-(const Vector3& v) const;
	Vector3 operator*(const Vector3& v) const;
	Vector3 operator/(const Vector3& v) const;
	Vector3 operator%(const Vector3& v) const;

	bool operator==(const Vector3& v) const;
	bool operator!=(const Vector3& v) const;
	bool operator>(const Vector3& v) const;
	bool operator<(const Vector3& v) const;
	bool operator>=(const Vector3& v) const;
	bool operator<=(const Vector3& v) const;
	bool IsZero() const;

	Vector3 operator-() const;
	Vector3 operator~() const;
	Vector3 operator!() const;

	F32 SqrMagnitude() const;
	F32 Magnitude() const;
	F32 Dot(const Vector3& v) const;
	Vector3& Normalize();
	Vector3 Normalized() const;
	Vector3 Projection(const Vector3& v) const;
	Vector3 OrthoProjection(const Vector3& v) const;
	Vector3 Cross(const Vector3& v) const;
	Vector3 Normal(const Vector3& v) const;
	Vector3& Rotate(const Vector3& center, const Quaternion3& quat);
	Vector3 Rotated(const Vector3& center, const Quaternion3& quat) const;
	Vector3& Clamp(const Vector3& xBound, const Vector3& yBound);
	Vector3 Clamped(const Vector3& xBound, const Vector3& yBound) const;
	Vector3& SetClosest(const Vector3& xBound, const Vector3& yBound);
	Vector3 Closest(const Vector3& xBound, const Vector3& yBound) const;

	F32& operator[] (U64 i);
	const F32& operator[] (U64 i) const;
	F32* Data();
	const F32* Data() const;

	operator String() const;
	operator String16() const;
	operator String32() const;

public:
	F32 x, y, z;

	static const Vector3 Zero;
	static const Vector3 One;
	static const Vector3 Left;
	static const Vector3 Right;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Forward;
	static const Vector3 Back;
};

struct Vector4
{
public:
	Vector4();
	Vector4(F32 f);
	Vector4(F32 x, F32 y, F32 z, F32 w);
	Vector4(const Vector4& v);
	Vector4(Vector4&& v) noexcept;

	Vector4& operator=(F32 f);
	Vector4& operator=(const Vector4& v);
	Vector4& operator=(Vector4&& v) noexcept;

	Vector4& operator+=(F32 f);
	Vector4& operator-=(F32 f);
	Vector4& operator*=(F32 f);
	Vector4& operator/=(F32 f);
	Vector4& operator%=(F32 f);
	Vector4& operator+=(const Vector4& v);
	Vector4& operator-=(const Vector4& v);
	Vector4& operator*=(const Vector4& v);
	Vector4& operator/=(const Vector4& v);
	Vector4& operator%=(const Vector4& v);

	Vector4 operator+(F32 f) const;
	Vector4 operator-(F32 f) const;
	Vector4 operator*(F32 f) const;
	Vector4 operator/(F32 f) const;
	Vector4 operator%(F32 f) const;
	Vector4 operator+(const Vector4& v) const;
	Vector4 operator-(const Vector4& v) const;
	Vector4 operator*(const Vector4& v) const;
	Vector4 operator/(const Vector4& v) const;
	Vector4 operator%(const Vector4& v) const;

	bool operator==(const Vector4& v) const;
	bool operator!=(const Vector4& v) const;
	bool operator>(const Vector4& v) const;
	bool operator<(const Vector4& v) const;
	bool operator>=(const Vector4& v) const;
	bool operator<=(const Vector4& v) const;
	bool IsZero() const;

	Vector4 operator-() const;
	Vector4 operator~() const;
	Vector4 operator!() const;

	F32 SqrMagnitude() const;
	F32 Magnitude() const;
	F32 Dot(const Vector4& v) const;
	Vector4& Normalize();
	Vector4 Normalized() const;
	Vector4 Projection(const Vector4& v) const;
	Vector4 OrthoProjection(const Vector4& v) const;
	Vector4 Cross(const Vector4& v) const;
	Vector4 Normal(const Vector4& v) const;
	Vector4& Rotate(const Vector4& center, const Quaternion3& quat);
	Vector4 Rotated(const Vector4& center, const Quaternion3& quat) const;
	Vector4& Clamp(const Vector4& xBound, const Vector4& yBound);
	Vector4 Clamped(const Vector4& xBound, const Vector4& yBound) const;
	Vector4& SetClosest(const Vector4& xBound, const Vector4& yBound);
	Vector4 Closest(const Vector4& xBound, const Vector4& yBound) const;

	F32& operator[] (U64 i);
	const F32& operator[] (U64 i) const;
	F32* Data();
	const F32* Data() const;

	operator String() const;
	operator String16() const;
	operator String32() const;

public:
	F32 x, y, z, w;

	static const Vector4 Zero;
	static const Vector4 One;
	static const Vector4 Left;
	static const Vector4 Right;
	static const Vector4 Up;
	static const Vector4 Down;
	static const Vector4 Forward;
	static const Vector4 Back;
	static const Vector4 In;
	static const Vector4 Out;
};

struct Vector2Int{};
struct Vector3Int{};
struct Vector4Int{};
struct Matrix2 {};
struct Matrix3 {};
struct Matrix4 {};
struct Quaternion2 {};
struct Quaternion3 {};