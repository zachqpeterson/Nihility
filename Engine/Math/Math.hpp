#pragma once

#include "MathDefines.hpp"

#include "Containers\String.hpp"
#include <math.h>

#undef far
#undef near

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

enum ProjectionType
{
	PROJECTION_TYPE_PERSPECTIVE,
	PROJECTION_TYPE_ORTHOGRAPHIC
};

class NH_API Math
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
	template <FloatingPoint Type> static Type Atan2(Type x, Type y) { if constexpr (IsSame<Type, F32>) { return atan2f(x, y); } else { return atan2(x, y); } }

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

	template <FloatingPoint Type> static bool Zero(Type f)
	{
		if constexpr (IsSame<Type, F32>) { return f < FLOAT_EPSILON&& f > -FLOAT_EPSILON; }
		else { return f < DOUBLE_EPSILON&& f > -DOUBLE_EPSILON; }
	}
	template <FloatingPoint Type> static bool NaN(Type f) { return isnan(f); }
	template <FloatingPoint Type> static bool Inf(Type f) { return isinf(f); }

	//INTERPOLATION
	template <FloatingPoint Type> static Type Lerp(Type a, Type b, Type t) { return a + (b - a) * t; }
	template <FloatingPoint Type> static Type InvLerp(Type a, Type b, Type t) { return (t - a) / (b - a); }
	template <VectorType Type> static Type Lerp(const Type& a, const Type& b, F32 t);
	template <FloatingPoint Type> static Type MoveTowards(Type a, Type b, Type t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }

	static Quaternion2 Slerp(const Quaternion2& a, const Quaternion2& b, F32 t);
	static Quaternion3 Slerp(const Quaternion3& a, const Quaternion3& b, F32 t);

	//EXPONENTS
	template <FloatingPoint Type> static Type Pow(Type b, Type e) { if constexpr (IsSame<Type, F32>) { return powf(b, e); } else { return pow(b, e); } }

private:

	STATIC_CLASS(Math);
};

struct NH_API Vector2
{
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
	Vector2& operator*=(const Quaternion2& q);

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
	Vector2 operator*(const Quaternion2& q) const;

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

struct NH_API Vector3
{
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
	Vector3& operator*=(const Quaternion3& q);

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
	Vector3 operator*(const Quaternion3& q) const;

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

struct NH_API Vector4
{
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

struct NH_API Vector2Int
{
	Vector2Int();
	Vector2Int(I32 i);
	Vector2Int(I32 x, I32 y);
	Vector2Int(const Vector2Int& v);
	Vector2Int(Vector2Int&& v) noexcept;

	Vector2Int& operator=(I32 i);
	Vector2Int& operator=(const Vector2Int& v);
	Vector2Int& operator=(Vector2Int&& v) noexcept;

	Vector2Int& operator+=(I32 i);
	Vector2Int& operator-=(I32 i);
	Vector2Int& operator*=(I32 i);
	Vector2Int& operator/=(I32 i);
	Vector2Int& operator%=(I32 i);
	Vector2Int& operator+=(const Vector2Int& v);
	Vector2Int& operator-=(const Vector2Int& v);
	Vector2Int& operator*=(const Vector2Int& v);
	Vector2Int& operator/=(const Vector2Int& v);
	Vector2Int& operator%=(const Vector2Int& v);

	Vector2Int operator+(I32 i) const;
	Vector2Int operator-(I32 i) const;
	Vector2Int operator*(I32 i) const;
	Vector2Int operator/(I32 i) const;
	Vector2Int operator%(I32 i) const;
	Vector2Int operator+(const Vector2Int& v) const;
	Vector2Int operator-(const Vector2Int& v) const;
	Vector2Int operator*(const Vector2Int& v) const;
	Vector2Int operator/(const Vector2Int& v) const;
	Vector2Int operator%(const Vector2Int& v) const;

	bool operator==(const Vector2Int& v) const;
	bool operator!=(const Vector2Int& v) const;
	bool operator>(const Vector2Int& v) const;
	bool operator<(const Vector2Int& v) const;
	bool operator>=(const Vector2Int& v) const;
	bool operator<=(const Vector2Int& v) const;
	bool IsZero() const;

	Vector2Int operator-() const;
	Vector2Int operator~() const;
	Vector2Int operator!() const;

	I32 SqrMagnitude() const;
	F32 Magnitude() const;
	I32 Dot(const Vector2Int& v) const;
	Vector2Int& Clamp(const Vector2Int& min, const Vector2Int& max);
	Vector2Int Clamped(const Vector2Int& min, const Vector2Int& max) const;
	Vector2Int& SetClosest(const Vector2Int& min, const Vector2Int& max);
	Vector2Int Closest(const Vector2Int& min, const Vector2Int& max) const;

	I32& operator[] (U64 i);
	const I32& operator[] (U64 i) const;
	I32* Data();
	const I32* Data() const;

	operator String() const;
	operator String16() const;
	operator String32() const;

public:
	I32 x, y;

	static const Vector2Int Zero;
	static const Vector2Int One;
	static const Vector2Int Left;
	static const Vector2Int Right;
	static const Vector2Int Up;
	static const Vector2Int Down;
};

struct NH_API Vector3Int
{
	Vector3Int();
	Vector3Int(I32 i);
	Vector3Int(I32 x, I32 y, I32 z);
	Vector3Int(const Vector3Int& v);
	Vector3Int(Vector3Int&& v) noexcept;

	Vector3Int& operator=(I32 i);
	Vector3Int& operator=(const Vector3Int& v);
	Vector3Int& operator=(Vector3Int&& v) noexcept;

	Vector3Int& operator+=(I32 i);
	Vector3Int& operator-=(I32 i);
	Vector3Int& operator*=(I32 i);
	Vector3Int& operator/=(I32 i);
	Vector3Int& operator%=(I32 i);
	Vector3Int& operator+=(const Vector3Int& v);
	Vector3Int& operator-=(const Vector3Int& v);
	Vector3Int& operator*=(const Vector3Int& v);
	Vector3Int& operator/=(const Vector3Int& v);
	Vector3Int& operator%=(const Vector3Int& v);

	Vector3Int operator+(I32 i) const;
	Vector3Int operator-(I32 i) const;
	Vector3Int operator*(I32 i) const;
	Vector3Int operator/(I32 i) const;
	Vector3Int operator%(I32 i) const;
	Vector3Int operator+(const Vector3Int& v) const;
	Vector3Int operator-(const Vector3Int& v) const;
	Vector3Int operator*(const Vector3Int& v) const;
	Vector3Int operator/(const Vector3Int& v) const;
	Vector3Int operator%(const Vector3Int& v) const;

	bool operator==(const Vector3Int& v) const;
	bool operator!=(const Vector3Int& v) const;
	bool operator>(const Vector3Int& v) const;
	bool operator<(const Vector3Int& v) const;
	bool operator>=(const Vector3Int& v) const;
	bool operator<=(const Vector3Int& v) const;
	bool IsZero() const;

	Vector3Int operator-() const;
	Vector3Int operator~() const;
	Vector3Int operator!() const;

	I32 SqrMagnitude() const;
	F32 Magnitude() const;
	I32 Dot(const Vector3Int& v) const;
	Vector3Int& Clamp(const Vector3Int& xBound, const Vector3Int& yBound);
	Vector3Int Clamped(const Vector3Int& xBound, const Vector3Int& yBound) const;
	Vector3Int& SetClosest(const Vector3Int& xBound, const Vector3Int& yBound);
	Vector3Int Closest(const Vector3Int& xBound, const Vector3Int& yBound) const;

	I32& operator[] (U64 i);
	const I32& operator[] (U64 i) const;
	I32* Data();
	const I32* Data() const;

	operator String() const;
	operator String16() const;
	operator String32() const;

public:
	I32 x, y, z;

	static const Vector3Int Zero;
	static const Vector3Int One;
	static const Vector3Int Left;
	static const Vector3Int Right;
	static const Vector3Int Up;
	static const Vector3Int Down;
	static const Vector3Int Forward;
	static const Vector3Int Back;
};

struct NH_API Vector4Int
{
	Vector4Int();
	Vector4Int(I32 i);
	Vector4Int(I32 x, I32 y, I32 z, I32 w);
	Vector4Int(const Vector4Int& v);
	Vector4Int(Vector4Int&& v) noexcept;

	Vector4Int& operator=(I32 i);
	Vector4Int& operator=(const Vector4Int& v);
	Vector4Int& operator=(Vector4Int&& v) noexcept;

	Vector4Int& operator+=(I32 i);
	Vector4Int& operator-=(I32 i);
	Vector4Int& operator*=(I32 i);
	Vector4Int& operator/=(I32 i);
	Vector4Int& operator%=(I32 i);
	Vector4Int& operator+=(const Vector4Int& v);
	Vector4Int& operator-=(const Vector4Int& v);
	Vector4Int& operator*=(const Vector4Int& v);
	Vector4Int& operator/=(const Vector4Int& v);
	Vector4Int& operator%=(const Vector4Int& v);

	Vector4Int operator+(I32 i) const;
	Vector4Int operator-(I32 i) const;
	Vector4Int operator*(I32 i) const;
	Vector4Int operator/(I32 i) const;
	Vector4Int operator%(I32 i) const;
	Vector4Int operator+(const Vector4Int& v) const;
	Vector4Int operator-(const Vector4Int& v) const;
	Vector4Int operator*(const Vector4Int& v) const;
	Vector4Int operator/(const Vector4Int& v) const;
	Vector4Int operator%(const Vector4Int& v) const;

	bool operator==(const Vector4Int& v) const;
	bool operator!=(const Vector4Int& v) const;
	bool operator>(const Vector4Int& v) const;
	bool operator<(const Vector4Int& v) const;
	bool operator>=(const Vector4Int& v) const;
	bool operator<=(const Vector4Int& v) const;
	bool IsZero() const;

	Vector4Int operator-() const;
	Vector4Int operator~() const;
	Vector4Int operator!() const;

	I32 SqrMagnitude() const;
	F32 Magnitude() const;
	I32 Dot(const Vector4Int& v) const;
	Vector4Int& Clamp(const Vector4Int& xBound, const Vector4Int& yBound);
	Vector4Int Clamped(const Vector4Int& xBound, const Vector4Int& yBound) const;
	Vector4Int& SetClosest(const Vector4Int& xBound, const Vector4Int& yBound);
	Vector4Int Closest(const Vector4Int& xBound, const Vector4Int& yBound) const;

	I32& operator[] (U64 i);
	const I32& operator[] (U64 i) const;
	I32* Data();
	const I32* Data() const;

	operator String() const;
	operator String16() const;
	operator String32() const;

public:
	I32 x, y, z, w;

	static const Vector4Int Zero;
	static const Vector4Int One;
	static const Vector4Int Left;
	static const Vector4Int Right;
	static const Vector4Int Up;
	static const Vector4Int Down;
	static const Vector4Int Forward;
	static const Vector4Int Back;
	static const Vector4Int In;
	static const Vector4Int Out;
};

template <VectorType Type>
inline Type Math::Lerp(const Type& a, const Type& b, F32 t)
{
	return a + (b - a) * t;
}

struct NH_API Matrix3
{
	Matrix3();
	Matrix3(F32 ax, F32 ay, F32 az, F32 bx, F32 by, F32 bz, F32 cx, F32 cy, F32 cz);
	Matrix3(const Vector3& a, const Vector3& b, const Vector3& c);
	Matrix3(Vector3&& v1, Vector3&& v2, Vector3&& v3) noexcept;
	Matrix3(const Matrix3& m);
	Matrix3(Matrix3&& m) noexcept;
	Matrix3(const Vector2& position, const F32& rotation, const Vector2& scale);
	Matrix3(const Vector2& position, const Quaternion2& rotation, const Vector2& scale);

	Matrix3& operator= (const Matrix3& m);
	Matrix3& operator= (Matrix3&& m) noexcept;

	void Set(const Vector2& position, const F32& rotation, const Vector2& scale);
	void Set(const Vector2& position, const Quaternion2& rotation, const Vector2& scale);

	Matrix3& operator+= (const Matrix3& m);
	Matrix3& operator-= (const Matrix3& m);
	Matrix3& operator*= (const Matrix3& m);

	Matrix3 operator+(const Matrix3& m) const;
	Matrix3 operator-(const Matrix3& m) const;
	Matrix3 operator*(const Matrix3& m) const;
	Vector3 operator*(const Vector3& v) const;

	Matrix3 operator-() const;
	Matrix3 operator~() const;
	Matrix3 operator!() const;
	bool operator==(const Matrix3& m) const;
	bool operator!=(const Matrix3& m) const;

	Vector3& operator[] (U8 i);
	const Vector3& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

public:
	Vector3 a, b, c; //Columns

	static const Matrix3 Identity;
};

struct NH_API Matrix4
{
	Matrix4();
	Matrix4(const Vector3& position, const Vector3& rotation = Vector3::Zero, const Vector3& scale = Vector3::One);
	Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale = Vector3::One);
	Matrix4(F32 ax, F32 ay, F32 az, F32 aw, F32 bx, F32 by, F32 bz, F32 bw, F32 cx, F32 cy, F32 cz, F32 cw, F32 dx, F32 dy, F32 dz, F32 dw);
	Matrix4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);
	Matrix4(Vector4&& a, Vector4&& b, Vector4&& c, Vector4&& d) noexcept;
	Matrix4(const Matrix4& m);
	Matrix4(const Matrix3& m);
	Matrix4(Matrix4&& m) noexcept;

	Matrix4& operator= (const Matrix4& m);
	Matrix4& operator= (Matrix4&& m) noexcept;

	void Set(const Vector3& position, const Vector3& rotation = Vector3::Zero, const Vector3& scale = Vector3::One);
	void Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale = Vector3::One);
	void SetPerspective(F32 fov, F32 aspect, F32 near, F32 far);
	void SetOrthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far);
	void LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);

	Matrix4& operator+= (const Matrix4& m);
	Matrix4& operator-= (const Matrix4& m);
	Matrix4& operator*= (const Matrix4& m);

	Matrix4 operator+(const Matrix4& m) const;
	Matrix4 operator-(const Matrix4& m) const;
	Matrix4 operator*(const Matrix4& m) const;
	Vector2 operator*(const Vector2& v) const;
	Vector3 operator*(const Vector3& v) const;
	Vector4 operator*(const Vector4& v) const;

	Matrix4 Inverse() const;
	Matrix4& Inversed();
	Matrix4 Invert() const;
	Matrix4& Inverted();
	Matrix4 Transpose() const;
	Matrix4& Transposed();

	//TODO: Move to Transform
	Vector3 Forward();
	Vector3 Back();
	Vector3 Right();
	Vector3 Left();
	Vector3 Up();
	Vector3 Down();

	Matrix4 operator-();
	Matrix4 operator~();
	Matrix4 operator!();
	bool operator==(const Matrix4& m) const;
	bool operator!=(const Matrix4& m) const;

	Vector4& operator[] (U8 i);
	const Vector4& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

public:
	Vector4 a, b, c, d; //Columns

	static const Matrix4 Identity;
};

struct NH_API Quaternion2
{
	Quaternion2();
	Quaternion2(F32 angle);
	Quaternion2(F32 x, F32 y);
	Quaternion2(const Quaternion2& q);
	Quaternion2(Quaternion2&& q) noexcept;

	Quaternion2& operator=(F32 angle);
	Quaternion2& operator=(const Quaternion2& q);
	Quaternion2& operator=(Quaternion2&& q) noexcept;

	Quaternion2& operator+=(const Quaternion2& q);
	Quaternion2& operator-=(const Quaternion2& q);
	Quaternion2& operator*=(const Quaternion2& q);
	Quaternion2& operator/=(const Quaternion2& q);

	Quaternion2 operator+(const Quaternion2& q) const;
	Quaternion2 operator-(const Quaternion2& q) const;
	Quaternion2 operator*(const Quaternion2& q) const;
	Quaternion2 operator/(const Quaternion2& q) const;

	void Set(F32 angle);
	void Rotate(F32 angle);

	F32 Angle() const;
	Quaternion2 Slerp(const Quaternion2& q, F32 t) const;
	Quaternion2& Slerped(const Quaternion2& q, F32 t);

	F32 Dot(const Quaternion2& q) const;
	F32 SqrNormal() const;
	F32 Normal() const;
	Quaternion2 Normalize() const;
	Quaternion2& Normalized();
	Quaternion2 Conjugate() const;
	Quaternion2& Conjugated();
	Quaternion2 Inverse() const;
	Quaternion2& Inversed();

	F32& operator[] (U8 i);
	const F32& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

public:
	F32 x, y; //sin, cos

	static const Quaternion2 Identity;
};

//TODO: Cache euler angles
struct NH_API Quaternion3
{
	Quaternion3();
	Quaternion3(F32 x, F32 y, F32 z, F32 w);
	Quaternion3(const Vector3& euler);
	Quaternion3(const Vector3& axis, F32 angle);
	Quaternion3(const Quaternion3& q);
	Quaternion3(Quaternion3&& q) noexcept;

	Quaternion3& operator=(const Vector3& euler);
	Quaternion3& operator=(const Quaternion3& q);
	Quaternion3& operator=(Quaternion3&& q) noexcept;

	Quaternion3& operator+=(const Quaternion3& q);
	Quaternion3& operator-=(const Quaternion3& q);
	Quaternion3& operator*=(const Quaternion3& q);
	Quaternion3& operator/=(const Quaternion3& q);

	Quaternion3 operator+(const Quaternion3& q) const;
	Quaternion3 operator-(const Quaternion3& q) const;
	Quaternion3 operator*(const Quaternion3& q) const;
	Quaternion3 operator/(const Quaternion3& q) const;

	Matrix3 ToMatrix3() const;
	Matrix4 ToMatrix4() const;
	Matrix4 RotationMatrix(Vector3 center) const;
	Vector3 Euler() const;
	Quaternion3 Slerp(const Quaternion3& q, F32 t) const;
	Quaternion3& Slerped(const Quaternion3& q, F32 t);

	F32 Dot(const Quaternion3& q) const;
	F32 SqrNormal() const;
	F32 Normal() const;
	Quaternion3 Normalize() const;
	Quaternion3& Normalized();
	Quaternion3 Conjugate() const;
	Quaternion3& Conjugated();
	Quaternion3 Inverse() const;
	Quaternion3& Inversed();

	F32& operator[] (U8 i);
	const F32& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

public:
	F32 x, y, z, w;

	static const Quaternion3 Identity;
};

struct HermiteSpline
{

};






inline const Vector2 Vector2::Zero{ 0.0f };
inline const Vector2 Vector2::One{ 1.0f };
inline const Vector2 Vector2::Left{ -1.0f, 0.0f };
inline const Vector2 Vector2::Right{ 1.0f, 0.0f };
inline const Vector2 Vector2::Up{ 0.0f, 1.0f };
inline const Vector2 Vector2::Down{ 0.0f, -1.0f };

inline const Vector3 Vector3::Zero{ 0.0f };
inline const Vector3 Vector3::One{ 1.0f };
inline const Vector3 Vector3::Left{ -1.0f, 0.0f, 0.0f };
inline const Vector3 Vector3::Right{ 1.0f, 0.0f, 0.0f };
inline const Vector3 Vector3::Up{ 0.0f, 1.0f, 0.0f };
inline const Vector3 Vector3::Down{ 0.0f, -1.0f, 0.0f };
inline const Vector3 Vector3::Forward{ 0.0f, 0.0f, 1.0f };
inline const Vector3 Vector3::Back{ 0.0f, 0.0f, -1.0f };

inline const Vector4 Vector4::Zero{ 0.0f };
inline const Vector4 Vector4::One{ 1.0f };
inline const Vector4 Vector4::Left{ -1.0f, 0.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::Right{ 1.0f, 0.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::Up{ 0.0f, 1.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::Down{ 0.0f, -1.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::Forward{ 0.0f, 0.0f, 1.0f, 0.0f };
inline const Vector4 Vector4::Back{ 0.0f, 0.0f, -1.0f, 0.0f };
inline const Vector4 Vector4::In{ 0.0f, 0.0f, 0.0f, 1.0f };
inline const Vector4 Vector4::Out{ 0.0f, 0.0f, 0.0f, -1.0f };

inline const Vector2Int Vector2Int::Zero{ 0 };
inline const Vector2Int Vector2Int::One{ 1 };
inline const Vector2Int Vector2Int::Left{ -1, 0 };
inline const Vector2Int Vector2Int::Right{ 1, 0 };
inline const Vector2Int Vector2Int::Up{ 0, 1 };
inline const Vector2Int Vector2Int::Down{ 0, -1 };

inline const Vector3Int Vector3Int::Zero{ 0 };
inline const Vector3Int Vector3Int::One{ 1 };
inline const Vector3Int Vector3Int::Left{ -1, 0, 0 };
inline const Vector3Int Vector3Int::Right{ 1, 0, 0 };
inline const Vector3Int Vector3Int::Up{ 0, 1, 0 };
inline const Vector3Int Vector3Int::Down{ 0, -1, 0 };
inline const Vector3Int Vector3Int::Forward{ 0, 0, 1 };
inline const Vector3Int Vector3Int::Back{ 0, 0, -1 };

inline const Vector4Int Vector4Int::Zero{ 0 };
inline const Vector4Int Vector4Int::One{ 1 };
inline const Vector4Int Vector4Int::Left{ -1, 0, 0, 0 };
inline const Vector4Int Vector4Int::Right{ 1, 0, 0, 0 };
inline const Vector4Int Vector4Int::Up{ 0, 1, 0, 0 };
inline const Vector4Int Vector4Int::Down{ 0, -1, 0, 0 };
inline const Vector4Int Vector4Int::Forward{ 0, 0, 1, 0 };
inline const Vector4Int Vector4Int::Back{ 0, 0, -1, 0 };
inline const Vector4Int Vector4Int::In{ 0, 0, 0, 1 };
inline const Vector4Int Vector4Int::Out{ 0, 0, 0, -1 };

inline const Matrix3 Matrix3::Identity{};
inline const Matrix4 Matrix4::Identity{};

inline const Quaternion2 Quaternion2::Identity{};
inline const Quaternion3 Quaternion3::Identity{};