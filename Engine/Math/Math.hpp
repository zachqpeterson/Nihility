#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Core/Time.hpp"

#include "gcem/gcem.hpp"

static constexpr inline F64 E = 2.718281828459045;
static constexpr inline F64 Pi = 3.141592653589793;
static constexpr inline F64 Phi = 1.618033988749894;
static constexpr inline F64 LogTwo = 0.693147180559945;
static constexpr inline F64 LogTen = 2.302585092994046;
static constexpr inline F64 TwoPi = Pi * 2.0;
static constexpr inline F64 HalfPi = Pi * 0.5;
static constexpr inline F64 QuarterPi = Pi * 0.25;
static constexpr inline F64 PiReciprocal = 1.0 / Pi;
static constexpr inline F64 TwoPiReciprocal = 1.0 / TwoPi;
static constexpr inline F64 SqrtTwo = 1.414213562373095;
static constexpr inline F64 SqrtTwoReciprocal = 1.0 / SqrtTwo;
static constexpr inline F64 DegToRad = Pi / 180.0;
static constexpr inline F64 RadToDeg = 180.0 / Pi;
static constexpr inline F64 OneThird = 1.0 / 3.0;
static constexpr inline F64 TwoThirds = 2.0 / 3.0;

//TODO: Wait for constexpr cmath to exist :((((((((((
class NH_API Math
{
public:
	template <class Type> static constexpr Type Min(const Type& a) noexcept { return a; }
	template <class Type> static constexpr Type Min(const Type& a, const Type& b) noexcept { return a > b ? b : a; }
	template <class Type, typename... Args> static constexpr Type Min(const Type& a, const Type& b, Args&&... args) noexcept { return a < b ? Min(a, args...) : Min(b, args...); }

	template <class Type> static constexpr Type Max(const Type& a) noexcept { return a; }
	template <class Type> static constexpr Type Max(const Type& a, const Type& b) noexcept { return a < b ? b : a; }
	template <class Type, typename... Args> static constexpr Type Max(const Type& a, const Type& b, Args&&... args) noexcept { return a > b ? Max(a, args...) : Max(b, args...); }

	template <class Type> static constexpr Type Abs(const Type& n) noexcept { return n < (Type)0 ? -n : n; }
	template <class Type> static constexpr Type Clamp(const Type& n, const Type& min, const Type& max) noexcept { return n < min ? min : n > max ? max : n; }
	template <class Type> static constexpr Type Sign(const Type& n) noexcept { return (Type)((n > (Type)0) - (n < (Type)0)); }
	template <class Type> static constexpr Type NonZeroSign(const Type& n) noexcept { return (Type)(2 * (n > (Type)0) - (Type)1); }
	template <FloatingPoint Type> static constexpr I64 Floor(const Type& n) noexcept { return n >= (Type)0 ? (I64)n : (I64)n - 1; }
	template <FloatingPoint Type> static constexpr Type FloorF(const Type& n) noexcept { return n > Traits<Type>::MaxPrecision ? n : n >= (Type)0 ? (Type)(I64)n : (Type)(I64)n - (Type)1; }
	template <FloatingPoint Type> static constexpr I64 Ceiling(const Type& n) noexcept { return (n - (I64)n) < (Type)0 ? (I64)n : (I64)n + 1; }
	template <FloatingPoint Type> static constexpr Type CeilingF(const Type& n) noexcept { return n > Traits<Type>::MaxPrecision ? n : (n - (I64)n) < (Type)0 ? (Type)(I64)n : (Type)(I64)n + (Type)1; }
	template <FloatingPoint Type> static constexpr Type Round(const Type& n) noexcept { return (Type)(I64)(n + 0.5); }
	template <FloatingPoint Type> static constexpr Type RoundI(const Type& n) noexcept { return (I64)(n + 0.5); }
	template <FloatingPoint Type> static constexpr Type Mod(const Type& n, const Type& d) noexcept { return n - d * FloorF(n / d); }
	template <FloatingPoint Type> static constexpr Type Closest(Type n, Type a, Type b) noexcept { return n < (b + a) * 0.5f ? a : b; }
	template <Integer Type> static constexpr Type Closest(Type n, Type a, Type b) noexcept { return n < (b + a) >> 1 ? a : b; }

	template <FloatingPoint Type> static constexpr Type DegreesToRadians(Type deg) noexcept { return deg * (F32)DegToRad; }
	template <FloatingPoint Type> static constexpr Type RadiansToDegrees(Type rad) noexcept { return rad * (F32)RadToDeg; }
	template <FloatingPoint Type> static constexpr Type NormalizeAngle(Type f) noexcept { return (Type)(f - (TwoPi * FloorF((f + Pi) / TwoPi))); }

	template <FloatingPoint Type> static constexpr bool IsZero(Type f) noexcept { return f < Traits<Type>::Epsilon && f > -Traits<Type>::Epsilon; }
	template <FloatingPoint Type> static constexpr bool IsNaN(Type f) noexcept { return f != f; }
	template <FloatingPoint Type> static constexpr bool IsInf(Type f) noexcept { return f == Traits<Type>::Infinity; }
	template <FloatingPoint Type> static constexpr bool IsNegInf(Type f) noexcept { return f == -Traits<Type>::Infinity; }
	template <FloatingPoint Type> static constexpr bool IsValid(Type f) noexcept { return !(IsNaN(f) || IsInf(f) || IsNegInf(f)); }

	template <FloatingPoint Type> static constexpr Type Remap(Type iMin, Type iMax, Type oMin, Type oMax, Type t) noexcept { return Lerp(oMin, oMax, InvLerp(iMin, iMax, t)); } //TODO: Vector for output
	template <FloatingPoint Type> static constexpr Type MoveTowards(Type a, Type b, Type t) noexcept { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }
	template <class Type, FloatingPoint FP> static constexpr Type Lerp(Type a, Type b, FP t) noexcept { return (Type)(a + (b - a) * t); }
	template <class Type, FloatingPoint FP> static constexpr Type InvLerp(Type a, Type b, FP t) noexcept { return (t - a) / (b - a); }
	template <class Type, FloatingPoint FP> static constexpr Type Mix(Type a, Type b, FP t) noexcept { return a * (FP(1) - t) + b * t; }
	template <class Type> static constexpr Type Tween(Type a, Type b) noexcept { return Lerp(a, b, 1.0 - Pow(0.01, Time::DeltaTime())); }

	template <class Type> static constexpr Type Sqrt(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::sqrt(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return sqrtf(n); }
			else { return sqrt(n); }
		}
	}
	template <class Type> static constexpr Type InvSqrt(Type n) noexcept
	{
		using FType = Conditional<IsFloatingPoint<Type>, Type, F64>;
		FType f = FType(n);

		return FType(1) / Sqrt(f);
	}

	template <class Base, class Degree> static constexpr CommonType<Base, Degree> Pow(Base base, Degree degree)
	{
		if (ConstantEvaluation())
		{
			return gcem::pow(base, degree);
		}
		else
		{
			using Type = CommonType<Base, Degree>;
			Type b = Type(base);
			Type d = Type(degree);

			if constexpr (IsSame<Type, F32>) { return powf(b, d); }
			else { return pow(b, d); }
		}
	}

	template <class Type> static constexpr Type Exp(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::exp(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return expf(n); }
			else { return exp(n); }
		}
	}

	template <class Type> static constexpr Type Log(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::log(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return logf(n); }
			else { return log(n); }
		}
	}

	template <class Type> static constexpr Type Log2(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::log2(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return log2f(n); }
			else { return log2(n); }
		}
	}

	template <class Type> static constexpr Type Log10(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::log10(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return log10f(n); }
			else { return log10(n); }
		}
	}

	template <class Type> static constexpr Type Cos(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::cos(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return cosf(n); }
			else { return cos(n); }
		}
	}

	template <class Type> static constexpr Type Sin(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::sin(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return sinf(n); }
			else { return sin(n); }
		}
	}

	template <class Type> static constexpr Type Tan(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::tan(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return tanf(n); }
			else { return tan(n); }
		}
	}

	template <class Type> static constexpr Type ACos(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::acos(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return acosf(n); }
			else { return acos(n); }
		}
	}

	template <class Type> static constexpr Type ASin(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::asin(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return asinf(n); }
			else { return asin(n); }
		}
	}

	template <class Type> static constexpr Type ATan(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::atan(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return atanf(n); }
			else { return atan(n); }
		}
	}

	template <class Type> static constexpr Type ATan2(Type n1, Type n2)
	{
		if (ConstantEvaluation())
		{
			return gcem::atan2(n1, n2);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return atan2f(n1, n2); }
			else { return atan2(n1, n2); }
		}
	}

	template <class Type> static constexpr Type CosH(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::cosh(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return coshf(n); }
			else { return cosh(n); }
		}
	}

	template <class Type> static constexpr Type SinH(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::sinh(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return sinhf(n); }
			else { return sinh(n); }
		}
	}

	template <class Type> static constexpr Type TanH(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::tanh(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return tanhf(n); }

			else { return tanh(n); }
		}
	}

	template <class Type> static constexpr Type ACosH(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::acosh(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return acoshf(n); }
			else { return acosh(n); }
		}
	}

	template <class Type> static constexpr Type ASinH(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::asinh(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return asinhf(n); }
			else { return asinh(n); }
		}
	}

	template <class Type> static constexpr Type ATanH(Type n)
	{
		if (ConstantEvaluation())
		{
			return gcem::atanh(n);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return atanhf(n); }

			else { return atanh(n); }
		}
	}

private:

	STATIC_CLASS(Math);
};

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

struct NH_API Vector2
{
	constexpr Vector2();
	constexpr Vector2(F32 f);
	constexpr Vector2(F32 x, F32 y);
	constexpr Vector2(const Vector2& v);
	constexpr Vector2(Vector2&& v) noexcept;

	constexpr Vector2& operator=(F32 f);
	constexpr Vector2& operator=(const Vector2& v);
	constexpr Vector2& operator=(Vector2&& v) noexcept;

	constexpr Vector2& operator+=(F32 f);
	constexpr Vector2& operator-=(F32 f);
	constexpr Vector2& operator*=(F32 f);
	constexpr Vector2& operator/=(F32 f);
	constexpr Vector2& operator%=(F32 f);
	constexpr Vector2& operator+=(const Vector2& v);
	constexpr Vector2& operator-=(const Vector2& v);
	constexpr Vector2& operator*=(const Vector2& v);
	constexpr Vector2& operator/=(const Vector2& v);
	constexpr Vector2& operator%=(const Vector2& v);
	constexpr Vector2& operator*=(const Quaternion2& q);
	constexpr Vector2& operator^=(const Quaternion2& q);

	constexpr Vector2 operator+(F32 f) const;
	constexpr Vector2 operator-(F32 f) const;
	constexpr Vector2 operator*(F32 f) const;
	constexpr Vector2 operator/(F32 f) const;
	constexpr Vector2 operator%(F32 f) const;
	constexpr Vector2 operator+(const Vector2& v) const;
	constexpr Vector2 operator-(const Vector2& v) const;
	constexpr Vector2 operator*(const Vector2& v) const;
	constexpr Vector2 operator/(const Vector2& v) const;
	constexpr Vector2 operator%(const Vector2& v) const;
	constexpr Vector2 operator*(const Quaternion2& q) const;
	constexpr Vector2 operator^(const Quaternion2& q) const;

	constexpr bool operator==(const Vector2& v) const;
	constexpr bool operator!=(const Vector2& v) const;
	constexpr bool operator>(const Vector2& v) const;
	constexpr bool operator<(const Vector2& v) const;
	constexpr bool operator>=(const Vector2& v) const;
	constexpr bool operator<=(const Vector2& v) const;
	constexpr bool IsZero() const;

	constexpr Vector2 operator-() const;
	constexpr Vector2 operator~() const;
	constexpr Vector2 operator!() const;

	constexpr F32 SqrMagnitude() const;
	constexpr F32 Magnitude() const;
	constexpr F32 Dot(const Vector2& v) const;
	constexpr F32 Dot(F32 vx, F32 vy) const;
	constexpr Vector2& Normalize();
	constexpr Vector2 Normalized() const;
	constexpr Vector2& Normalize(F32& magnitude);
	constexpr Vector2 Normalized(F32& magnitude);
	constexpr F32 AngleBetween(const Vector2& v) const;
	constexpr Vector2 Projection(const Vector2& v) const;
	constexpr Vector2 OrthoProjection(const Vector2& v) const;
	constexpr F32 Cross(const Vector2& v) const;
	constexpr Vector2 Cross(F32 f) const;
	constexpr Vector2 CrossInv(F32 f) const;
	constexpr Vector2 PerpendicularLeft() const;
	constexpr Vector2 PerpendicularRight() const;
	constexpr Vector2 Normal(const Vector2& v) const;
	constexpr Vector2& Rotate(const Vector2& center, F32 angle);
	constexpr Vector2 Rotated(const Vector2& center, F32 angle) const;
	constexpr Vector2& Rotate(F32 angle);
	constexpr Vector2 Rotated(F32 angle) const;
	constexpr Vector2& Rotate(const Vector2& center, const Quaternion2& quat);
	constexpr Vector2 Rotated(const Vector2& center, const Quaternion2& quat) const;
	constexpr Vector2& Rotate(const Quaternion2& quat);
	constexpr Vector2 Rotated(const Quaternion2& quat) const;
	constexpr Vector2& Clamp(const Vector2& min, const Vector2& max);
	constexpr Vector2 Clamped(const Vector2& min, const Vector2& max) const;
	constexpr Vector2& SetClosest(const Vector2& min, const Vector2& max);
	constexpr Vector2 Closest(const Vector2& min, const Vector2& max) const;

	constexpr Vector2 Min(const Vector2& other);
	constexpr Vector2 Max(const Vector2& other);

	constexpr bool Valid();

	F32& operator[] (U64 i);
	const F32& operator[] (U64 i) const;

	F32* Data();
	const F32* Data() const;

	constexpr Vector2 xx() const;
	constexpr Vector2 xy() const;
	constexpr Vector2 yx() const;
	constexpr Vector2 yy() const;

	constexpr explicit operator Vector3() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector3Int() const;
	constexpr explicit operator Vector4Int() const;

	//TODO:
	//operator String8() const;
	//operator String16() const;
	//operator String32() const;

public:
	F32 x, y;

	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 Up;
	static const Vector2 Down;
	static const Vector2 Left;
	static const Vector2 Right;
};

struct NH_API Vector3
{
	constexpr Vector3();
	constexpr Vector3(F32 f);
	constexpr Vector3(F32 x, F32 y, F32 z);
	constexpr Vector3(const Vector2& v);
	constexpr Vector3(const Vector2& v, F32 z);
	constexpr Vector3(F32 x, const Vector2& v);
	constexpr Vector3(const Vector3& v);
	constexpr Vector3(Vector3&& v) noexcept;

	constexpr Vector3& operator=(F32 f);
	constexpr Vector3& operator=(const Vector3& v);
	constexpr Vector3& operator=(Vector3&& v) noexcept;

	constexpr Vector3& operator+=(F32 f);
	constexpr Vector3& operator-=(F32 f);
	constexpr Vector3& operator*=(F32 f);
	constexpr Vector3& operator/=(F32 f);
	constexpr Vector3& operator%=(F32 f);
	constexpr Vector3& operator+=(const Vector3& v);
	constexpr Vector3& operator-=(const Vector3& v);
	constexpr Vector3& operator*=(const Vector3& v);
	constexpr Vector3& operator/=(const Vector3& v);
	constexpr Vector3& operator%=(const Vector3& v);
	constexpr Vector3& operator*=(const Quaternion3& q);
	constexpr Vector3& operator^=(const Quaternion3& q);

	constexpr Vector3 operator+(F32 f) const;
	constexpr Vector3 operator-(F32 f) const;
	constexpr Vector3 operator*(F32 f) const;
	constexpr Vector3 operator/(F32 f) const;
	constexpr Vector3 operator%(F32 f) const;
	constexpr Vector3 operator+(const Vector3& v) const;
	constexpr Vector3 operator-(const Vector3& v) const;
	constexpr Vector3 operator*(const Vector3& v) const;
	constexpr Vector3 operator/(const Vector3& v) const;
	constexpr Vector3 operator%(const Vector3& v) const;
	constexpr Vector3 operator*(const Quaternion3& q) const;
	constexpr Vector3 operator^(const Quaternion3& q) const;

	constexpr bool operator==(const Vector3& v) const;
	constexpr bool operator!=(const Vector3& v) const;
	constexpr bool operator>(const Vector3& v) const;
	constexpr bool operator<(const Vector3& v) const;
	constexpr bool operator>=(const Vector3& v) const;
	constexpr bool operator<=(const Vector3& v) const;
	constexpr bool IsZero() const;

	constexpr Vector3 operator-() const;
	constexpr Vector3 operator~() const;
	constexpr Vector3 operator!() const;

	constexpr F32 SqrMagnitude() const;
	constexpr F32 Magnitude() const;
	constexpr F32 Dot() const;
	constexpr F32 Dot(const Vector3& v) const;
	constexpr F32 Dot(F32 vx, F32 vy, F32 vz) const;
	constexpr Vector3& Normalize();
	constexpr Vector3 Normalized() const;
	constexpr Vector3& Normalize(F32& magnitude);
	constexpr Vector3 Normalized(F32& magnitude);
	constexpr F32 AngleBetween(const Vector3& v) const;
	constexpr Vector3 Projection(const Vector3& v) const;
	constexpr Vector3 OrthoProjection(const Vector3& v) const;
	constexpr Vector3 Cross(const Vector3& v) const;
	constexpr Vector3 Cross(F32 f) const;
	constexpr Vector3 CrossInv(F32 f) const;
	constexpr Vector3 PerpendicularLeft() const;
	constexpr Vector3 PerpendicularRight() const;
	constexpr Vector3 Normal(const Vector3& v) const;
	constexpr Vector3& Rotate(const Vector3& center, const Quaternion3& quat);
	constexpr Vector3 Rotated(const Vector3& center, const Quaternion3& quat) const;
	constexpr Vector3& Rotate(const Quaternion3& quat);
	constexpr Vector3 Rotated(const Quaternion3& quat) const;
	constexpr Vector3& Clamp(const Vector3& min, const Vector3& max);
	constexpr Vector3 Clamped(const Vector3& min, const Vector3& max) const;
	constexpr Vector3& SetClosest(const Vector3& min, const Vector3& max);
	constexpr Vector3 Closest(const Vector3& min, const Vector3& max) const;

	constexpr Vector3 Min(const Vector3& other);
	constexpr Vector3 Max(const Vector3& other);

	constexpr bool Valid();

	F32& operator[] (U64 i);
	const F32& operator[] (U64 i) const;

	F32* Data();
	const F32* Data() const;

	constexpr Vector2 xx() const;
	constexpr Vector2 xy() const;
	constexpr Vector2 xz() const;
	constexpr Vector2 yx() const;
	constexpr Vector2 yy() const;
	constexpr Vector2 yz() const;
	constexpr Vector2 zx() const;
	constexpr Vector2 zy() const;
	constexpr Vector2 zz() const;
	constexpr Vector3 xxx() const;
	constexpr Vector3 xxy() const;
	constexpr Vector3 xxz() const;
	constexpr Vector3 xyx() const;
	constexpr Vector3 xyy() const;
	constexpr Vector3 xyz() const;
	constexpr Vector3 xzx() const;
	constexpr Vector3 xzy() const;
	constexpr Vector3 xzz() const;
	constexpr Vector3 yxx() const;
	constexpr Vector3 yxy() const;
	constexpr Vector3 yxz() const;
	constexpr Vector3 yyx() const;
	constexpr Vector3 yyy() const;
	constexpr Vector3 yyz() const;
	constexpr Vector3 yzx() const;
	constexpr Vector3 yzy() const;
	constexpr Vector3 yzz() const;
	constexpr Vector3 zxx() const;
	constexpr Vector3 zxy() const;
	constexpr Vector3 zxz() const;
	constexpr Vector3 zyx() const;
	constexpr Vector3 zyy() const;
	constexpr Vector3 zyz() const;
	constexpr Vector3 zzx() const;
	constexpr Vector3 zzy() const;
	constexpr Vector3 zzz() const;

	constexpr explicit operator Vector2() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector3Int() const;
	constexpr explicit operator Vector4Int() const;

	//TODO:
	//operator String8() const;
	//operator String16() const;
	//operator String32() const;

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
	constexpr Vector4();
	constexpr Vector4(F32 f);
	constexpr Vector4(F32 x, F32 y, F32 z, F32 w);
	constexpr Vector4(const Vector2& a, const Vector2& b);
	constexpr Vector4(F32 x, const Vector2& a, F32 w);
	constexpr Vector4(const Vector4& v);
	constexpr Vector4(Vector4&& v) noexcept;

	constexpr Vector4& operator=(F32 f);
	constexpr Vector4& operator=(const Vector4& v);
	constexpr Vector4& operator=(Vector4&& v) noexcept;

	constexpr Vector4& operator+=(F32 f);
	constexpr Vector4& operator-=(F32 f);
	constexpr Vector4& operator*=(F32 f);
	constexpr Vector4& operator/=(F32 f);
	constexpr Vector4& operator%=(F32 f);
	constexpr Vector4& operator+=(const Vector4& v);
	constexpr Vector4& operator-=(const Vector4& v);
	constexpr Vector4& operator*=(const Vector4& v);
	constexpr Vector4& operator/=(const Vector4& v);
	constexpr Vector4& operator%=(const Vector4& v);

	constexpr Vector4 operator+(F32 f) const;
	constexpr Vector4 operator-(F32 f) const;
	constexpr Vector4 operator*(F32 f) const;
	constexpr Vector4 operator/(F32 f) const;
	constexpr Vector4 operator%(F32 f) const;
	constexpr Vector4 operator+(const Vector4& v) const;
	constexpr Vector4 operator-(const Vector4& v) const;
	constexpr Vector4 operator*(const Vector4& v) const;
	constexpr Vector4 operator/(const Vector4& v) const;
	constexpr Vector4 operator%(const Vector4& v) const;

	constexpr bool operator==(const Vector4& v) const;
	constexpr bool operator!=(const Vector4& v) const;
	constexpr bool operator>(const Vector4& v) const;
	constexpr bool operator<(const Vector4& v) const;
	constexpr bool operator>=(const Vector4& v) const;
	constexpr bool operator<=(const Vector4& v) const;
	constexpr bool IsZero() const;

	constexpr Vector4 operator-() const;
	constexpr Vector4 operator~() const;
	constexpr Vector4 operator!() const;

	constexpr F32 SqrMagnitude() const;
	constexpr F32 Magnitude() const;
	constexpr F32 Dot(const Vector4& v) const;
	constexpr F32 Dot(F32 vx, F32 vy, F32 vz, F32 vw) const;
	constexpr Vector4& Normalize();
	constexpr Vector4 Normalized() const;
	constexpr Vector4 Projection(const Vector4& v) const;
	constexpr Vector4 OrthoProjection(const Vector4& v) const;
	constexpr Vector4& Clamp(const Vector4& min, const Vector4& max);
	constexpr Vector4 Clamped(const Vector4& min, const Vector4& max) const;
	constexpr Vector4& SetClosest(const Vector4& min, const Vector4& max);
	constexpr Vector4 Closest(const Vector4& min, const Vector4& max) const;

	constexpr Vector4 Min(const Vector4& other);
	constexpr Vector4 Max(const Vector4& other);

	constexpr bool Valid();

	F32& operator[] (U64 i);
	const F32& operator[] (U64 i) const;

	F32* Data();
	const F32* Data() const;

#pragma region SwizzleFunctions
	constexpr Vector2 xx() const;
	constexpr Vector2 xy() const;
	constexpr Vector2 xz() const;
	constexpr Vector2 xw() const;
	constexpr Vector2 yx() const;
	constexpr Vector2 yy() const;
	constexpr Vector2 yz() const;
	constexpr Vector2 yw() const;
	constexpr Vector2 zx() const;
	constexpr Vector2 zy() const;
	constexpr Vector2 zz() const;
	constexpr Vector2 zw() const;
	constexpr Vector2 wx() const;
	constexpr Vector2 wy() const;
	constexpr Vector2 wz() const;
	constexpr Vector2 ww() const;
	constexpr Vector3 xxx() const;
	constexpr Vector3 xxy() const;
	constexpr Vector3 xxz() const;
	constexpr Vector3 xxw() const;
	constexpr Vector3 xyx() const;
	constexpr Vector3 xyy() const;
	constexpr Vector3 xyz() const;
	constexpr Vector3 xyw() const;
	constexpr Vector3 xzx() const;
	constexpr Vector3 xzy() const;
	constexpr Vector3 xzz() const;
	constexpr Vector3 xzw() const;
	constexpr Vector3 xwx() const;
	constexpr Vector3 xwy() const;
	constexpr Vector3 xwz() const;
	constexpr Vector3 xww() const;
	constexpr Vector3 yxx() const;
	constexpr Vector3 yxy() const;
	constexpr Vector3 yxz() const;
	constexpr Vector3 yxw() const;
	constexpr Vector3 yyx() const;
	constexpr Vector3 yyy() const;
	constexpr Vector3 yyz() const;
	constexpr Vector3 yyw() const;
	constexpr Vector3 yzx() const;
	constexpr Vector3 yzy() const;
	constexpr Vector3 yzz() const;
	constexpr Vector3 yzw() const;
	constexpr Vector3 ywx() const;
	constexpr Vector3 ywy() const;
	constexpr Vector3 ywz() const;
	constexpr Vector3 yww() const;
	constexpr Vector3 zxx() const;
	constexpr Vector3 zxy() const;
	constexpr Vector3 zxz() const;
	constexpr Vector3 zxw() const;
	constexpr Vector3 zyx() const;
	constexpr Vector3 zyy() const;
	constexpr Vector3 zyz() const;
	constexpr Vector3 zyw() const;
	constexpr Vector3 zzx() const;
	constexpr Vector3 zzy() const;
	constexpr Vector3 zzz() const;
	constexpr Vector3 zzw() const;
	constexpr Vector3 zwx() const;
	constexpr Vector3 zwy() const;
	constexpr Vector3 zwz() const;
	constexpr Vector3 zww() const;
	constexpr Vector3 wxx() const;
	constexpr Vector3 wxy() const;
	constexpr Vector3 wxz() const;
	constexpr Vector3 wxw() const;
	constexpr Vector3 wyx() const;
	constexpr Vector3 wyy() const;
	constexpr Vector3 wyz() const;
	constexpr Vector3 wyw() const;
	constexpr Vector3 wzx() const;
	constexpr Vector3 wzy() const;
	constexpr Vector3 wzz() const;
	constexpr Vector3 wzw() const;
	constexpr Vector3 wwx() const;
	constexpr Vector3 wwy() const;
	constexpr Vector3 wwz() const;
	constexpr Vector3 www() const;
	constexpr Vector4 xxxx() const;
	constexpr Vector4 xxxy() const;
	constexpr Vector4 xxxz() const;
	constexpr Vector4 xxxw() const;
	constexpr Vector4 xxyx() const;
	constexpr Vector4 xxyy() const;
	constexpr Vector4 xxyz() const;
	constexpr Vector4 xxyw() const;
	constexpr Vector4 xxzx() const;
	constexpr Vector4 xxzy() const;
	constexpr Vector4 xxzz() const;
	constexpr Vector4 xxzw() const;
	constexpr Vector4 xxwx() const;
	constexpr Vector4 xxwy() const;
	constexpr Vector4 xxwz() const;
	constexpr Vector4 xxww() const;
	constexpr Vector4 xyxx() const;
	constexpr Vector4 xyxy() const;
	constexpr Vector4 xyxz() const;
	constexpr Vector4 xyxw() const;
	constexpr Vector4 xyyx() const;
	constexpr Vector4 xyyy() const;
	constexpr Vector4 xyyz() const;
	constexpr Vector4 xyyw() const;
	constexpr Vector4 xyzx() const;
	constexpr Vector4 xyzy() const;
	constexpr Vector4 xyzz() const;
	constexpr Vector4 xyzw() const;
	constexpr Vector4 xywx() const;
	constexpr Vector4 xywy() const;
	constexpr Vector4 xywz() const;
	constexpr Vector4 xyww() const;
	constexpr Vector4 xzxx() const;
	constexpr Vector4 xzxy() const;
	constexpr Vector4 xzxz() const;
	constexpr Vector4 xzxw() const;
	constexpr Vector4 xzyx() const;
	constexpr Vector4 xzyy() const;
	constexpr Vector4 xzyz() const;
	constexpr Vector4 xzyw() const;
	constexpr Vector4 xzzx() const;
	constexpr Vector4 xzzy() const;
	constexpr Vector4 xzzz() const;
	constexpr Vector4 xzzw() const;
	constexpr Vector4 xzwx() const;
	constexpr Vector4 xzwy() const;
	constexpr Vector4 xzwz() const;
	constexpr Vector4 xzww() const;
	constexpr Vector4 xwxx() const;
	constexpr Vector4 xwxy() const;
	constexpr Vector4 xwxz() const;
	constexpr Vector4 xwxw() const;
	constexpr Vector4 xwyx() const;
	constexpr Vector4 xwyy() const;
	constexpr Vector4 xwyz() const;
	constexpr Vector4 xwyw() const;
	constexpr Vector4 xwzx() const;
	constexpr Vector4 xwzy() const;
	constexpr Vector4 xwzz() const;
	constexpr Vector4 xwzw() const;
	constexpr Vector4 xwwx() const;
	constexpr Vector4 xwwy() const;
	constexpr Vector4 xwwz() const;
	constexpr Vector4 xwww() const;
	constexpr Vector4 yxxx() const;
	constexpr Vector4 yxxy() const;
	constexpr Vector4 yxxz() const;
	constexpr Vector4 yxxw() const;
	constexpr Vector4 yxyx() const;
	constexpr Vector4 yxyy() const;
	constexpr Vector4 yxyz() const;
	constexpr Vector4 yxyw() const;
	constexpr Vector4 yxzx() const;
	constexpr Vector4 yxzy() const;
	constexpr Vector4 yxzz() const;
	constexpr Vector4 yxzw() const;
	constexpr Vector4 yxwx() const;
	constexpr Vector4 yxwy() const;
	constexpr Vector4 yxwz() const;
	constexpr Vector4 yxww() const;
	constexpr Vector4 yyxx() const;
	constexpr Vector4 yyxy() const;
	constexpr Vector4 yyxz() const;
	constexpr Vector4 yyxw() const;
	constexpr Vector4 yyyx() const;
	constexpr Vector4 yyyy() const;
	constexpr Vector4 yyyz() const;
	constexpr Vector4 yyyw() const;
	constexpr Vector4 yyzx() const;
	constexpr Vector4 yyzy() const;
	constexpr Vector4 yyzz() const;
	constexpr Vector4 yyzw() const;
	constexpr Vector4 yywx() const;
	constexpr Vector4 yywy() const;
	constexpr Vector4 yywz() const;
	constexpr Vector4 yyww() const;
	constexpr Vector4 yzxx() const;
	constexpr Vector4 yzxy() const;
	constexpr Vector4 yzxz() const;
	constexpr Vector4 yzxw() const;
	constexpr Vector4 yzyx() const;
	constexpr Vector4 yzyy() const;
	constexpr Vector4 yzyz() const;
	constexpr Vector4 yzyw() const;
	constexpr Vector4 yzzx() const;
	constexpr Vector4 yzzy() const;
	constexpr Vector4 yzzz() const;
	constexpr Vector4 yzzw() const;
	constexpr Vector4 yzwx() const;
	constexpr Vector4 yzwy() const;
	constexpr Vector4 yzwz() const;
	constexpr Vector4 yzww() const;
	constexpr Vector4 ywxx() const;
	constexpr Vector4 ywxy() const;
	constexpr Vector4 ywxz() const;
	constexpr Vector4 ywxw() const;
	constexpr Vector4 ywyx() const;
	constexpr Vector4 ywyy() const;
	constexpr Vector4 ywyz() const;
	constexpr Vector4 ywyw() const;
	constexpr Vector4 ywzx() const;
	constexpr Vector4 ywzy() const;
	constexpr Vector4 ywzz() const;
	constexpr Vector4 ywzw() const;
	constexpr Vector4 ywwx() const;
	constexpr Vector4 ywwy() const;
	constexpr Vector4 ywwz() const;
	constexpr Vector4 ywww() const;
	constexpr Vector4 zxxx() const;
	constexpr Vector4 zxxy() const;
	constexpr Vector4 zxxz() const;
	constexpr Vector4 zxxw() const;
	constexpr Vector4 zxyx() const;
	constexpr Vector4 zxyy() const;
	constexpr Vector4 zxyz() const;
	constexpr Vector4 zxyw() const;
	constexpr Vector4 zxzx() const;
	constexpr Vector4 zxzy() const;
	constexpr Vector4 zxzz() const;
	constexpr Vector4 zxzw() const;
	constexpr Vector4 zxwx() const;
	constexpr Vector4 zxwy() const;
	constexpr Vector4 zxwz() const;
	constexpr Vector4 zxww() const;
	constexpr Vector4 zyxx() const;
	constexpr Vector4 zyxy() const;
	constexpr Vector4 zyxz() const;
	constexpr Vector4 zyxw() const;
	constexpr Vector4 zyyx() const;
	constexpr Vector4 zyyy() const;
	constexpr Vector4 zyyz() const;
	constexpr Vector4 zyyw() const;
	constexpr Vector4 zyzx() const;
	constexpr Vector4 zyzy() const;
	constexpr Vector4 zyzz() const;
	constexpr Vector4 zyzw() const;
	constexpr Vector4 zywx() const;
	constexpr Vector4 zywy() const;
	constexpr Vector4 zywz() const;
	constexpr Vector4 zyww() const;
	constexpr Vector4 zzxx() const;
	constexpr Vector4 zzxy() const;
	constexpr Vector4 zzxz() const;
	constexpr Vector4 zzxw() const;
	constexpr Vector4 zzyx() const;
	constexpr Vector4 zzyy() const;
	constexpr Vector4 zzyz() const;
	constexpr Vector4 zzyw() const;
	constexpr Vector4 zzzx() const;
	constexpr Vector4 zzzy() const;
	constexpr Vector4 zzzz() const;
	constexpr Vector4 zzzw() const;
	constexpr Vector4 zzwx() const;
	constexpr Vector4 zzwy() const;
	constexpr Vector4 zzwz() const;
	constexpr Vector4 zzww() const;
	constexpr Vector4 zwxx() const;
	constexpr Vector4 zwxy() const;
	constexpr Vector4 zwxz() const;
	constexpr Vector4 zwxw() const;
	constexpr Vector4 zwyx() const;
	constexpr Vector4 zwyy() const;
	constexpr Vector4 zwyz() const;
	constexpr Vector4 zwyw() const;
	constexpr Vector4 zwzx() const;
	constexpr Vector4 zwzy() const;
	constexpr Vector4 zwzz() const;
	constexpr Vector4 zwzw() const;
	constexpr Vector4 zwwx() const;
	constexpr Vector4 zwwy() const;
	constexpr Vector4 zwwz() const;
	constexpr Vector4 zwww() const;
	constexpr Vector4 wxxx() const;
	constexpr Vector4 wxxy() const;
	constexpr Vector4 wxxz() const;
	constexpr Vector4 wxxw() const;
	constexpr Vector4 wxyx() const;
	constexpr Vector4 wxyy() const;
	constexpr Vector4 wxyz() const;
	constexpr Vector4 wxyw() const;
	constexpr Vector4 wxzx() const;
	constexpr Vector4 wxzy() const;
	constexpr Vector4 wxzz() const;
	constexpr Vector4 wxzw() const;
	constexpr Vector4 wxwx() const;
	constexpr Vector4 wxwy() const;
	constexpr Vector4 wxwz() const;
	constexpr Vector4 wxww() const;
	constexpr Vector4 wyxx() const;
	constexpr Vector4 wyxy() const;
	constexpr Vector4 wyxz() const;
	constexpr Vector4 wyxw() const;
	constexpr Vector4 wyyx() const;
	constexpr Vector4 wyyy() const;
	constexpr Vector4 wyyz() const;
	constexpr Vector4 wyyw() const;
	constexpr Vector4 wyzx() const;
	constexpr Vector4 wyzy() const;
	constexpr Vector4 wyzz() const;
	constexpr Vector4 wyzw() const;
	constexpr Vector4 wywx() const;
	constexpr Vector4 wywy() const;
	constexpr Vector4 wywz() const;
	constexpr Vector4 wyww() const;
	constexpr Vector4 wzxx() const;
	constexpr Vector4 wzxy() const;
	constexpr Vector4 wzxz() const;
	constexpr Vector4 wzxw() const;
	constexpr Vector4 wzyx() const;
	constexpr Vector4 wzyy() const;
	constexpr Vector4 wzyz() const;
	constexpr Vector4 wzyw() const;
	constexpr Vector4 wzzx() const;
	constexpr Vector4 wzzy() const;
	constexpr Vector4 wzzz() const;
	constexpr Vector4 wzzw() const;
	constexpr Vector4 wzwx() const;
	constexpr Vector4 wzwy() const;
	constexpr Vector4 wzwz() const;
	constexpr Vector4 wzww() const;
	constexpr Vector4 wwxx() const;
	constexpr Vector4 wwxy() const;
	constexpr Vector4 wwxz() const;
	constexpr Vector4 wwxw() const;
	constexpr Vector4 wwyx() const;
	constexpr Vector4 wwyy() const;
	constexpr Vector4 wwyz() const;
	constexpr Vector4 wwyw() const;
	constexpr Vector4 wwzx() const;
	constexpr Vector4 wwzy() const;
	constexpr Vector4 wwzz() const;
	constexpr Vector4 wwzw() const;
	constexpr Vector4 wwwx() const;
	constexpr Vector4 wwwy() const;
	constexpr Vector4 wwwz() const;
	constexpr Vector4 wwww() const;
#pragma endregion

	constexpr explicit operator Vector2() const;
	constexpr explicit operator Vector3() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector3Int() const;
	constexpr explicit operator Vector4Int() const;

	//operator String8() const { return String8(x, ", ", y, ", ", z, ", ", w); }
	//operator String16() const { return String16(x, u", ", y, u", ", z, u", ", w); }
	//operator String32() const { return String32(x, U", ", y, U", ", z, U", ", w); }

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
	constexpr Vector2Int();
	constexpr Vector2Int(I32 i);
	constexpr Vector2Int(I32 x, I32 y);
	constexpr Vector2Int(const Vector2Int& v);
	constexpr Vector2Int(Vector2Int&& v) noexcept;

	constexpr Vector2Int& operator=(I32 i);
	constexpr Vector2Int& operator=(const Vector2Int& v);
	constexpr Vector2Int& operator=(Vector2Int&& v) noexcept;

	constexpr Vector2Int& operator+=(I32 i);
	constexpr Vector2Int& operator-=(I32 i);
	constexpr Vector2Int& operator*=(I32 i);
	constexpr Vector2Int& operator/=(I32 i);
	constexpr Vector2Int& operator%=(I32 i);
	constexpr Vector2Int& operator+=(const Vector2Int& v);
	constexpr Vector2Int& operator-=(const Vector2Int& v);
	constexpr Vector2Int& operator*=(const Vector2Int& v);
	constexpr Vector2Int& operator/=(const Vector2Int& v);
	constexpr Vector2Int& operator%=(const Vector2Int& v);

	constexpr Vector2Int operator+(I32 i) const;
	constexpr Vector2Int operator-(I32 i) const;
	constexpr Vector2Int operator*(I32 i) const;
	constexpr Vector2Int operator/(I32 i) const;
	constexpr Vector2Int operator%(I32 i) const;
	constexpr Vector2Int operator+(const Vector2Int& v) const;
	constexpr Vector2Int operator-(const Vector2Int& v) const;
	constexpr Vector2Int operator*(const Vector2Int& v) const;
	constexpr Vector2Int operator/(const Vector2Int& v) const;
	constexpr Vector2Int operator%(const Vector2Int& v) const;
	constexpr Vector2Int operator+(const Vector2& v) const;
	constexpr Vector2Int operator-(const Vector2& v) const;
	constexpr Vector2Int operator*(const Vector2& v) const;
	constexpr Vector2Int operator/(const Vector2& v) const;

	constexpr bool operator==(const Vector2Int& v) const;
	constexpr bool operator!=(const Vector2Int& v) const;
	constexpr bool operator>(const Vector2Int& v) const;
	constexpr bool operator<(const Vector2Int& v) const;
	constexpr bool operator>=(const Vector2Int& v) const;
	constexpr bool operator<=(const Vector2Int& v) const;
	constexpr bool IsZero() const;

	constexpr Vector2Int operator-() const;
	constexpr Vector2Int operator~() const;
	constexpr Vector2Int operator!() const;

	constexpr I32 SqrMagnitude() const;
	constexpr F32 Magnitude() const;
	constexpr I32 Dot(const Vector2Int& v) const;
	constexpr I32 Dot(I32 vx, I32 vy) const;

	constexpr Vector2Int& Clamp(const Vector2Int& min, const Vector2Int& max);
	constexpr Vector2Int Clamped(const Vector2Int& min, const Vector2Int& max) const;
	constexpr Vector2Int& SetClosest(const Vector2Int& min, const Vector2Int& max);
	constexpr Vector2Int Closest(const Vector2Int& min, const Vector2Int& max) const;

	I32& operator[] (U64 i);
	const I32& operator[] (U64 i) const;

	I32* Data();
	const I32* Data() const;

	constexpr Vector2Int xx() const;
	constexpr Vector2Int xy() const;
	constexpr Vector2Int yx() const;
	constexpr Vector2Int yy() const;

	constexpr explicit operator Vector2() const;
	constexpr explicit operator Vector3() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector3Int() const;
	constexpr explicit operator Vector4Int() const;

	//operator String8() const { return String8(x, ", ", y); }
	//operator String16() const { return String16(x, u", ", y); }
	//operator String32() const { return String32(x, U", ", y); }

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
	constexpr Vector3Int();
	constexpr Vector3Int(I32 i);
	constexpr Vector3Int(I32 x, I32 y, I32 z);
	constexpr Vector3Int(const Vector3Int& v);
	constexpr Vector3Int(Vector3Int&& v) noexcept;

	constexpr Vector3Int& operator=(I32 i);
	constexpr Vector3Int& operator=(const Vector3Int& v);
	constexpr Vector3Int& operator=(Vector3Int&& v) noexcept;

	constexpr Vector3Int& operator+=(I32 i);
	constexpr Vector3Int& operator-=(I32 i);
	constexpr Vector3Int& operator*=(I32 i);
	constexpr Vector3Int& operator/=(I32 i);
	constexpr Vector3Int& operator%=(I32 i);
	constexpr Vector3Int& operator+=(const Vector3Int& v);
	constexpr Vector3Int& operator-=(const Vector3Int& v);
	constexpr Vector3Int& operator*=(const Vector3Int& v);
	constexpr Vector3Int& operator/=(const Vector3Int& v);
	constexpr Vector3Int& operator%=(const Vector3Int& v);

	constexpr Vector3Int operator+(I32 i) const;
	constexpr Vector3Int operator-(I32 i) const;
	constexpr Vector3Int operator*(I32 i) const;
	constexpr Vector3Int operator/(I32 i) const;
	constexpr Vector3Int operator%(I32 i) const;
	constexpr Vector3Int operator+(const Vector3Int& v) const;
	constexpr Vector3Int operator-(const Vector3Int& v) const;
	constexpr Vector3Int operator*(const Vector3Int& v) const;
	constexpr Vector3Int operator/(const Vector3Int& v) const;
	constexpr Vector3Int operator%(const Vector3Int& v) const;

	constexpr bool operator==(const Vector3Int& v) const;
	constexpr bool operator!=(const Vector3Int& v) const;
	constexpr bool operator>(const Vector3Int& v) const;
	constexpr bool operator<(const Vector3Int& v) const;
	constexpr bool operator>=(const Vector3Int& v) const;
	constexpr bool operator<=(const Vector3Int& v) const;
	constexpr bool IsZero() const;

	constexpr Vector3Int operator-() const;
	constexpr Vector3Int operator~() const;
	constexpr Vector3Int operator!() const;

	constexpr I32 SqrMagnitude() const;
	constexpr F32 Magnitude() const;
	constexpr I32 Dot(const Vector3Int& v) const;
	constexpr I32 Dot(I32 vx, I32 vy, I32 vz) const;

	constexpr Vector3Int& Clamp(const Vector3Int& min, const Vector3Int& max);
	constexpr Vector3Int Clamped(const Vector3Int& min, const Vector3Int& max) const;
	constexpr Vector3Int& SetClosest(const Vector3Int& min, const Vector3Int& max);
	constexpr Vector3Int Closest(const Vector3Int& min, const Vector3Int& max) const;

	I32& operator[] (U64 i);
	const I32& operator[] (U64 i) const;

	I32* Data();
	const I32* Data() const;

#pragma region SwizzleFunctions
	constexpr Vector2Int xx() const;
	constexpr Vector2Int xy() const;
	constexpr Vector2Int xz() const;
	constexpr Vector2Int yx() const;
	constexpr Vector2Int yy() const;
	constexpr Vector2Int yz() const;
	constexpr Vector2Int zx() const;
	constexpr Vector2Int zy() const;
	constexpr Vector2Int zz() const;
	constexpr Vector3Int xxx() const;
	constexpr Vector3Int xxy() const;
	constexpr Vector3Int xxz() const;
	constexpr Vector3Int xyx() const;
	constexpr Vector3Int xyy() const;
	constexpr Vector3Int xyz() const;
	constexpr Vector3Int xzx() const;
	constexpr Vector3Int xzy() const;
	constexpr Vector3Int xzz() const;
	constexpr Vector3Int yxx() const;
	constexpr Vector3Int yxy() const;
	constexpr Vector3Int yxz() const;
	constexpr Vector3Int yyx() const;
	constexpr Vector3Int yyy() const;
	constexpr Vector3Int yyz() const;
	constexpr Vector3Int yzx() const;
	constexpr Vector3Int yzy() const;
	constexpr Vector3Int yzz() const;
	constexpr Vector3Int zxx() const;
	constexpr Vector3Int zxy() const;
	constexpr Vector3Int zxz() const;
	constexpr Vector3Int zyx() const;
	constexpr Vector3Int zyy() const;
	constexpr Vector3Int zyz() const;
	constexpr Vector3Int zzx() const;
	constexpr Vector3Int zzy() const;
	constexpr Vector3Int zzz() const;
#pragma endregion

	constexpr explicit operator Vector2() const;
	constexpr explicit operator Vector3() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector4Int() const;

	//operator String8() const { return String8(x, ", ", y, ", ", z); }
	//operator String16() const { return String16(x, u", ", y, u", ", z); }
	//operator String32() const { return String32(x, U", ", y, U", ", z); }

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
	constexpr Vector4Int();
	constexpr Vector4Int(I32 i);
	constexpr Vector4Int(I32 x, I32 y, I32 z, I32 w);
	constexpr Vector4Int(const Vector4Int& v);
	constexpr Vector4Int(Vector4Int&& v) noexcept;

	constexpr Vector4Int& operator=(I32 i);
	constexpr Vector4Int& operator=(const Vector4Int& v);
	constexpr Vector4Int& operator=(Vector4Int&& v) noexcept;

	constexpr Vector4Int& operator+=(I32 i);
	constexpr Vector4Int& operator-=(I32 i);
	constexpr Vector4Int& operator*=(I32 i);
	constexpr Vector4Int& operator/=(I32 i);
	constexpr Vector4Int& operator%=(I32 i);
	constexpr Vector4Int& operator+=(F32 f);
	constexpr Vector4Int& operator-=(F32 f);
	constexpr Vector4Int& operator*=(F32 f);
	constexpr Vector4Int& operator/=(F32 f);
	constexpr Vector4Int& operator+=(const Vector4Int& v);
	constexpr Vector4Int& operator-=(const Vector4Int& v);
	constexpr Vector4Int& operator*=(const Vector4Int& v);
	constexpr Vector4Int& operator/=(const Vector4Int& v);
	constexpr Vector4Int& operator%=(const Vector4Int& v);

	constexpr Vector4Int operator+(I32 i) const;
	constexpr Vector4Int operator-(I32 i) const;
	constexpr Vector4Int operator*(I32 i) const;
	constexpr Vector4Int operator/(I32 i) const;
	constexpr Vector4Int operator%(I32 i) const;
	constexpr Vector4Int operator+(F32 f) const;
	constexpr Vector4Int operator-(F32 f) const;
	constexpr Vector4Int operator*(F32 f) const;
	constexpr Vector4Int operator/(F32 f) const;
	constexpr Vector4Int operator+(const Vector4Int& v) const;
	constexpr Vector4Int operator-(const Vector4Int& v) const;
	constexpr Vector4Int operator*(const Vector4Int& v) const;
	constexpr Vector4Int operator/(const Vector4Int& v) const;
	constexpr Vector4Int operator%(const Vector4Int& v) const;

	constexpr bool operator==(const Vector4Int& v) const;
	constexpr bool operator!=(const Vector4Int& v) const;
	constexpr bool operator>(const Vector4Int& v) const;
	constexpr bool operator<(const Vector4Int& v) const;
	constexpr bool operator>=(const Vector4Int& v) const;
	constexpr bool operator<=(const Vector4Int& v) const;
	constexpr bool IsZero() const;

	constexpr Vector4Int operator-() const;
	constexpr Vector4Int operator~() const;
	constexpr Vector4Int operator!() const;

	constexpr I32 SqrMagnitude() const;
	constexpr F32 Magnitude() const;
	constexpr I32 Dot(const Vector4Int& v) const;
	constexpr I32 Dot(I32 vx, I32 vy, I32 vz, I32 vw) const;
	constexpr Vector4Int& Clamp(const Vector4Int& min, const Vector4Int& max);
	constexpr Vector4Int Clamped(const Vector4Int& min, const Vector4Int& max) const;
	constexpr Vector4Int& SetClosest(const Vector4Int& min, const Vector4Int& max);
	constexpr Vector4Int Closest(const Vector4Int& min, const Vector4Int& max) const;

	I32& operator[] (U64 i);
	const I32& operator[] (U64 i) const;

	I32* Data();
	const I32* Data() const;

#pragma region SwizzleFunctions
	constexpr Vector2Int xx() const;
	constexpr Vector2Int xy() const;
	constexpr Vector2Int xz() const;
	constexpr Vector2Int xw() const;
	constexpr Vector2Int yx() const;
	constexpr Vector2Int yy() const;
	constexpr Vector2Int yz() const;
	constexpr Vector2Int yw() const;
	constexpr Vector2Int zx() const;
	constexpr Vector2Int zy() const;
	constexpr Vector2Int zz() const;
	constexpr Vector2Int zw() const;
	constexpr Vector2Int wx() const;
	constexpr Vector2Int wy() const;
	constexpr Vector2Int wz() const;
	constexpr Vector2Int ww() const;
	constexpr Vector3Int xxx() const;
	constexpr Vector3Int xxy() const;
	constexpr Vector3Int xxz() const;
	constexpr Vector3Int xxw() const;
	constexpr Vector3Int xyx() const;
	constexpr Vector3Int xyy() const;
	constexpr Vector3Int xyz() const;
	constexpr Vector3Int xyw() const;
	constexpr Vector3Int xzx() const;
	constexpr Vector3Int xzy() const;
	constexpr Vector3Int xzz() const;
	constexpr Vector3Int xzw() const;
	constexpr Vector3Int xwx() const;
	constexpr Vector3Int xwy() const;
	constexpr Vector3Int xwz() const;
	constexpr Vector3Int xww() const;
	constexpr Vector3Int yxx() const;
	constexpr Vector3Int yxy() const;
	constexpr Vector3Int yxz() const;
	constexpr Vector3Int yxw() const;
	constexpr Vector3Int yyx() const;
	constexpr Vector3Int yyy() const;
	constexpr Vector3Int yyz() const;
	constexpr Vector3Int yyw() const;
	constexpr Vector3Int yzx() const;
	constexpr Vector3Int yzy() const;
	constexpr Vector3Int yzz() const;
	constexpr Vector3Int yzw() const;
	constexpr Vector3Int ywx() const;
	constexpr Vector3Int ywy() const;
	constexpr Vector3Int ywz() const;
	constexpr Vector3Int yww() const;
	constexpr Vector3Int zxx() const;
	constexpr Vector3Int zxy() const;
	constexpr Vector3Int zxz() const;
	constexpr Vector3Int zxw() const;
	constexpr Vector3Int zyx() const;
	constexpr Vector3Int zyy() const;
	constexpr Vector3Int zyz() const;
	constexpr Vector3Int zyw() const;
	constexpr Vector3Int zzx() const;
	constexpr Vector3Int zzy() const;
	constexpr Vector3Int zzz() const;
	constexpr Vector3Int zzw() const;
	constexpr Vector3Int zwx() const;
	constexpr Vector3Int zwy() const;
	constexpr Vector3Int zwz() const;
	constexpr Vector3Int zww() const;
	constexpr Vector3Int wxx() const;
	constexpr Vector3Int wxy() const;
	constexpr Vector3Int wxz() const;
	constexpr Vector3Int wxw() const;
	constexpr Vector3Int wyx() const;
	constexpr Vector3Int wyy() const;
	constexpr Vector3Int wyz() const;
	constexpr Vector3Int wyw() const;
	constexpr Vector3Int wzx() const;
	constexpr Vector3Int wzy() const;
	constexpr Vector3Int wzz() const;
	constexpr Vector3Int wzw() const;
	constexpr Vector3Int wwx() const;
	constexpr Vector3Int wwy() const;
	constexpr Vector3Int wwz() const;
	constexpr Vector3Int www() const;
	constexpr Vector4Int xxxx() const;
	constexpr Vector4Int xxxy() const;
	constexpr Vector4Int xxxz() const;
	constexpr Vector4Int xxxw() const;
	constexpr Vector4Int xxyx() const;
	constexpr Vector4Int xxyy() const;
	constexpr Vector4Int xxyz() const;
	constexpr Vector4Int xxyw() const;
	constexpr Vector4Int xxzx() const;
	constexpr Vector4Int xxzy() const;
	constexpr Vector4Int xxzz() const;
	constexpr Vector4Int xxzw() const;
	constexpr Vector4Int xxwx() const;
	constexpr Vector4Int xxwy() const;
	constexpr Vector4Int xxwz() const;
	constexpr Vector4Int xxww() const;
	constexpr Vector4Int xyxx() const;
	constexpr Vector4Int xyxy() const;
	constexpr Vector4Int xyxz() const;
	constexpr Vector4Int xyxw() const;
	constexpr Vector4Int xyyx() const;
	constexpr Vector4Int xyyy() const;
	constexpr Vector4Int xyyz() const;
	constexpr Vector4Int xyyw() const;
	constexpr Vector4Int xyzx() const;
	constexpr Vector4Int xyzy() const;
	constexpr Vector4Int xyzz() const;
	constexpr Vector4Int xyzw() const;
	constexpr Vector4Int xywx() const;
	constexpr Vector4Int xywy() const;
	constexpr Vector4Int xywz() const;
	constexpr Vector4Int xyww() const;
	constexpr Vector4Int xzxx() const;
	constexpr Vector4Int xzxy() const;
	constexpr Vector4Int xzxz() const;
	constexpr Vector4Int xzxw() const;
	constexpr Vector4Int xzyx() const;
	constexpr Vector4Int xzyy() const;
	constexpr Vector4Int xzyz() const;
	constexpr Vector4Int xzyw() const;
	constexpr Vector4Int xzzx() const;
	constexpr Vector4Int xzzy() const;
	constexpr Vector4Int xzzz() const;
	constexpr Vector4Int xzzw() const;
	constexpr Vector4Int xzwx() const;
	constexpr Vector4Int xzwy() const;
	constexpr Vector4Int xzwz() const;
	constexpr Vector4Int xzww() const;
	constexpr Vector4Int xwxx() const;
	constexpr Vector4Int xwxy() const;
	constexpr Vector4Int xwxz() const;
	constexpr Vector4Int xwxw() const;
	constexpr Vector4Int xwyx() const;
	constexpr Vector4Int xwyy() const;
	constexpr Vector4Int xwyz() const;
	constexpr Vector4Int xwyw() const;
	constexpr Vector4Int xwzx() const;
	constexpr Vector4Int xwzy() const;
	constexpr Vector4Int xwzz() const;
	constexpr Vector4Int xwzw() const;
	constexpr Vector4Int xwwx() const;
	constexpr Vector4Int xwwy() const;
	constexpr Vector4Int xwwz() const;
	constexpr Vector4Int xwww() const;
	constexpr Vector4Int yxxx() const;
	constexpr Vector4Int yxxy() const;
	constexpr Vector4Int yxxz() const;
	constexpr Vector4Int yxxw() const;
	constexpr Vector4Int yxyx() const;
	constexpr Vector4Int yxyy() const;
	constexpr Vector4Int yxyz() const;
	constexpr Vector4Int yxyw() const;
	constexpr Vector4Int yxzx() const;
	constexpr Vector4Int yxzy() const;
	constexpr Vector4Int yxzz() const;
	constexpr Vector4Int yxzw() const;
	constexpr Vector4Int yxwx() const;
	constexpr Vector4Int yxwy() const;
	constexpr Vector4Int yxwz() const;
	constexpr Vector4Int yxww() const;
	constexpr Vector4Int yyxx() const;
	constexpr Vector4Int yyxy() const;
	constexpr Vector4Int yyxz() const;
	constexpr Vector4Int yyxw() const;
	constexpr Vector4Int yyyx() const;
	constexpr Vector4Int yyyy() const;
	constexpr Vector4Int yyyz() const;
	constexpr Vector4Int yyyw() const;
	constexpr Vector4Int yyzx() const;
	constexpr Vector4Int yyzy() const;
	constexpr Vector4Int yyzz() const;
	constexpr Vector4Int yyzw() const;
	constexpr Vector4Int yywx() const;
	constexpr Vector4Int yywy() const;
	constexpr Vector4Int yywz() const;
	constexpr Vector4Int yyww() const;
	constexpr Vector4Int yzxx() const;
	constexpr Vector4Int yzxy() const;
	constexpr Vector4Int yzxz() const;
	constexpr Vector4Int yzxw() const;
	constexpr Vector4Int yzyx() const;
	constexpr Vector4Int yzyy() const;
	constexpr Vector4Int yzyz() const;
	constexpr Vector4Int yzyw() const;
	constexpr Vector4Int yzzx() const;
	constexpr Vector4Int yzzy() const;
	constexpr Vector4Int yzzz() const;
	constexpr Vector4Int yzzw() const;
	constexpr Vector4Int yzwx() const;
	constexpr Vector4Int yzwy() const;
	constexpr Vector4Int yzwz() const;
	constexpr Vector4Int yzww() const;
	constexpr Vector4Int ywxx() const;
	constexpr Vector4Int ywxy() const;
	constexpr Vector4Int ywxz() const;
	constexpr Vector4Int ywxw() const;
	constexpr Vector4Int ywyx() const;
	constexpr Vector4Int ywyy() const;
	constexpr Vector4Int ywyz() const;
	constexpr Vector4Int ywyw() const;
	constexpr Vector4Int ywzx() const;
	constexpr Vector4Int ywzy() const;
	constexpr Vector4Int ywzz() const;
	constexpr Vector4Int ywzw() const;
	constexpr Vector4Int ywwx() const;
	constexpr Vector4Int ywwy() const;
	constexpr Vector4Int ywwz() const;
	constexpr Vector4Int ywww() const;
	constexpr Vector4Int zxxx() const;
	constexpr Vector4Int zxxy() const;
	constexpr Vector4Int zxxz() const;
	constexpr Vector4Int zxxw() const;
	constexpr Vector4Int zxyx() const;
	constexpr Vector4Int zxyy() const;
	constexpr Vector4Int zxyz() const;
	constexpr Vector4Int zxyw() const;
	constexpr Vector4Int zxzx() const;
	constexpr Vector4Int zxzy() const;
	constexpr Vector4Int zxzz() const;
	constexpr Vector4Int zxzw() const;
	constexpr Vector4Int zxwx() const;
	constexpr Vector4Int zxwy() const;
	constexpr Vector4Int zxwz() const;
	constexpr Vector4Int zxww() const;
	constexpr Vector4Int zyxx() const;
	constexpr Vector4Int zyxy() const;
	constexpr Vector4Int zyxz() const;
	constexpr Vector4Int zyxw() const;
	constexpr Vector4Int zyyx() const;
	constexpr Vector4Int zyyy() const;
	constexpr Vector4Int zyyz() const;
	constexpr Vector4Int zyyw() const;
	constexpr Vector4Int zyzx() const;
	constexpr Vector4Int zyzy() const;
	constexpr Vector4Int zyzz() const;
	constexpr Vector4Int zyzw() const;
	constexpr Vector4Int zywx() const;
	constexpr Vector4Int zywy() const;
	constexpr Vector4Int zywz() const;
	constexpr Vector4Int zyww() const;
	constexpr Vector4Int zzxx() const;
	constexpr Vector4Int zzxy() const;
	constexpr Vector4Int zzxz() const;
	constexpr Vector4Int zzxw() const;
	constexpr Vector4Int zzyx() const;
	constexpr Vector4Int zzyy() const;
	constexpr Vector4Int zzyz() const;
	constexpr Vector4Int zzyw() const;
	constexpr Vector4Int zzzx() const;
	constexpr Vector4Int zzzy() const;
	constexpr Vector4Int zzzz() const;
	constexpr Vector4Int zzzw() const;
	constexpr Vector4Int zzwx() const;
	constexpr Vector4Int zzwy() const;
	constexpr Vector4Int zzwz() const;
	constexpr Vector4Int zzww() const;
	constexpr Vector4Int zwxx() const;
	constexpr Vector4Int zwxy() const;
	constexpr Vector4Int zwxz() const;
	constexpr Vector4Int zwxw() const;
	constexpr Vector4Int zwyx() const;
	constexpr Vector4Int zwyy() const;
	constexpr Vector4Int zwyz() const;
	constexpr Vector4Int zwyw() const;
	constexpr Vector4Int zwzx() const;
	constexpr Vector4Int zwzy() const;
	constexpr Vector4Int zwzz() const;
	constexpr Vector4Int zwzw() const;
	constexpr Vector4Int zwwx() const;
	constexpr Vector4Int zwwy() const;
	constexpr Vector4Int zwwz() const;
	constexpr Vector4Int zwww() const;
	constexpr Vector4Int wxxx() const;
	constexpr Vector4Int wxxy() const;
	constexpr Vector4Int wxxz() const;
	constexpr Vector4Int wxxw() const;
	constexpr Vector4Int wxyx() const;
	constexpr Vector4Int wxyy() const;
	constexpr Vector4Int wxyz() const;
	constexpr Vector4Int wxyw() const;
	constexpr Vector4Int wxzx() const;
	constexpr Vector4Int wxzy() const;
	constexpr Vector4Int wxzz() const;
	constexpr Vector4Int wxzw() const;
	constexpr Vector4Int wxwx() const;
	constexpr Vector4Int wxwy() const;
	constexpr Vector4Int wxwz() const;
	constexpr Vector4Int wxww() const;
	constexpr Vector4Int wyxx() const;
	constexpr Vector4Int wyxy() const;
	constexpr Vector4Int wyxz() const;
	constexpr Vector4Int wyxw() const;
	constexpr Vector4Int wyyx() const;
	constexpr Vector4Int wyyy() const;
	constexpr Vector4Int wyyz() const;
	constexpr Vector4Int wyyw() const;
	constexpr Vector4Int wyzx() const;
	constexpr Vector4Int wyzy() const;
	constexpr Vector4Int wyzz() const;
	constexpr Vector4Int wyzw() const;
	constexpr Vector4Int wywx() const;
	constexpr Vector4Int wywy() const;
	constexpr Vector4Int wywz() const;
	constexpr Vector4Int wyww() const;
	constexpr Vector4Int wzxx() const;
	constexpr Vector4Int wzxy() const;
	constexpr Vector4Int wzxz() const;
	constexpr Vector4Int wzxw() const;
	constexpr Vector4Int wzyx() const;
	constexpr Vector4Int wzyy() const;
	constexpr Vector4Int wzyz() const;
	constexpr Vector4Int wzyw() const;
	constexpr Vector4Int wzzx() const;
	constexpr Vector4Int wzzy() const;
	constexpr Vector4Int wzzz() const;
	constexpr Vector4Int wzzw() const;
	constexpr Vector4Int wzwx() const;
	constexpr Vector4Int wzwy() const;
	constexpr Vector4Int wzwz() const;
	constexpr Vector4Int wzww() const;
	constexpr Vector4Int wwxx() const;
	constexpr Vector4Int wwxy() const;
	constexpr Vector4Int wwxz() const;
	constexpr Vector4Int wwxw() const;
	constexpr Vector4Int wwyx() const;
	constexpr Vector4Int wwyy() const;
	constexpr Vector4Int wwyz() const;
	constexpr Vector4Int wwyw() const;
	constexpr Vector4Int wwzx() const;
	constexpr Vector4Int wwzy() const;
	constexpr Vector4Int wwzz() const;
	constexpr Vector4Int wwzw() const;
	constexpr Vector4Int wwwx() const;
	constexpr Vector4Int wwwy() const;
	constexpr Vector4Int wwwz() const;
	constexpr Vector4Int wwww() const;
#pragma endregion

	constexpr explicit operator Vector2() const;
	constexpr explicit operator Vector3() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector3Int() const;

	//operator String8() const { return String8(x, ", ", y, ", ", z, ", ", w); }
	//operator String16() const { return String16(x, u", ", y, u", ", z, u", ", w); }
	//operator String32() const { return String32(x, U", ", y, U", ", z, U", ", w); }

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

struct NH_API Matrix2
{
	constexpr Matrix2();
	constexpr Matrix2(F32 ax, F32 ay, F32 bx, F32 by);
	constexpr Matrix2(const Vector2& v);
	constexpr Matrix2(const Vector2& a, const Vector2& b);
	constexpr Matrix2(Vector2&& a, Vector2&& b) noexcept;
	constexpr Matrix2(const Matrix2& m);
	constexpr Matrix2(Matrix2&& m) noexcept;

	constexpr Matrix2& operator= (const Matrix2& m);
	constexpr Matrix2& operator= (Matrix2&& m) noexcept;

	constexpr Matrix2& operator+= (const Matrix2& m);
	constexpr Matrix2& operator-= (const Matrix2& m);
	constexpr Matrix2& operator*= (const Matrix2& m);

	constexpr Matrix2 operator+(const Matrix2& m) const;
	constexpr Matrix2 operator-(const Matrix2& m) const;
	constexpr Matrix2 operator*(const Matrix2& m) const;
	constexpr Vector2 operator*(const Vector2& v) const;

	constexpr Vector2 Solve(const Vector2& v) const;

	constexpr Matrix2 Inverse() const;
	constexpr Matrix2& Inversed();
	constexpr Matrix2 Transpose() const;
	constexpr Matrix2& Transposed();

	constexpr Matrix2 operator-() const;
	constexpr Matrix2 operator~() const;
	constexpr Matrix2 operator!() const;

	constexpr bool operator==(const Matrix2& m) const;
	constexpr bool operator!=(const Matrix2& m) const;

	const Vector2& operator[] (U8 i) const;
	Vector2& operator[] (U8 i);

	F32* Data();
	const F32* Data() const;

public:
	Vector2 a, b; //Columns
};

struct NH_API Matrix3
{
	constexpr Matrix3();
	constexpr Matrix3(F32 ax, F32 ay, F32 az, F32 bx, F32 by, F32 bz, F32 cx, F32 cy, F32 cz);
	constexpr Matrix3(const Vector3& v);
	constexpr Matrix3(const Vector3& a, const Vector3& b, const Vector3& c);
	constexpr Matrix3(Vector3&& v1, Vector3&& v2, Vector3&& v3) noexcept;
	constexpr Matrix3(const Matrix3& m);
	constexpr Matrix3(Matrix3&& m) noexcept;
	constexpr Matrix3(const Vector2& position, const F32& rotation, const Vector2& scale);
	constexpr Matrix3(const Vector2& position, const Quaternion2& rotation, const Vector2& scale);

	constexpr Matrix3& operator= (const Matrix3& m);
	constexpr Matrix3& operator= (Matrix3&& m) noexcept;
	constexpr void Set(const Vector2& position, const F32& rotation, const Vector2& scale);
	constexpr void Set(const Vector2& position, const Quaternion2& rotation, const Vector2& scale);

	constexpr Matrix3& operator+= (const Matrix3& m);
	constexpr Matrix3& operator-= (const Matrix3& m);
	constexpr Matrix3& operator*= (const Matrix3& m);

	constexpr Matrix3 operator+(const Matrix3& m) const;
	constexpr Matrix3 operator-(const Matrix3& m) const;
	constexpr Matrix3 operator*(const Matrix3& m) const;
	constexpr Vector3 operator*(const Vector3& v) const;

	constexpr Matrix3 Inverse() const;
	constexpr Matrix3& Inversed();
	constexpr Matrix3 Transpose() const;
	constexpr Matrix3& Transposed();

	constexpr Matrix3 operator-() const;
	constexpr Matrix3 operator~() const;
	constexpr Matrix3 operator!() const;

	constexpr bool operator==(const Matrix3& m) const;
	constexpr bool operator!=(const Matrix3& m) const;

	const Vector3& operator[] (U8 i) const;
	Vector3& operator[] (U8 i);

	F32* Data();
	const F32* Data() const;

public:
	Vector3 a, b, c; //Columns
};

struct NH_API Matrix4
{
	static constexpr Matrix4 Translate(const Vector3& position);
	static constexpr Matrix4 Rotate(const Vector3& rotation);
	static constexpr Matrix4 Rotate(const Matrix4& m, F32 angle, const Vector3& axis);
	static constexpr Matrix4 Rotate(const Quaternion3& rotation);
	static constexpr Matrix4 Scale(const Vector3& scale);

	constexpr Matrix4();
	constexpr Matrix4(F32 ax, F32 ay, F32 az, F32 aw, F32 bx, F32 by, F32 bz, F32 bw, F32 cx, F32 cy, F32 cz, F32 cw, F32 dx, F32 dy, F32 dz, F32 dw);
	constexpr Matrix4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);
	constexpr Matrix4(Vector4&& a, Vector4&& b, Vector4&& c, Vector4&& d) noexcept;
	constexpr Matrix4(const Matrix4& m);
	constexpr Matrix4(const Matrix3& m);
	constexpr Matrix4(Matrix4&& m) noexcept;
	constexpr Matrix4(const Vector3& position, const Vector3& rotation = (0.0f), const Vector3& scale = (1.0f));
	constexpr Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale = (1.0f));

	constexpr Matrix4& operator= (const Matrix4& m);
	constexpr Matrix4& operator= (Matrix4&& m) noexcept;
	constexpr Matrix4& operator+= (const Matrix4& m);
	constexpr Matrix4& operator-= (const Matrix4& m);
	constexpr Matrix4& operator*= (const Matrix4& m);

	constexpr Matrix4 operator+(const Matrix4& m) const;
	constexpr Matrix4 operator-(const Matrix4& m) const;
	constexpr Matrix4 operator*(const Matrix4& m) const;
	constexpr Vector2 operator*(const Vector2& v) const;
	constexpr Vector3 operator*(const Vector3& v) const;
	constexpr Vector4 operator*(const Vector4& v) const;

	constexpr Matrix4 Inverse() const;
	constexpr Matrix4& Inversed();
	constexpr Matrix4 Invert() const;
	constexpr Matrix4& Inverted();
	constexpr Matrix4 Transpose() const;
	constexpr Matrix4& Transposed();

	constexpr void Set(const Vector3& position, const Vector3& rotation, const Vector3& scale);
	constexpr void Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale);
	constexpr void SetPerspective(F32 fov, F32 aspect, F32 nearPlane, F32 farPlane);
	constexpr void SetOrthographic(F32 left, F32 right, F32 bottom, F32 top, F32 nearPlane, F32 farPlane);
	constexpr void SetPosition(const Vector3& position);

	constexpr void LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);

	constexpr Vector3 Forward();
	constexpr Vector3 Back();
	constexpr Vector3 Right();
	constexpr Vector3 Left();
	constexpr Vector3 Up();
	constexpr Vector3 Down();

	constexpr Matrix4 operator-();
	constexpr Matrix4 operator~();
	constexpr Matrix4 operator!();

	constexpr bool operator==(const Matrix4& m) const;
	constexpr bool operator!=(const Matrix4& m) const;

	Vector4& operator[] (U8 i);
	const Vector4& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

public:
	Vector4 a, b, c, d; //Columns

	static const Matrix4 Identity;
	static const Matrix4 Zero;
};

struct NH_API Quaternion2
{
	constexpr Quaternion2();
	constexpr Quaternion2(F32 x, F32 y);
	constexpr Quaternion2(F32 angle);
	constexpr Quaternion2(const Matrix2& mat);
	constexpr Quaternion2(const Quaternion2& q);
	constexpr Quaternion2(Quaternion2&& q) noexcept;

	constexpr Quaternion2& operator=(F32 angle);
	constexpr Quaternion2& operator=(const Quaternion2& q);
	constexpr Quaternion2& operator=(Quaternion2&& q) noexcept;

	constexpr Quaternion2& operator+=(const Quaternion2& q);
	constexpr Quaternion2& operator-=(const Quaternion2& q);
	constexpr Quaternion2& operator*=(const Quaternion2& q);
	constexpr Quaternion2& operator/=(const Quaternion2& q);

	constexpr Quaternion2 operator*(F32 f) const;
	constexpr Quaternion2 operator/(F32 f) const;
	constexpr Quaternion2 operator+(const Quaternion2& q) const;
	constexpr Quaternion2 operator-(const Quaternion2& q) const;
	constexpr Quaternion2 operator*(const Quaternion2& q) const;
	constexpr Quaternion2 operator^(const Quaternion2& q) const;
	constexpr Quaternion2 operator/(const Quaternion2& q) const;

	constexpr void Set(F32 angle);
	constexpr void Rotate(F32 angle);
	constexpr F32 Angle() const;

	constexpr Matrix2 ToMatrix2() const;
	constexpr Matrix3 ToMatrix3() const;
	constexpr Matrix4 RotationMatrix(Vector2 center) const;

	constexpr Quaternion2 Slerp(const Quaternion2& q, F32 t) const;
	constexpr Quaternion2& Slerped(const Quaternion2& q, F32 t);
	constexpr Quaternion2 NLerp(const Quaternion2& q, F32 t) const;
	constexpr Quaternion2& NLerped(const Quaternion2& q, F32 t);

	constexpr F32 RelativeAngle(const Quaternion2& q) const;
	constexpr F32 Dot(const Quaternion2& q) const;
	constexpr F32 SqrNormal() const;
	constexpr F32 Normal() const;
	constexpr Quaternion2 Normalize() const;
	constexpr Quaternion2& Normalized();
	constexpr Quaternion2 Conjugate() const;
	constexpr Quaternion2& Conjugated();
	constexpr Quaternion2 Inverse() const;
	constexpr Quaternion2& Inversed();
	constexpr Quaternion2 Integrate(F32 deltaAngle);
	constexpr Quaternion2& Integrated(F32 deltaAngle);

	F32& operator[] (U8 i);
	const F32& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

	constexpr explicit operator Quaternion3() const;

	//TODO:
	//operator String8() const;
	//operator String16() const;
	//operator String32() const;

public:
	F32 x, y; //sin, cos

	static const Quaternion2 Identity;
};

//W is real part
struct NH_API Quaternion3
{
	constexpr Quaternion3();
	constexpr Quaternion3(F32 x, F32 y, F32 z, F32 w);
	constexpr Quaternion3(const Vector3& euler);
	constexpr Quaternion3(const Vector3& axis, F32 angle);
	constexpr Quaternion3(const Quaternion3& q);
	constexpr Quaternion3(const Quaternion2& q);
	constexpr Quaternion3(Quaternion3&& q) noexcept;

	constexpr Quaternion3& operator=(const Vector3& euler);

	constexpr Quaternion3& operator=(const Quaternion3& q);
	constexpr Quaternion3& operator=(Quaternion3&& q) noexcept;

	constexpr Quaternion3& operator+=(const Quaternion3& q);
	constexpr Quaternion3& operator-=(const Quaternion3& q);
	constexpr Quaternion3& operator*=(const Quaternion3& q);
	constexpr Quaternion3& operator/=(const Quaternion3& q);

	constexpr Quaternion3 operator+(const Quaternion3& q) const;
	constexpr Quaternion3 operator-(const Quaternion3& q) const;
	constexpr Quaternion3 operator*(const Quaternion3& q) const;
	constexpr Quaternion3 operator/(const Quaternion3& q) const;

	constexpr Matrix3 ToMatrix3() const;
	constexpr Matrix4 ToMatrix4() const;

	constexpr Matrix4 RotationMatrix(Vector3 center) const;

	constexpr Vector3 Euler() const;

	constexpr Quaternion3 Slerp(const Quaternion3& q, F32 t) const;
	constexpr Quaternion3& Slerped(const Quaternion3& q, F32 t);

	constexpr F32 Dot(const Quaternion3& q) const;
	constexpr F32 Dot(const Vector3& q) const;
	constexpr F32 SqrNormal() const;
	constexpr F32 Normal() const;
	constexpr Quaternion3 Cross(const Quaternion3& q) const;
	constexpr Quaternion3 Normalize() const;
	constexpr Quaternion3& Normalized();
	constexpr Quaternion3 Conjugate() const;
	constexpr Quaternion3& Conjugated();
	constexpr Quaternion3 Inverse() const;
	constexpr Quaternion3& Inversed();

	F32& operator[] (U8 i);
	const F32& operator[] (U8 i) const;

	F32* Data();
	const F32* Data() const;

	constexpr explicit operator Quaternion2() const;
	constexpr explicit operator Vector3() const;

public:
	F32 x, y, z, w;

	static const Quaternion3 Identity;
};

#pragma region Vector2
inline constexpr Vector2::Vector2() : x(0.0f), y(0.0f) {}
inline constexpr Vector2::Vector2(F32 f) : x(f), y(f) {}
inline constexpr Vector2::Vector2(F32 x, F32 y) : x(x), y(y) {}
inline constexpr Vector2::Vector2(const Vector2& v) : x(v.x), y(v.y) {}
inline constexpr Vector2::Vector2(Vector2&& v) noexcept : x(v.x), y(v.y) {}

inline constexpr Vector2& Vector2::operator=(F32 f) { x = f; y = f; return *this; }
inline constexpr Vector2& Vector2::operator=(const Vector2& v) { x = v.x; y = v.y; return *this; }
inline constexpr Vector2& Vector2::operator=(Vector2&& v) noexcept { x = v.x; y = v.y; return *this; }

inline constexpr Vector2& Vector2::operator+=(F32 f) { x += f; y += f; return *this; }
inline constexpr Vector2& Vector2::operator-=(F32 f) { x -= f; y -= f; return *this; }
inline constexpr Vector2& Vector2::operator*=(F32 f) { x *= f; y *= f; return *this; }
inline constexpr Vector2& Vector2::operator/=(F32 f) { x /= f; y /= f; return *this; }
inline constexpr Vector2& Vector2::operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); return *this; }
inline constexpr Vector2& Vector2::operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
inline constexpr Vector2& Vector2::operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
inline constexpr Vector2& Vector2::operator*=(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
inline constexpr Vector2& Vector2::operator/=(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
inline constexpr Vector2& Vector2::operator%=(const Vector2& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); return *this; }
inline constexpr Vector2& Vector2::operator*=(const Quaternion2& q) { F32 temp = q.y * x - q.x * y; y = q.x * x + q.y * y; x = temp; return *this; }
inline constexpr Vector2& Vector2::operator^=(const Quaternion2& q) { F32 temp = q.y * x + q.x * y; y = q.y * y - q.x * x; x = temp; return *this; }

inline constexpr Vector2 Vector2::operator+(F32 f) const { return { x + f, y + f }; }
inline constexpr Vector2 Vector2::operator-(F32 f) const { return { x - f, y - f }; }
inline constexpr Vector2 Vector2::operator*(F32 f) const { return { x * f, y * f }; }
inline constexpr Vector2 Vector2::operator/(F32 f) const { return { x / f, y / f }; }
inline constexpr Vector2 Vector2::operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f) }; }
inline constexpr Vector2 Vector2::operator+(const Vector2& v) const { return { x + v.x, y + v.y }; }
inline constexpr Vector2 Vector2::operator-(const Vector2& v) const { return { x - v.x, y - v.y }; }
inline constexpr Vector2 Vector2::operator*(const Vector2& v) const { return { x * v.x, y * v.y }; }
inline constexpr Vector2 Vector2::operator/(const Vector2& v) const { return { x / v.x, y / v.y }; }
inline constexpr Vector2 Vector2::operator%(const Vector2& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y) }; }
inline constexpr Vector2 Vector2::operator*(const Quaternion2& q) const { return Vector2{ q.y * x - q.x * y, q.x * x + q.y * y }; }
inline constexpr Vector2 Vector2::operator^(const Quaternion2& q) const { return Vector2{ q.y * x + q.x * y, q.y * y - q.x * x }; }

inline constexpr bool Vector2::operator==(const Vector2& v) const { return Math::IsZero(x - v.x) && Math::IsZero(y - v.y); }
inline constexpr bool Vector2::operator!=(const Vector2& v) const { return !Math::IsZero(x - v.x) || !Math::IsZero(y - v.y); }
inline constexpr bool Vector2::operator>(const Vector2& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector2::operator<(const Vector2& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector2::operator>=(const Vector2& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector2::operator<=(const Vector2& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector2::IsZero() const { return Math::IsZero(x) && Math::IsZero(y); }

inline constexpr Vector2 Vector2::operator-() const { return { -x, -y }; }
inline constexpr Vector2 Vector2::operator~() const { return { -x, -y }; }
inline constexpr Vector2 Vector2::operator!() const { return { -x, -y }; }

inline constexpr F32 Vector2::SqrMagnitude() const { return x * x + y * y; }
inline constexpr F32 Vector2::Magnitude() const { return Math::Sqrt(x * x + y * y); }
inline constexpr F32 Vector2::Dot(const Vector2& v) const { return x * v.x + y * v.y; }
inline constexpr F32 Vector2::Dot(F32 vx, F32 vy) const { return x * vx + y * vy; }
inline constexpr Vector2& Vector2::Normalize() { Vector2 v = Normalized(); x = v.x; y = v.y; return *this; }
inline constexpr Vector2 Vector2::Normalized() const { return IsZero() ? Vector2{ 0.0f } : (*this) / Magnitude(); }
inline constexpr Vector2& Vector2::Normalize(F32& magnitude) { Vector2 v = Normalized(magnitude); x = v.x; y = v.y; return *this; }
inline constexpr Vector2 Vector2::Normalized(F32& magnitude) { magnitude = Magnitude(); return IsZero() ? Vector2{ 0.0f } : (*this) / magnitude; }
inline constexpr F32 Vector2::AngleBetween(const Vector2& v) const { return Math::ACos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
inline constexpr Vector2 Vector2::Projection(const Vector2& v) const { return v * (Dot(v) / v.Dot(v)); }
inline constexpr Vector2 Vector2::OrthoProjection(const Vector2& v) const { return *this - Projection(v); }
inline constexpr F32 Vector2::Cross(const Vector2& v) const { return x * v.y - y * v.x; }
inline constexpr Vector2 Vector2::Cross(F32 f) const { return { y * f, x * -f }; }
inline constexpr Vector2 Vector2::CrossInv(F32 f) const { return { -f * y, f * x }; }
inline constexpr Vector2 Vector2::PerpendicularLeft() const { return { -y, x }; }
inline constexpr Vector2 Vector2::PerpendicularRight() const { return { y, -x }; }
inline constexpr Vector2 Vector2::Normal(const Vector2& v) const { return Vector2(-(v.y - y), v.x - x).Normalized(); }
inline constexpr Vector2& Vector2::Rotate(const Vector2& center, F32 angle)
{
	F32 cos = Math::Cos(angle * (F32)DegToRad);
	F32 sin = Math::Sin(angle * (F32)DegToRad);
	F32 temp = cos * (x - center.x) - sin * (y - center.y) + center.x;
	y = sin * (x - center.x) + cos * (y - center.y) + center.y;
	x = temp;

	return *this;
}
inline constexpr Vector2 Vector2::Rotated(const Vector2& center, F32 angle) const
{
	F32 cos = Math::Cos(angle * (F32)DegToRad);
	F32 sin = Math::Sin(angle * (F32)DegToRad);
	return Vector2{ cos * (x - center.x) - sin * (y - center.y) + center.x,
	sin * (x - center.x) + cos * (y - center.y) + center.y };
}
inline constexpr Vector2& Vector2::Rotate(F32 angle)
{
	F32 cos = Math::Cos(angle * (F32)DegToRad);
	F32 sin = Math::Sin(angle * (F32)DegToRad);
	y = cos * x - sin * y;
	x = sin * x + cos * y;

	return *this;
}
inline constexpr Vector2 Vector2::Rotated(F32 angle) const
{
	F32 cos = (F32)Math::Cos(angle * DegToRad);
	F32 sin = (F32)Math::Sin(angle * DegToRad);
	return { cos * x - sin * y, sin * x + cos * y };
}
inline constexpr Vector2& Vector2::Rotate(const Vector2& center, const Quaternion2& quat)
{
	F32 temp = quat.y * (x - center.x) - quat.x * (y - center.y) + center.x;
	y = quat.x * (x - center.x) + quat.y * (y - center.y) + center.y;
	x = temp;
	return *this;
}
inline constexpr Vector2 Vector2::Rotated(const Vector2& center, const Quaternion2& quat) const
{
	return Vector2{ quat.y * (x - center.x) - quat.x * (y - center.y) + center.x, quat.x * (x - center.x) + quat.y * (y - center.y) + center.y };
}
inline constexpr Vector2& Vector2::Rotate(const Quaternion2& quat) { y = quat.y * x - quat.x * y; x = quat.x * x + quat.y * y; return *this; }
inline constexpr Vector2 Vector2::Rotated(const Quaternion2& quat) const { return { quat.y * x - quat.x * y, quat.x * x + quat.y * y }; }
inline constexpr Vector2& Vector2::Clamp(const Vector2& min, const Vector2& max) { x = Math::Clamp(x, min.x, max.x); y = Math::Clamp(y, min.y, max.y); return *this; }
inline constexpr Vector2 Vector2::Clamped(const Vector2& min, const Vector2& max) const { return { Math::Clamp(x, min.x, max.x), Math::Clamp(y, min.y, max.y) }; }
inline constexpr Vector2& Vector2::SetClosest(const Vector2& min, const Vector2& max) { x = Math::Closest(x, min.x, max.x); y = Math::Closest(y, min.y, max.y); return *this; }
inline constexpr Vector2 Vector2::Closest(const Vector2& min, const Vector2& max) const { return { Math::Closest(x, min.x, max.x), Math::Closest(y, min.y, max.y) }; }

inline constexpr Vector2 Vector2::Min(const Vector2& other) { return { Math::Min(x, other.x), Math::Min(y, other.y) }; }
inline constexpr Vector2 Vector2::Max(const Vector2& other) { return { Math::Max(x, other.x), Math::Max(y, other.y) }; }

inline constexpr bool Vector2::Valid() { return !(Math::IsValid(x) || Math::IsValid(y)); }

inline F32& Vector2::operator[] (U64 i) { return (&x)[i]; }
inline const F32& Vector2::operator[] (U64 i) const { return (&x)[i]; }

inline F32* Vector2::Data() { return &x; }
inline const F32* Vector2::Data() const { return &x; }

inline constexpr Vector2 Vector2::xx() const { return { x, x }; }
inline constexpr Vector2 Vector2::xy() const { return { x, y }; }
inline constexpr Vector2 Vector2::yx() const { return { y, x }; }
inline constexpr Vector2 Vector2::yy() const { return { y, y }; }

inline constexpr Vector2::operator Vector3() const { return { x, y, 0.0f }; }
inline constexpr Vector2::operator Vector4() const { return { x, y, 0.0f, 0.0f }; }
inline constexpr Vector2::operator Vector2Int() const { return { (I32)x, (I32)y }; }
inline constexpr Vector2::operator Vector3Int() const { return { (I32)x, (I32)y, 0 }; }
inline constexpr Vector2::operator Vector4Int() const { return { (I32)x, (I32)y, 0, 0 }; }
#pragma endregion

#pragma region Vector3
inline constexpr Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
inline constexpr Vector3::Vector3(F32 f) : x(f), y(f), z(f) {}
inline constexpr Vector3::Vector3(F32 x, F32 y, F32 z) : x(x), y(y), z(z) {}
inline constexpr Vector3::Vector3(const Vector2& v) : x(v.x), y(v.y), z(0.0f) {}
inline constexpr Vector3::Vector3(const Vector2& v, F32 z) : x(v.x), y(v.y), z(z) {}
inline constexpr Vector3::Vector3(F32 x, const Vector2& v) : x(x), y(v.x), z(v.y) {}
inline constexpr Vector3::Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}
inline constexpr Vector3::Vector3(Vector3&& v) noexcept : x(v.x), y(v.y), z(v.z) {}

inline constexpr Vector3& Vector3::operator=(F32 f) { x = f; y = f; z = f; return *this; }
inline constexpr Vector3& Vector3::operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z; return *this; }
inline constexpr Vector3& Vector3::operator=(Vector3&& v) noexcept { x = v.x; y = v.y; z = v.z; return *this; }

inline constexpr Vector3& Vector3::operator+=(F32 f) { x += f; y += f; z += f; return *this; }
inline constexpr Vector3& Vector3::operator-=(F32 f) { x -= f; y -= f; z -= f; return *this; }
inline constexpr Vector3& Vector3::operator*=(F32 f) { x *= f; y *= f; z *= f; return *this; }
inline constexpr Vector3& Vector3::operator/=(F32 f) { x /= f; y /= f; z /= f; return *this; }
inline constexpr Vector3& Vector3::operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); return *this; }
inline constexpr Vector3& Vector3::operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
inline constexpr Vector3& Vector3::operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
inline constexpr Vector3& Vector3::operator*=(const Vector3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
inline constexpr Vector3& Vector3::operator/=(const Vector3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
inline constexpr Vector3& Vector3::operator%=(const Vector3& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); z = Math::Mod(z, v.z); return *this; }
inline constexpr Vector3& Vector3::operator*=(const Quaternion3& q)
{
	F32 xx = q.x + q.x;
	F32 yy = q.y + q.y;
	F32 zz = q.z + q.z;
	F32 xxw = xx * q.w;
	F32 yyw = yy * q.w;
	F32 zzw = zz * q.w;
	F32 xxx = xx * q.x;
	F32 yyx = yy * x;
	F32 zzx = zz * q.x;
	F32 yyy = yy * q.y;
	F32 zzy = zz * q.y;
	F32 zzz = zz * z;
	x = ((x * ((1.0f - yyy) - zzz)) + (y * (yyx - zzw))) + (z * (zzx + yyw));
	y = ((x * (yyx + zzw)) + (y * ((1.0f - xxx) - zzz))) + (z * (zzy - xxw));
	z = ((x * (zzx - yyw)) + (y * (zzy + xxw))) + (z * ((1.0f - xxx) - yyy));

	return *this;
}

inline constexpr Vector3 Vector3::operator+(F32 f) const { return { x + f, y + f, z + f }; }
inline constexpr Vector3 Vector3::operator-(F32 f) const { return { x - f, y - f, z - f }; }
inline constexpr Vector3 Vector3::operator*(F32 f) const { return { x * f, y * f, z * f }; }
inline constexpr Vector3 Vector3::operator/(F32 f) const { return { x / f, y / f, z / f }; }
inline constexpr Vector3 Vector3::operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f) }; }
inline constexpr Vector3 Vector3::operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
inline constexpr Vector3 Vector3::operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
inline constexpr Vector3 Vector3::operator*(const Vector3& v) const { return { x * v.x, y * v.y, z * v.z }; }
inline constexpr Vector3 Vector3::operator/(const Vector3& v) const { return { x / v.x, y / v.y, z / v.z }; }
inline constexpr Vector3 Vector3::operator%(const Vector3& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y), Math::Mod(z, v.z) }; }
inline constexpr Vector3 Vector3::operator*(const Quaternion3& q) const
{
	F32 xx = q.x + q.x;
	F32 yy = q.y + q.y;
	F32 zz = q.z + q.z;
	F32 xxw = xx * q.w;
	F32 yyw = yy * q.w;
	F32 zzw = zz * q.w;
	F32 xxx = xx * q.x;
	F32 yyx = yy * x;
	F32 zzx = zz * q.x;
	F32 yyy = yy * q.y;
	F32 zzy = zz * q.y;
	F32 zzz = zz * z;

	return {
		((x * ((1.0f - yyy) - zzz)) + (y * (yyx - zzw))) + (z * (zzx + yyw)),
		((x * (yyx + zzw)) + (y * ((1.0f - xxx) - zzz))) + (z * (zzy - xxw)),
		((x * (zzx - yyw)) + (y * (zzy + xxw))) + (z * ((1.0f - xxx) - yyy))
	};
}

inline constexpr bool Vector3::operator==(const Vector3& v) const { return Math::IsZero(x - v.x) && Math::IsZero(y - v.y) && Math::IsZero(z - v.z); }
inline constexpr bool Vector3::operator!=(const Vector3& v) const { return !Math::IsZero(x - v.x) || !Math::IsZero(y - v.y) || !Math::IsZero(z - v.z); }
inline constexpr bool Vector3::operator>(const Vector3& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector3::operator<(const Vector3& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector3::operator>=(const Vector3& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector3::operator<=(const Vector3& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector3::IsZero() const { return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z); }

inline constexpr Vector3 Vector3::operator-() const { return { -x, -y, -z }; }
inline constexpr Vector3 Vector3::operator~() const { return { -x, -y, -z }; }
inline constexpr Vector3 Vector3::operator!() const { return { -x, -y, -z }; }

inline constexpr F32 Vector3::SqrMagnitude() const { return x * x + y * y + z * z; }
inline constexpr F32 Vector3::Magnitude() const { return Math::Sqrt(x * x + y * y + z * z); }
inline constexpr F32 Vector3::Dot() const { return x * x + y * y + z * z; }
inline constexpr F32 Vector3::Dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
inline constexpr F32 Vector3::Dot(F32 vx, F32 vy, F32 vz) const { return x * vx + y * vy + z * vz; }
inline constexpr Vector3& Vector3::Normalize() { *this = Normalized(); return *this; }
inline constexpr Vector3 Vector3::Normalized() const { return IsZero() ? Vector3{ 0.0f } : (*this) / Magnitude(); }
inline constexpr Vector3 Vector3::Projection(const Vector3& v) const { return v * (Dot(v) / v.Dot(v)); }
inline constexpr Vector3 Vector3::OrthoProjection(const Vector3& v) const { return *this - Projection(v); }
inline constexpr Vector3 Vector3::Cross(const Vector3& v) const { return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x }; }
inline constexpr Vector3 Vector3::Normal(const Vector3& v) const { return Cross(v).Normalized(); }
inline constexpr Vector3& Vector3::Rotate(const Quaternion3& quat)
{
	Vector3 q = (Vector3)quat;
	*this = q * quat.Dot(*this) * 2.0f + *this * (quat.w * quat.w - q.Dot()) + q.Cross(*this) * quat.w * 2.0f;
	return *this;
}
inline constexpr Vector3 Vector3::Rotated(const Quaternion3& quat) const
{
	Vector3 q = (Vector3)quat;
	return q * quat.Dot(*this) * 2.0f + *this * (quat.w * quat.w - q.Dot()) + q.Cross(*this) * quat.w * 2.0f;
}
inline constexpr Vector3& Vector3::Rotate(const Vector3& center, const Quaternion3& quat)
{
	Vector3 q = (Vector3)quat;
	Vector3 v = *this - center;

	*this = (q * quat.Dot(v) * 2.0f + v * (quat.w * quat.w - q.Dot()) + q.Cross(v) * quat.w * 2.0f) + center;
	return *this;
}
inline constexpr Vector3 Vector3::Rotated(const Vector3& center, const Quaternion3& quat) const
{
	Vector3 q = (Vector3)quat;
	Vector3 v = *this - center;

	return (q * quat.Dot(v) * 2.0f + v * (quat.w * quat.w - q.Dot()) + q.Cross(v) * quat.w * 2.0f) + center;
}
inline constexpr Vector3& Vector3::Clamp(const Vector3& min, const Vector3& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	return *this;
}
inline constexpr Vector3 Vector3::Clamped(const Vector3& min, const Vector3& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z)
	};
}
inline constexpr Vector3& Vector3::SetClosest(const Vector3& min, const Vector3& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	return *this;
}
inline constexpr Vector3 Vector3::Closest(const Vector3& min, const Vector3& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z)
	};
}

inline constexpr Vector3 Vector3::Min(const Vector3& other) { return { Math::Min(x, other.x), Math::Min(y, other.y), Math::Min(z, other.z) }; }
inline constexpr Vector3 Vector3::Max(const Vector3& other) { return { Math::Max(x, other.x), Math::Max(y, other.y), Math::Max(z, other.z) }; }

inline constexpr bool Vector3::Valid() { return !(Math::IsValid(x) || Math::IsValid(y) || Math::IsValid(z)); }

inline F32& Vector3::operator[] (U64 i) { return (&x)[i]; }
inline const F32& Vector3::operator[] (U64 i) const { return (&x)[i]; }

inline F32* Vector3::Data() { return &x; }
inline const F32* Vector3::Data() const { return &x; }

inline constexpr Vector2 Vector3::xx() const { return { x, x }; }
inline constexpr Vector2 Vector3::xy() const { return { x, y }; }
inline constexpr Vector2 Vector3::xz() const { return { x, z }; }
inline constexpr Vector2 Vector3::yx() const { return { y, x }; }
inline constexpr Vector2 Vector3::yy() const { return { y, y }; }
inline constexpr Vector2 Vector3::yz() const { return { y, z }; }
inline constexpr Vector2 Vector3::zx() const { return { z, x }; }
inline constexpr Vector2 Vector3::zy() const { return { z, y }; }
inline constexpr Vector2 Vector3::zz() const { return { z, z }; }
inline constexpr Vector3 Vector3::xxx() const { return { x, x, x }; }
inline constexpr Vector3 Vector3::xxy() const { return { x, x, y }; }
inline constexpr Vector3 Vector3::xxz() const { return { x, x, z }; }
inline constexpr Vector3 Vector3::xyx() const { return { x, y, x }; }
inline constexpr Vector3 Vector3::xyy() const { return { x, y, y }; }
inline constexpr Vector3 Vector3::xyz() const { return { x, y, z }; }
inline constexpr Vector3 Vector3::xzx() const { return { x, z, x }; }
inline constexpr Vector3 Vector3::xzy() const { return { x, z, y }; }
inline constexpr Vector3 Vector3::xzz() const { return { x, z, z }; }
inline constexpr Vector3 Vector3::yxx() const { return { y, x, x }; }
inline constexpr Vector3 Vector3::yxy() const { return { y, x, y }; }
inline constexpr Vector3 Vector3::yxz() const { return { y, x, z }; }
inline constexpr Vector3 Vector3::yyx() const { return { y, y, x }; }
inline constexpr Vector3 Vector3::yyy() const { return { y, y, y }; }
inline constexpr Vector3 Vector3::yyz() const { return { y, y, z }; }
inline constexpr Vector3 Vector3::yzx() const { return { y, z, x }; }
inline constexpr Vector3 Vector3::yzy() const { return { y, z, y }; }
inline constexpr Vector3 Vector3::yzz() const { return { y, z, z }; }
inline constexpr Vector3 Vector3::zxx() const { return { z, x, x }; }
inline constexpr Vector3 Vector3::zxy() const { return { z, x, y }; }
inline constexpr Vector3 Vector3::zxz() const { return { z, x, z }; }
inline constexpr Vector3 Vector3::zyx() const { return { z, y, x }; }
inline constexpr Vector3 Vector3::zyy() const { return { z, y, y }; }
inline constexpr Vector3 Vector3::zyz() const { return { z, y, z }; }
inline constexpr Vector3 Vector3::zzx() const { return { z, z, x }; }
inline constexpr Vector3 Vector3::zzy() const { return { z, z, y }; }
inline constexpr Vector3 Vector3::zzz() const { return { z, z, z }; }

inline constexpr Vector3::operator Vector2() const { return { x, y }; }
inline constexpr Vector3::operator Vector4() const { return { x, y, z, 0.0f }; }
inline constexpr Vector3::operator Vector2Int() const { return { (I32)x, (I32)y }; }
inline constexpr Vector3::operator Vector3Int() const { return { (I32)x, (I32)y, (I32)z }; }
inline constexpr Vector3::operator Vector4Int() const { return { (I32)x, (I32)y, (I32)z, 0 }; }
#pragma endregion

#pragma region Vector4
inline constexpr Vector4::Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
inline constexpr Vector4::Vector4(F32 f) : x(f), y(f), z(f), w(f) {}
inline constexpr Vector4::Vector4(F32 x, F32 y, F32 z, F32 w) : x(x), y(y), z(z), w(w) {}
inline constexpr Vector4::Vector4(const Vector2& a, const Vector2& b) : x(a.x), y(a.y), z(b.x), w(b.y) {}
inline constexpr Vector4::Vector4(F32 x, const Vector2& a, F32 w) : x(x), y(a.x), z(a.y), w(w) {}
inline constexpr Vector4::Vector4(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
inline constexpr Vector4::Vector4(Vector4&& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) {}

inline constexpr Vector4& Vector4::operator=(F32 f) { x = f; y = f; z = f; w = f; return *this; }
inline constexpr Vector4& Vector4::operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
inline constexpr Vector4& Vector4::operator=(Vector4&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

inline constexpr Vector4& Vector4::operator+=(F32 f) { x += f; y += f; z += f; w += f; return *this; }
inline constexpr Vector4& Vector4::operator-=(F32 f) { x -= f; y -= f; z -= f; w -= f; return *this; }
inline constexpr Vector4& Vector4::operator*=(F32 f) { x *= f; y *= f; z *= f; w *= f; return *this; }
inline constexpr Vector4& Vector4::operator/=(F32 f) { x /= f; y /= f; z /= f; w /= f; return *this; }
inline constexpr Vector4& Vector4::operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); w = Math::Mod(w, f); return *this; }
inline constexpr Vector4& Vector4::operator+=(const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
inline constexpr Vector4& Vector4::operator-=(const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
inline constexpr Vector4& Vector4::operator*=(const Vector4& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
inline constexpr Vector4& Vector4::operator/=(const Vector4& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
inline constexpr Vector4& Vector4::operator%=(const Vector4& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); z = Math::Mod(z, v.z); w = Math::Mod(w, v.w); return *this; }

inline constexpr Vector4 Vector4::operator+(F32 f) const { return { x + f, y + f, z + f, w + f }; }
inline constexpr Vector4 Vector4::operator-(F32 f) const { return { x - f, y - f, z - f, w - f }; }
inline constexpr Vector4 Vector4::operator*(F32 f) const { return { x * f, y * f, z * f, w * f }; }
inline constexpr Vector4 Vector4::operator/(F32 f) const { return { x / f, y / f, z / f, w / f }; }
inline constexpr Vector4 Vector4::operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f), Math::Mod(w, f) }; }
inline constexpr Vector4 Vector4::operator+(const Vector4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
inline constexpr Vector4 Vector4::operator-(const Vector4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
inline constexpr Vector4 Vector4::operator*(const Vector4& v) const { return { x * v.x, y * v.y, z * v.z, w * v.w }; }
inline constexpr Vector4 Vector4::operator/(const Vector4& v) const { return { x / v.x, y / v.y, z / v.z, w / v.w }; }
inline constexpr Vector4 Vector4::operator%(const Vector4& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y), Math::Mod(z, v.z), Math::Mod(w, v.w) }; }

inline constexpr bool Vector4::operator==(const Vector4& v) const { return Math::IsZero(x - v.x) && Math::IsZero(y - v.y) && Math::IsZero(z - v.z) && Math::IsZero(w - v.w); }
inline constexpr bool Vector4::operator!=(const Vector4& v) const { return !Math::IsZero(x - v.x) || !Math::IsZero(y - v.y) || !Math::IsZero(z - v.z) || !Math::IsZero(w - v.w); }
inline constexpr bool Vector4::operator>(const Vector4& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector4::operator<(const Vector4& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector4::operator>=(const Vector4& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector4::operator<=(const Vector4& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector4::IsZero() const { return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z) && Math::IsZero(w); }

inline constexpr Vector4 Vector4::operator-() const { return { -x, -y, -z, -w }; }
inline constexpr Vector4 Vector4::operator~() const { return { -x, -y, -z, -w }; }
inline constexpr Vector4 Vector4::operator!() const { return { -x, -y, -z, -w }; }

inline constexpr F32 Vector4::SqrMagnitude() const { return x * x + y * y + z * z + w * w; }
inline constexpr F32 Vector4::Magnitude() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
inline constexpr F32 Vector4::Dot(const Vector4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
inline constexpr F32 Vector4::Dot(F32 vx, F32 vy, F32 vz, F32 vw) const { return x * vx + y * vy + z * vz + w * vw; }
inline constexpr Vector4& Vector4::Normalize() { Vector4 v = Normalized(); x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
inline constexpr Vector4 Vector4::Normalized() const { return IsZero() ? Vector4{ 0.0f } : (Vector3(this->x, this->y, this->z) / Magnitude(), this->z); }
inline constexpr Vector4 Vector4::Projection(const Vector4& v) const { return v * (Dot(v) / v.Dot(v)); }
inline constexpr Vector4 Vector4::OrthoProjection(const Vector4& v) const { return *this - Projection(v); }
inline constexpr Vector4& Vector4::Clamp(const Vector4& min, const Vector4& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	w = Math::Clamp(w, min.w, max.w);
	return *this;
}
inline constexpr Vector4 Vector4::Clamped(const Vector4& min, const Vector4& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z),
		Math::Clamp(w, min.w, max.w)
	};
}
inline constexpr Vector4& Vector4::SetClosest(const Vector4& min, const Vector4& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	w = Math::Closest(w, min.w, max.w);
	return *this;
}
inline constexpr Vector4 Vector4::Closest(const Vector4& min, const Vector4& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z),
		Math::Closest(w, min.w, max.w)
	};
}

inline constexpr Vector4 Vector4::Min(const Vector4& other) { return { Math::Min(x, other.x), Math::Min(y, other.y), Math::Min(z, other.z), Math::Min(w, other.w) }; }
inline constexpr Vector4 Vector4::Max(const Vector4& other) { return { Math::Max(x, other.x), Math::Max(y, other.y), Math::Max(z, other.z), Math::Max(w, other.w) }; }

inline constexpr bool Vector4::Valid() { return !(Math::IsValid(x) || Math::IsValid(y) || Math::IsValid(z) || Math::IsValid(w)); }

inline F32& Vector4::operator[] (U64 i) { return (&x)[i]; }
inline const F32& Vector4::operator[] (U64 i) const { return (&x)[i]; }

inline F32* Vector4::Data() { return &x; }
inline const F32* Vector4::Data() const { return &x; }

#pragma region SwizzleFunctions
inline constexpr Vector2 Vector4::xx() const { return { x, x }; }
inline constexpr Vector2 Vector4::xy() const { return { x, y }; }
inline constexpr Vector2 Vector4::xz() const { return { x, z }; }
inline constexpr Vector2 Vector4::xw() const { return { x, w }; }
inline constexpr Vector2 Vector4::yx() const { return { y, x }; }
inline constexpr Vector2 Vector4::yy() const { return { y, y }; }
inline constexpr Vector2 Vector4::yz() const { return { y, z }; }
inline constexpr Vector2 Vector4::yw() const { return { y, w }; }
inline constexpr Vector2 Vector4::zx() const { return { z, x }; }
inline constexpr Vector2 Vector4::zy() const { return { z, y }; }
inline constexpr Vector2 Vector4::zz() const { return { z, z }; }
inline constexpr Vector2 Vector4::zw() const { return { z, w }; }
inline constexpr Vector2 Vector4::wx() const { return { w, x }; }
inline constexpr Vector2 Vector4::wy() const { return { w, y }; }
inline constexpr Vector2 Vector4::wz() const { return { w, z }; }
inline constexpr Vector2 Vector4::ww() const { return { w, w }; }
inline constexpr Vector3 Vector4::xxx() const { return { x, x, x }; }
inline constexpr Vector3 Vector4::xxy() const { return { x, x, y }; }
inline constexpr Vector3 Vector4::xxz() const { return { x, x, z }; }
inline constexpr Vector3 Vector4::xxw() const { return { x, x, w }; }
inline constexpr Vector3 Vector4::xyx() const { return { x, y, x }; }
inline constexpr Vector3 Vector4::xyy() const { return { x, y, y }; }
inline constexpr Vector3 Vector4::xyz() const { return { x, y, z }; }
inline constexpr Vector3 Vector4::xyw() const { return { x, y, w }; }
inline constexpr Vector3 Vector4::xzx() const { return { x, z, x }; }
inline constexpr Vector3 Vector4::xzy() const { return { x, z, y }; }
inline constexpr Vector3 Vector4::xzz() const { return { x, z, z }; }
inline constexpr Vector3 Vector4::xzw() const { return { x, z, w }; }
inline constexpr Vector3 Vector4::xwx() const { return { x, w, x }; }
inline constexpr Vector3 Vector4::xwy() const { return { x, w, y }; }
inline constexpr Vector3 Vector4::xwz() const { return { x, w, z }; }
inline constexpr Vector3 Vector4::xww() const { return { x, w, w }; }
inline constexpr Vector3 Vector4::yxx() const { return { y, x, x }; }
inline constexpr Vector3 Vector4::yxy() const { return { y, x, y }; }
inline constexpr Vector3 Vector4::yxz() const { return { y, x, z }; }
inline constexpr Vector3 Vector4::yxw() const { return { y, x, w }; }
inline constexpr Vector3 Vector4::yyx() const { return { y, y, x }; }
inline constexpr Vector3 Vector4::yyy() const { return { y, y, y }; }
inline constexpr Vector3 Vector4::yyz() const { return { y, y, z }; }
inline constexpr Vector3 Vector4::yyw() const { return { y, y, w }; }
inline constexpr Vector3 Vector4::yzx() const { return { y, z, x }; }
inline constexpr Vector3 Vector4::yzy() const { return { y, z, y }; }
inline constexpr Vector3 Vector4::yzz() const { return { y, z, z }; }
inline constexpr Vector3 Vector4::yzw() const { return { y, z, w }; }
inline constexpr Vector3 Vector4::ywx() const { return { y, w, x }; }
inline constexpr Vector3 Vector4::ywy() const { return { y, w, y }; }
inline constexpr Vector3 Vector4::ywz() const { return { y, w, z }; }
inline constexpr Vector3 Vector4::yww() const { return { y, w, w }; }
inline constexpr Vector3 Vector4::zxx() const { return { z, x, x }; }
inline constexpr Vector3 Vector4::zxy() const { return { z, x, y }; }
inline constexpr Vector3 Vector4::zxz() const { return { z, x, z }; }
inline constexpr Vector3 Vector4::zxw() const { return { z, x, w }; }
inline constexpr Vector3 Vector4::zyx() const { return { z, y, x }; }
inline constexpr Vector3 Vector4::zyy() const { return { z, y, y }; }
inline constexpr Vector3 Vector4::zyz() const { return { z, y, z }; }
inline constexpr Vector3 Vector4::zyw() const { return { z, y, w }; }
inline constexpr Vector3 Vector4::zzx() const { return { z, z, x }; }
inline constexpr Vector3 Vector4::zzy() const { return { z, z, y }; }
inline constexpr Vector3 Vector4::zzz() const { return { z, z, z }; }
inline constexpr Vector3 Vector4::zzw() const { return { z, z, w }; }
inline constexpr Vector3 Vector4::zwx() const { return { z, w, x }; }
inline constexpr Vector3 Vector4::zwy() const { return { z, w, y }; }
inline constexpr Vector3 Vector4::zwz() const { return { z, w, z }; }
inline constexpr Vector3 Vector4::zww() const { return { z, w, w }; }
inline constexpr Vector3 Vector4::wxx() const { return { w, x, x }; }
inline constexpr Vector3 Vector4::wxy() const { return { w, x, y }; }
inline constexpr Vector3 Vector4::wxz() const { return { w, x, z }; }
inline constexpr Vector3 Vector4::wxw() const { return { w, x, w }; }
inline constexpr Vector3 Vector4::wyx() const { return { w, y, x }; }
inline constexpr Vector3 Vector4::wyy() const { return { w, y, y }; }
inline constexpr Vector3 Vector4::wyz() const { return { w, y, z }; }
inline constexpr Vector3 Vector4::wyw() const { return { w, y, w }; }
inline constexpr Vector3 Vector4::wzx() const { return { w, z, x }; }
inline constexpr Vector3 Vector4::wzy() const { return { w, z, y }; }
inline constexpr Vector3 Vector4::wzz() const { return { w, z, z }; }
inline constexpr Vector3 Vector4::wzw() const { return { w, z, w }; }
inline constexpr Vector3 Vector4::wwx() const { return { w, w, x }; }
inline constexpr Vector3 Vector4::wwy() const { return { w, w, y }; }
inline constexpr Vector3 Vector4::wwz() const { return { w, w, z }; }
inline constexpr Vector3 Vector4::www() const { return { w, w, w }; }
inline constexpr Vector4 Vector4::xxxx() const { return { x, x, x, x }; }
inline constexpr Vector4 Vector4::xxxy() const { return { x, x, x, y }; }
inline constexpr Vector4 Vector4::xxxz() const { return { x, x, x, z }; }
inline constexpr Vector4 Vector4::xxxw() const { return { x, x, x, w }; }
inline constexpr Vector4 Vector4::xxyx() const { return { x, x, y, x }; }
inline constexpr Vector4 Vector4::xxyy() const { return { x, x, y, y }; }
inline constexpr Vector4 Vector4::xxyz() const { return { x, x, y, z }; }
inline constexpr Vector4 Vector4::xxyw() const { return { x, x, y, w }; }
inline constexpr Vector4 Vector4::xxzx() const { return { x, x, z, x }; }
inline constexpr Vector4 Vector4::xxzy() const { return { x, x, z, y }; }
inline constexpr Vector4 Vector4::xxzz() const { return { x, x, z, z }; }
inline constexpr Vector4 Vector4::xxzw() const { return { x, x, z, w }; }
inline constexpr Vector4 Vector4::xxwx() const { return { x, x, w, x }; }
inline constexpr Vector4 Vector4::xxwy() const { return { x, x, w, y }; }
inline constexpr Vector4 Vector4::xxwz() const { return { x, x, w, z }; }
inline constexpr Vector4 Vector4::xxww() const { return { x, x, w, w }; }
inline constexpr Vector4 Vector4::xyxx() const { return { x, y, x, x }; }
inline constexpr Vector4 Vector4::xyxy() const { return { x, y, x, y }; }
inline constexpr Vector4 Vector4::xyxz() const { return { x, y, x, z }; }
inline constexpr Vector4 Vector4::xyxw() const { return { x, y, x, w }; }
inline constexpr Vector4 Vector4::xyyx() const { return { x, y, y, x }; }
inline constexpr Vector4 Vector4::xyyy() const { return { x, y, y, y }; }
inline constexpr Vector4 Vector4::xyyz() const { return { x, y, y, z }; }
inline constexpr Vector4 Vector4::xyyw() const { return { x, y, y, w }; }
inline constexpr Vector4 Vector4::xyzx() const { return { x, y, z, x }; }
inline constexpr Vector4 Vector4::xyzy() const { return { x, y, z, y }; }
inline constexpr Vector4 Vector4::xyzz() const { return { x, y, z, z }; }
inline constexpr Vector4 Vector4::xyzw() const { return { x, y, z, w }; }
inline constexpr Vector4 Vector4::xywx() const { return { x, y, w, x }; }
inline constexpr Vector4 Vector4::xywy() const { return { x, y, w, y }; }
inline constexpr Vector4 Vector4::xywz() const { return { x, y, w, z }; }
inline constexpr Vector4 Vector4::xyww() const { return { x, y, w, w }; }
inline constexpr Vector4 Vector4::xzxx() const { return { x, z, x, x }; }
inline constexpr Vector4 Vector4::xzxy() const { return { x, z, x, y }; }
inline constexpr Vector4 Vector4::xzxz() const { return { x, z, x, z }; }
inline constexpr Vector4 Vector4::xzxw() const { return { x, z, x, w }; }
inline constexpr Vector4 Vector4::xzyx() const { return { x, z, y, x }; }
inline constexpr Vector4 Vector4::xzyy() const { return { x, z, y, y }; }
inline constexpr Vector4 Vector4::xzyz() const { return { x, z, y, z }; }
inline constexpr Vector4 Vector4::xzyw() const { return { x, z, y, w }; }
inline constexpr Vector4 Vector4::xzzx() const { return { x, z, z, x }; }
inline constexpr Vector4 Vector4::xzzy() const { return { x, z, z, y }; }
inline constexpr Vector4 Vector4::xzzz() const { return { x, z, z, z }; }
inline constexpr Vector4 Vector4::xzzw() const { return { x, z, z, w }; }
inline constexpr Vector4 Vector4::xzwx() const { return { x, z, w, x }; }
inline constexpr Vector4 Vector4::xzwy() const { return { x, z, w, y }; }
inline constexpr Vector4 Vector4::xzwz() const { return { x, z, w, z }; }
inline constexpr Vector4 Vector4::xzww() const { return { x, z, w, w }; }
inline constexpr Vector4 Vector4::xwxx() const { return { x, w, x, x }; }
inline constexpr Vector4 Vector4::xwxy() const { return { x, w, x, y }; }
inline constexpr Vector4 Vector4::xwxz() const { return { x, w, x, z }; }
inline constexpr Vector4 Vector4::xwxw() const { return { x, w, x, w }; }
inline constexpr Vector4 Vector4::xwyx() const { return { x, w, y, x }; }
inline constexpr Vector4 Vector4::xwyy() const { return { x, w, y, y }; }
inline constexpr Vector4 Vector4::xwyz() const { return { x, w, y, z }; }
inline constexpr Vector4 Vector4::xwyw() const { return { x, w, y, w }; }
inline constexpr Vector4 Vector4::xwzx() const { return { x, w, z, x }; }
inline constexpr Vector4 Vector4::xwzy() const { return { x, w, z, y }; }
inline constexpr Vector4 Vector4::xwzz() const { return { x, w, z, z }; }
inline constexpr Vector4 Vector4::xwzw() const { return { x, w, z, w }; }
inline constexpr Vector4 Vector4::xwwx() const { return { x, w, w, x }; }
inline constexpr Vector4 Vector4::xwwy() const { return { x, w, w, y }; }
inline constexpr Vector4 Vector4::xwwz() const { return { x, w, w, z }; }
inline constexpr Vector4 Vector4::xwww() const { return { x, w, w, w }; }
inline constexpr Vector4 Vector4::yxxx() const { return { y, x, x, x }; }
inline constexpr Vector4 Vector4::yxxy() const { return { y, x, x, y }; }
inline constexpr Vector4 Vector4::yxxz() const { return { y, x, x, z }; }
inline constexpr Vector4 Vector4::yxxw() const { return { y, x, x, w }; }
inline constexpr Vector4 Vector4::yxyx() const { return { y, x, y, x }; }
inline constexpr Vector4 Vector4::yxyy() const { return { y, x, y, y }; }
inline constexpr Vector4 Vector4::yxyz() const { return { y, x, y, z }; }
inline constexpr Vector4 Vector4::yxyw() const { return { y, x, y, w }; }
inline constexpr Vector4 Vector4::yxzx() const { return { y, x, z, x }; }
inline constexpr Vector4 Vector4::yxzy() const { return { y, x, z, y }; }
inline constexpr Vector4 Vector4::yxzz() const { return { y, x, z, z }; }
inline constexpr Vector4 Vector4::yxzw() const { return { y, x, z, w }; }
inline constexpr Vector4 Vector4::yxwx() const { return { y, x, w, x }; }
inline constexpr Vector4 Vector4::yxwy() const { return { y, x, w, y }; }
inline constexpr Vector4 Vector4::yxwz() const { return { y, x, w, z }; }
inline constexpr Vector4 Vector4::yxww() const { return { y, x, w, w }; }
inline constexpr Vector4 Vector4::yyxx() const { return { y, y, x, x }; }
inline constexpr Vector4 Vector4::yyxy() const { return { y, y, x, y }; }
inline constexpr Vector4 Vector4::yyxz() const { return { y, y, x, z }; }
inline constexpr Vector4 Vector4::yyxw() const { return { y, y, x, w }; }
inline constexpr Vector4 Vector4::yyyx() const { return { y, y, y, x }; }
inline constexpr Vector4 Vector4::yyyy() const { return { y, y, y, y }; }
inline constexpr Vector4 Vector4::yyyz() const { return { y, y, y, z }; }
inline constexpr Vector4 Vector4::yyyw() const { return { y, y, y, w }; }
inline constexpr Vector4 Vector4::yyzx() const { return { y, y, z, x }; }
inline constexpr Vector4 Vector4::yyzy() const { return { y, y, z, y }; }
inline constexpr Vector4 Vector4::yyzz() const { return { y, y, z, z }; }
inline constexpr Vector4 Vector4::yyzw() const { return { y, y, z, w }; }
inline constexpr Vector4 Vector4::yywx() const { return { y, y, w, x }; }
inline constexpr Vector4 Vector4::yywy() const { return { y, y, w, y }; }
inline constexpr Vector4 Vector4::yywz() const { return { y, y, w, z }; }
inline constexpr Vector4 Vector4::yyww() const { return { y, y, w, w }; }
inline constexpr Vector4 Vector4::yzxx() const { return { y, z, x, x }; }
inline constexpr Vector4 Vector4::yzxy() const { return { y, z, x, y }; }
inline constexpr Vector4 Vector4::yzxz() const { return { y, z, x, z }; }
inline constexpr Vector4 Vector4::yzxw() const { return { y, z, x, w }; }
inline constexpr Vector4 Vector4::yzyx() const { return { y, z, y, x }; }
inline constexpr Vector4 Vector4::yzyy() const { return { y, z, y, y }; }
inline constexpr Vector4 Vector4::yzyz() const { return { y, z, y, z }; }
inline constexpr Vector4 Vector4::yzyw() const { return { y, z, y, w }; }
inline constexpr Vector4 Vector4::yzzx() const { return { y, z, z, x }; }
inline constexpr Vector4 Vector4::yzzy() const { return { y, z, z, y }; }
inline constexpr Vector4 Vector4::yzzz() const { return { y, z, z, z }; }
inline constexpr Vector4 Vector4::yzzw() const { return { y, z, z, w }; }
inline constexpr Vector4 Vector4::yzwx() const { return { y, z, w, x }; }
inline constexpr Vector4 Vector4::yzwy() const { return { y, z, w, y }; }
inline constexpr Vector4 Vector4::yzwz() const { return { y, z, w, z }; }
inline constexpr Vector4 Vector4::yzww() const { return { y, z, w, w }; }
inline constexpr Vector4 Vector4::ywxx() const { return { y, w, x, x }; }
inline constexpr Vector4 Vector4::ywxy() const { return { y, w, x, y }; }
inline constexpr Vector4 Vector4::ywxz() const { return { y, w, x, z }; }
inline constexpr Vector4 Vector4::ywxw() const { return { y, w, x, w }; }
inline constexpr Vector4 Vector4::ywyx() const { return { y, w, y, x }; }
inline constexpr Vector4 Vector4::ywyy() const { return { y, w, y, y }; }
inline constexpr Vector4 Vector4::ywyz() const { return { y, w, y, z }; }
inline constexpr Vector4 Vector4::ywyw() const { return { y, w, y, w }; }
inline constexpr Vector4 Vector4::ywzx() const { return { y, w, z, x }; }
inline constexpr Vector4 Vector4::ywzy() const { return { y, w, z, y }; }
inline constexpr Vector4 Vector4::ywzz() const { return { y, w, z, z }; }
inline constexpr Vector4 Vector4::ywzw() const { return { y, w, z, w }; }
inline constexpr Vector4 Vector4::ywwx() const { return { y, w, w, x }; }
inline constexpr Vector4 Vector4::ywwy() const { return { y, w, w, y }; }
inline constexpr Vector4 Vector4::ywwz() const { return { y, w, w, z }; }
inline constexpr Vector4 Vector4::ywww() const { return { y, w, w, w }; }
inline constexpr Vector4 Vector4::zxxx() const { return { z, x, x, x }; }
inline constexpr Vector4 Vector4::zxxy() const { return { z, x, x, y }; }
inline constexpr Vector4 Vector4::zxxz() const { return { z, x, x, z }; }
inline constexpr Vector4 Vector4::zxxw() const { return { z, x, x, w }; }
inline constexpr Vector4 Vector4::zxyx() const { return { z, x, y, x }; }
inline constexpr Vector4 Vector4::zxyy() const { return { z, x, y, y }; }
inline constexpr Vector4 Vector4::zxyz() const { return { z, x, y, z }; }
inline constexpr Vector4 Vector4::zxyw() const { return { z, x, y, w }; }
inline constexpr Vector4 Vector4::zxzx() const { return { z, x, z, x }; }
inline constexpr Vector4 Vector4::zxzy() const { return { z, x, z, y }; }
inline constexpr Vector4 Vector4::zxzz() const { return { z, x, z, z }; }
inline constexpr Vector4 Vector4::zxzw() const { return { z, x, z, w }; }
inline constexpr Vector4 Vector4::zxwx() const { return { z, x, w, x }; }
inline constexpr Vector4 Vector4::zxwy() const { return { z, x, w, y }; }
inline constexpr Vector4 Vector4::zxwz() const { return { z, x, w, z }; }
inline constexpr Vector4 Vector4::zxww() const { return { z, x, w, w }; }
inline constexpr Vector4 Vector4::zyxx() const { return { z, y, x, x }; }
inline constexpr Vector4 Vector4::zyxy() const { return { z, y, x, y }; }
inline constexpr Vector4 Vector4::zyxz() const { return { z, y, x, z }; }
inline constexpr Vector4 Vector4::zyxw() const { return { z, y, x, w }; }
inline constexpr Vector4 Vector4::zyyx() const { return { z, y, y, x }; }
inline constexpr Vector4 Vector4::zyyy() const { return { z, y, y, y }; }
inline constexpr Vector4 Vector4::zyyz() const { return { z, y, y, z }; }
inline constexpr Vector4 Vector4::zyyw() const { return { z, y, y, w }; }
inline constexpr Vector4 Vector4::zyzx() const { return { z, y, z, x }; }
inline constexpr Vector4 Vector4::zyzy() const { return { z, y, z, y }; }
inline constexpr Vector4 Vector4::zyzz() const { return { z, y, z, z }; }
inline constexpr Vector4 Vector4::zyzw() const { return { z, y, z, w }; }
inline constexpr Vector4 Vector4::zywx() const { return { z, y, w, x }; }
inline constexpr Vector4 Vector4::zywy() const { return { z, y, w, y }; }
inline constexpr Vector4 Vector4::zywz() const { return { z, y, w, z }; }
inline constexpr Vector4 Vector4::zyww() const { return { z, y, w, w }; }
inline constexpr Vector4 Vector4::zzxx() const { return { z, z, x, x }; }
inline constexpr Vector4 Vector4::zzxy() const { return { z, z, x, y }; }
inline constexpr Vector4 Vector4::zzxz() const { return { z, z, x, z }; }
inline constexpr Vector4 Vector4::zzxw() const { return { z, z, x, w }; }
inline constexpr Vector4 Vector4::zzyx() const { return { z, z, y, x }; }
inline constexpr Vector4 Vector4::zzyy() const { return { z, z, y, y }; }
inline constexpr Vector4 Vector4::zzyz() const { return { z, z, y, z }; }
inline constexpr Vector4 Vector4::zzyw() const { return { z, z, y, w }; }
inline constexpr Vector4 Vector4::zzzx() const { return { z, z, z, x }; }
inline constexpr Vector4 Vector4::zzzy() const { return { z, z, z, y }; }
inline constexpr Vector4 Vector4::zzzz() const { return { z, z, z, z }; }
inline constexpr Vector4 Vector4::zzzw() const { return { z, z, z, w }; }
inline constexpr Vector4 Vector4::zzwx() const { return { z, z, w, x }; }
inline constexpr Vector4 Vector4::zzwy() const { return { z, z, w, y }; }
inline constexpr Vector4 Vector4::zzwz() const { return { z, z, w, z }; }
inline constexpr Vector4 Vector4::zzww() const { return { z, z, w, w }; }
inline constexpr Vector4 Vector4::zwxx() const { return { z, w, x, x }; }
inline constexpr Vector4 Vector4::zwxy() const { return { z, w, x, y }; }
inline constexpr Vector4 Vector4::zwxz() const { return { z, w, x, z }; }
inline constexpr Vector4 Vector4::zwxw() const { return { z, w, x, w }; }
inline constexpr Vector4 Vector4::zwyx() const { return { z, w, y, x }; }
inline constexpr Vector4 Vector4::zwyy() const { return { z, w, y, y }; }
inline constexpr Vector4 Vector4::zwyz() const { return { z, w, y, z }; }
inline constexpr Vector4 Vector4::zwyw() const { return { z, w, y, w }; }
inline constexpr Vector4 Vector4::zwzx() const { return { z, w, z, x }; }
inline constexpr Vector4 Vector4::zwzy() const { return { z, w, z, y }; }
inline constexpr Vector4 Vector4::zwzz() const { return { z, w, z, z }; }
inline constexpr Vector4 Vector4::zwzw() const { return { z, w, z, w }; }
inline constexpr Vector4 Vector4::zwwx() const { return { z, w, w, x }; }
inline constexpr Vector4 Vector4::zwwy() const { return { z, w, w, y }; }
inline constexpr Vector4 Vector4::zwwz() const { return { z, w, w, z }; }
inline constexpr Vector4 Vector4::zwww() const { return { z, w, w, w }; }
inline constexpr Vector4 Vector4::wxxx() const { return { w, x, x, x }; }
inline constexpr Vector4 Vector4::wxxy() const { return { w, x, x, y }; }
inline constexpr Vector4 Vector4::wxxz() const { return { w, x, x, z }; }
inline constexpr Vector4 Vector4::wxxw() const { return { w, x, x, w }; }
inline constexpr Vector4 Vector4::wxyx() const { return { w, x, y, x }; }
inline constexpr Vector4 Vector4::wxyy() const { return { w, x, y, y }; }
inline constexpr Vector4 Vector4::wxyz() const { return { w, x, y, z }; }
inline constexpr Vector4 Vector4::wxyw() const { return { w, x, y, w }; }
inline constexpr Vector4 Vector4::wxzx() const { return { w, x, z, x }; }
inline constexpr Vector4 Vector4::wxzy() const { return { w, x, z, y }; }
inline constexpr Vector4 Vector4::wxzz() const { return { w, x, z, z }; }
inline constexpr Vector4 Vector4::wxzw() const { return { w, x, z, w }; }
inline constexpr Vector4 Vector4::wxwx() const { return { w, x, w, x }; }
inline constexpr Vector4 Vector4::wxwy() const { return { w, x, w, y }; }
inline constexpr Vector4 Vector4::wxwz() const { return { w, x, w, z }; }
inline constexpr Vector4 Vector4::wxww() const { return { w, x, w, w }; }
inline constexpr Vector4 Vector4::wyxx() const { return { w, y, x, x }; }
inline constexpr Vector4 Vector4::wyxy() const { return { w, y, x, y }; }
inline constexpr Vector4 Vector4::wyxz() const { return { w, y, x, z }; }
inline constexpr Vector4 Vector4::wyxw() const { return { w, y, x, w }; }
inline constexpr Vector4 Vector4::wyyx() const { return { w, y, y, x }; }
inline constexpr Vector4 Vector4::wyyy() const { return { w, y, y, y }; }
inline constexpr Vector4 Vector4::wyyz() const { return { w, y, y, z }; }
inline constexpr Vector4 Vector4::wyyw() const { return { w, y, y, w }; }
inline constexpr Vector4 Vector4::wyzx() const { return { w, y, z, x }; }
inline constexpr Vector4 Vector4::wyzy() const { return { w, y, z, y }; }
inline constexpr Vector4 Vector4::wyzz() const { return { w, y, z, z }; }
inline constexpr Vector4 Vector4::wyzw() const { return { w, y, z, w }; }
inline constexpr Vector4 Vector4::wywx() const { return { w, y, w, x }; }
inline constexpr Vector4 Vector4::wywy() const { return { w, y, w, y }; }
inline constexpr Vector4 Vector4::wywz() const { return { w, y, w, z }; }
inline constexpr Vector4 Vector4::wyww() const { return { w, y, w, w }; }
inline constexpr Vector4 Vector4::wzxx() const { return { w, z, x, x }; }
inline constexpr Vector4 Vector4::wzxy() const { return { w, z, x, y }; }
inline constexpr Vector4 Vector4::wzxz() const { return { w, z, x, z }; }
inline constexpr Vector4 Vector4::wzxw() const { return { w, z, x, w }; }
inline constexpr Vector4 Vector4::wzyx() const { return { w, z, y, x }; }
inline constexpr Vector4 Vector4::wzyy() const { return { w, z, y, y }; }
inline constexpr Vector4 Vector4::wzyz() const { return { w, z, y, z }; }
inline constexpr Vector4 Vector4::wzyw() const { return { w, z, y, w }; }
inline constexpr Vector4 Vector4::wzzx() const { return { w, z, z, x }; }
inline constexpr Vector4 Vector4::wzzy() const { return { w, z, z, y }; }
inline constexpr Vector4 Vector4::wzzz() const { return { w, z, z, z }; }
inline constexpr Vector4 Vector4::wzzw() const { return { w, z, z, w }; }
inline constexpr Vector4 Vector4::wzwx() const { return { w, z, w, x }; }
inline constexpr Vector4 Vector4::wzwy() const { return { w, z, w, y }; }
inline constexpr Vector4 Vector4::wzwz() const { return { w, z, w, z }; }
inline constexpr Vector4 Vector4::wzww() const { return { w, z, w, w }; }
inline constexpr Vector4 Vector4::wwxx() const { return { w, w, x, x }; }
inline constexpr Vector4 Vector4::wwxy() const { return { w, w, x, y }; }
inline constexpr Vector4 Vector4::wwxz() const { return { w, w, x, z }; }
inline constexpr Vector4 Vector4::wwxw() const { return { w, w, x, w }; }
inline constexpr Vector4 Vector4::wwyx() const { return { w, w, y, x }; }
inline constexpr Vector4 Vector4::wwyy() const { return { w, w, y, y }; }
inline constexpr Vector4 Vector4::wwyz() const { return { w, w, y, z }; }
inline constexpr Vector4 Vector4::wwyw() const { return { w, w, y, w }; }
inline constexpr Vector4 Vector4::wwzx() const { return { w, w, z, x }; }
inline constexpr Vector4 Vector4::wwzy() const { return { w, w, z, y }; }
inline constexpr Vector4 Vector4::wwzz() const { return { w, w, z, z }; }
inline constexpr Vector4 Vector4::wwzw() const { return { w, w, z, w }; }
inline constexpr Vector4 Vector4::wwwx() const { return { w, w, w, x }; }
inline constexpr Vector4 Vector4::wwwy() const { return { w, w, w, y }; }
inline constexpr Vector4 Vector4::wwwz() const { return { w, w, w, z }; }
inline constexpr Vector4 Vector4::wwww() const { return { w, w, w, w }; }
#pragma endregion

inline constexpr Vector4::operator Vector2() const { return { x, y }; }
inline constexpr Vector4::operator Vector3() const { return { x, y, z }; }
inline constexpr Vector4::operator Vector2Int() const { return { (I32)x, (I32)y }; }
inline constexpr Vector4::operator Vector3Int() const { return { (I32)x, (I32)y, (I32)z }; }
inline constexpr Vector4::operator Vector4Int() const { return { (I32)x, (I32)y, (I32)z, (I32)w }; }
#pragma endregion

#pragma region Vector2Int
inline constexpr Vector2Int::Vector2Int() : x(0), y(0) {}
inline constexpr Vector2Int::Vector2Int(I32 i) : x(i), y(i) {}
inline constexpr Vector2Int::Vector2Int(I32 x, I32 y) : x(x), y(y) {}
inline constexpr Vector2Int::Vector2Int(const Vector2Int& v) : x(v.x), y(v.y) {}
inline constexpr Vector2Int::Vector2Int(Vector2Int&& v) noexcept : x(v.x), y(v.y) {}

inline constexpr Vector2Int& Vector2Int::operator=(I32 i) { x = i; y = i; return *this; }
inline constexpr Vector2Int& Vector2Int::operator=(const Vector2Int& v) { x = v.x; y = v.y; return *this; }
inline constexpr Vector2Int& Vector2Int::operator=(Vector2Int&& v) noexcept { x = v.x; y = v.y; return *this; }

inline constexpr Vector2Int& Vector2Int::operator+=(I32 i) { x += i; y += i; return *this; }
inline constexpr Vector2Int& Vector2Int::operator-=(I32 i) { x -= i; y -= i; return *this; }
inline constexpr Vector2Int& Vector2Int::operator*=(I32 i) { x *= i; y *= i; return *this; }
inline constexpr Vector2Int& Vector2Int::operator/=(I32 i) { x /= i; y /= i; return *this; }
inline constexpr Vector2Int& Vector2Int::operator%=(I32 i) { x %= i; y %= i; return *this; }
inline constexpr Vector2Int& Vector2Int::operator+=(const Vector2Int& v) { x += v.x; y += v.y; return *this; }
inline constexpr Vector2Int& Vector2Int::operator-=(const Vector2Int& v) { x -= v.x; y -= v.y; return *this; }
inline constexpr Vector2Int& Vector2Int::operator*=(const Vector2Int& v) { x *= v.x; y *= v.y; return *this; }
inline constexpr Vector2Int& Vector2Int::operator/=(const Vector2Int& v) { x /= v.x; y /= v.y; return *this; }
inline constexpr Vector2Int& Vector2Int::operator%=(const Vector2Int& v) { x &= v.x; y &= v.y; return *this; }

inline constexpr Vector2Int Vector2Int::operator+(I32 i) const { return { x + i, y + i }; }
inline constexpr Vector2Int Vector2Int::operator-(I32 i) const { return { x - i, y - i }; }
inline constexpr Vector2Int Vector2Int::operator*(I32 i) const { return { x * i, y * i }; }
inline constexpr Vector2Int Vector2Int::operator/(I32 i) const { return { x / i, y / i }; }
inline constexpr Vector2Int Vector2Int::operator%(I32 i) const { return { x % i, y % i }; }
inline constexpr Vector2Int Vector2Int::operator+(const Vector2Int& v) const { return { x + v.x, y + v.y }; }
inline constexpr Vector2Int Vector2Int::operator-(const Vector2Int& v) const { return { x - v.x, y - v.y }; }
inline constexpr Vector2Int Vector2Int::operator*(const Vector2Int& v) const { return { x * v.x, y * v.y }; }
inline constexpr Vector2Int Vector2Int::operator/(const Vector2Int& v) const { return { x / v.x, y / v.y }; }
inline constexpr Vector2Int Vector2Int::operator%(const Vector2Int& v) const { return { x % v.x, y % v.y }; }
inline constexpr Vector2Int Vector2Int::operator+(const Vector2& v) const { return { (I32)(x + v.x), (I32)(y + v.y) }; }
inline constexpr Vector2Int Vector2Int::operator-(const Vector2& v) const { return { (I32)(x - v.x), (I32)(y - v.y) }; }
inline constexpr Vector2Int Vector2Int::operator*(const Vector2& v) const { return { (I32)(x * v.x), (I32)(y * v.y) }; }
inline constexpr Vector2Int Vector2Int::operator/(const Vector2& v) const { return { (I32)(x / v.x), (I32)(y / v.y) }; }

inline constexpr bool Vector2Int::operator==(const Vector2Int& v) const { return !(x - v.x) && !(y - v.y); }
inline constexpr bool Vector2Int::operator!=(const Vector2Int& v) const { return (x - v.x) || (y - v.y); }
inline constexpr bool Vector2Int::operator>(const Vector2Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector2Int::operator<(const Vector2Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector2Int::operator>=(const Vector2Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector2Int::operator<=(const Vector2Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector2Int::IsZero() const { return !x && !y; }

inline constexpr Vector2Int Vector2Int::operator-() const { return { -x, -y }; }
inline constexpr Vector2Int Vector2Int::operator~() const { return { -x, -y }; }
inline constexpr Vector2Int Vector2Int::operator!() const { return { -x, -y }; }

inline constexpr I32 Vector2Int::SqrMagnitude() const { return x * x + y * y; }
inline constexpr F32 Vector2Int::Magnitude() const { return Math::Sqrt((F32)(x * x + y * y)); }
inline constexpr I32 Vector2Int::Dot(const Vector2Int& v) const { return x * v.x + y * v.y; }
inline constexpr I32 Vector2Int::Dot(I32 vx, I32 vy) const { return x * vx + y * vy; }

inline constexpr Vector2Int& Vector2Int::Clamp(const Vector2Int& min, const Vector2Int& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	return *this;
}
inline constexpr Vector2Int Vector2Int::Clamped(const Vector2Int& min, const Vector2Int& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y)
	};
}
inline constexpr Vector2Int& Vector2Int::SetClosest(const Vector2Int& min, const Vector2Int& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	return *this;
}
inline constexpr Vector2Int Vector2Int::Closest(const Vector2Int& min, const Vector2Int& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y)
	};
}

inline I32& Vector2Int::operator[] (U64 i) { return (&x)[i]; }
inline const I32& Vector2Int::operator[] (U64 i) const { return (&x)[i]; }

inline I32* Vector2Int::Data() { return &x; }
inline const I32* Vector2Int::Data() const { return &x; }

inline constexpr Vector2Int Vector2Int::xx() const { return { x, x }; }
inline constexpr Vector2Int Vector2Int::xy() const { return { x, y }; }
inline constexpr Vector2Int Vector2Int::yx() const { return { y, x }; }
inline constexpr Vector2Int Vector2Int::yy() const { return { y, y }; }

inline constexpr Vector2Int::operator Vector2() const { return { (F32)x, (F32)y }; }
inline constexpr Vector2Int::operator Vector3() const { return { (F32)x, (F32)y, 0.0f }; }
inline constexpr Vector2Int::operator Vector4() const { return { (F32)x, (F32)y, 0.0f, 0.0f }; }
inline constexpr Vector2Int::operator Vector3Int() const { return { x, y, 0 }; }
inline constexpr Vector2Int::operator Vector4Int() const { return { x, y, 0, 0 }; }
#pragma endregion

#pragma region Vector3Int
inline constexpr Vector3Int::Vector3Int() : x(0), y(0), z(0) {}
inline constexpr Vector3Int::Vector3Int(I32 i) : x(i), y(i), z(i) {}
inline constexpr Vector3Int::Vector3Int(I32 x, I32 y, I32 z) : x(x), y(y), z(z) {}
inline constexpr Vector3Int::Vector3Int(const Vector3Int& v) : x(v.x), y(v.y), z(v.z) {}
inline constexpr Vector3Int::Vector3Int(Vector3Int&& v) noexcept : x(v.x), y(v.y), z(v.z) {}

inline constexpr Vector3Int& Vector3Int::operator=(I32 i) { x = i; y = i; z = i; return *this; }
inline constexpr Vector3Int& Vector3Int::operator=(const Vector3Int& v) { x = v.x; y = v.y; z = v.z; return *this; }
inline constexpr Vector3Int& Vector3Int::operator=(Vector3Int&& v) noexcept { x = v.x; y = v.y; z = v.z; return *this; }

inline constexpr Vector3Int& Vector3Int::operator+=(I32 i) { x += i; y += i; z += i; return *this; }
inline constexpr Vector3Int& Vector3Int::operator-=(I32 i) { x -= i; y -= i; z -= i; return *this; }
inline constexpr Vector3Int& Vector3Int::operator*=(I32 i) { x *= i; y *= i; z *= i; return *this; }
inline constexpr Vector3Int& Vector3Int::operator/=(I32 i) { x /= i; y /= i; z /= i; return *this; }
inline constexpr Vector3Int& Vector3Int::operator%=(I32 i) { x %= i; y %= i; z %= i; return *this; }
inline constexpr Vector3Int& Vector3Int::operator+=(const Vector3Int& v) { x += v.x; y += v.y; z += v.z; return *this; }
inline constexpr Vector3Int& Vector3Int::operator-=(const Vector3Int& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
inline constexpr Vector3Int& Vector3Int::operator*=(const Vector3Int& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
inline constexpr Vector3Int& Vector3Int::operator/=(const Vector3Int& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
inline constexpr Vector3Int& Vector3Int::operator%=(const Vector3Int& v) { x %= v.x; y %= v.y; z %= v.z; return *this; }

inline constexpr Vector3Int Vector3Int::operator+(I32 i) const { return { x + i, y + i, z + i }; }
inline constexpr Vector3Int Vector3Int::operator-(I32 i) const { return { x - i, y - i, z - i }; }
inline constexpr Vector3Int Vector3Int::operator*(I32 i) const { return { x * i, y * i, z * i }; }
inline constexpr Vector3Int Vector3Int::operator/(I32 i) const { return { x / i, y / i, z / i }; }
inline constexpr Vector3Int Vector3Int::operator%(I32 i) const { return { x % i, y % i, z % i }; }
inline constexpr Vector3Int Vector3Int::operator+(const Vector3Int& v) const { return { x + v.x, y + v.y, z + v.z }; }
inline constexpr Vector3Int Vector3Int::operator-(const Vector3Int& v) const { return { x - v.x, y - v.y, z - v.z }; }
inline constexpr Vector3Int Vector3Int::operator*(const Vector3Int& v) const { return { x * v.x, y * v.y, z * v.z }; }
inline constexpr Vector3Int Vector3Int::operator/(const Vector3Int& v) const { return { x / v.x, y / v.y, z / v.z }; }
inline constexpr Vector3Int Vector3Int::operator%(const Vector3Int& v) const { return { x % v.x, y % v.y, z % v.z }; }

inline constexpr bool Vector3Int::operator==(const Vector3Int& v) const { return !(x - v.x) && !(y - v.y) && !(z - v.z); }
inline constexpr bool Vector3Int::operator!=(const Vector3Int& v) const { return (x - v.x) || (y - v.y) || (z - v.z); }
inline constexpr bool Vector3Int::operator>(const Vector3Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector3Int::operator<(const Vector3Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector3Int::operator>=(const Vector3Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector3Int::operator<=(const Vector3Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector3Int::IsZero() const { return !x && !y && !z; }

inline constexpr Vector3Int Vector3Int::operator-() const { return { -x, -y, -z }; }
inline constexpr Vector3Int Vector3Int::operator~() const { return { ~x, ~y, ~z }; }
inline constexpr Vector3Int Vector3Int::operator!() const { return { !x, !y, !z }; }

inline constexpr I32 Vector3Int::SqrMagnitude() const { return x * x + y * y + z * z; }
inline constexpr F32 Vector3Int::Magnitude() const { return Math::Sqrt((F32)(x * x + y * y + z * z)); }
inline constexpr I32 Vector3Int::Dot(const Vector3Int& v) const { return x * v.x + y * v.y + z * v.z; }
inline constexpr I32 Vector3Int::Dot(I32 vx, I32 vy, I32 vz) const { return x * vx + y * vy + z * vz; }

inline constexpr Vector3Int& Vector3Int::Clamp(const Vector3Int& min, const Vector3Int& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	return *this;
}
inline constexpr Vector3Int Vector3Int::Clamped(const Vector3Int& min, const Vector3Int& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z)
	};
}
inline constexpr Vector3Int& Vector3Int::SetClosest(const Vector3Int& min, const Vector3Int& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	return *this;
}
inline constexpr Vector3Int Vector3Int::Closest(const Vector3Int& min, const Vector3Int& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z)
	};
}

inline I32& Vector3Int::operator[] (U64 i) { return (&x)[i]; }
inline const I32& Vector3Int::operator[] (U64 i) const { return (&x)[i]; }

inline I32* Vector3Int::Data() { return &x; }
inline const I32* Vector3Int::Data() const { return &x; }

#pragma region SwizzleFunctions
inline constexpr Vector2Int Vector3Int::xx() const { return { x, x }; }
inline constexpr Vector2Int Vector3Int::xy() const { return { x, y }; }
inline constexpr Vector2Int Vector3Int::xz() const { return { x, z }; }
inline constexpr Vector2Int Vector3Int::yx() const { return { y, x }; }
inline constexpr Vector2Int Vector3Int::yy() const { return { y, y }; }
inline constexpr Vector2Int Vector3Int::yz() const { return { y, z }; }
inline constexpr Vector2Int Vector3Int::zx() const { return { z, x }; }
inline constexpr Vector2Int Vector3Int::zy() const { return { z, y }; }
inline constexpr Vector2Int Vector3Int::zz() const { return { z, z }; }
inline constexpr Vector3Int Vector3Int::xxx() const { return { x, x, x }; }
inline constexpr Vector3Int Vector3Int::xxy() const { return { x, x, y }; }
inline constexpr Vector3Int Vector3Int::xxz() const { return { x, x, z }; }
inline constexpr Vector3Int Vector3Int::xyx() const { return { x, y, x }; }
inline constexpr Vector3Int Vector3Int::xyy() const { return { x, y, y }; }
inline constexpr Vector3Int Vector3Int::xyz() const { return { x, y, z }; }
inline constexpr Vector3Int Vector3Int::xzx() const { return { x, z, x }; }
inline constexpr Vector3Int Vector3Int::xzy() const { return { x, z, y }; }
inline constexpr Vector3Int Vector3Int::xzz() const { return { x, z, z }; }
inline constexpr Vector3Int Vector3Int::yxx() const { return { y, x, x }; }
inline constexpr Vector3Int Vector3Int::yxy() const { return { y, x, y }; }
inline constexpr Vector3Int Vector3Int::yxz() const { return { y, x, z }; }
inline constexpr Vector3Int Vector3Int::yyx() const { return { y, y, x }; }
inline constexpr Vector3Int Vector3Int::yyy() const { return { y, y, y }; }
inline constexpr Vector3Int Vector3Int::yyz() const { return { y, y, z }; }
inline constexpr Vector3Int Vector3Int::yzx() const { return { y, z, x }; }
inline constexpr Vector3Int Vector3Int::yzy() const { return { y, z, y }; }
inline constexpr Vector3Int Vector3Int::yzz() const { return { y, z, z }; }
inline constexpr Vector3Int Vector3Int::zxx() const { return { z, x, x }; }
inline constexpr Vector3Int Vector3Int::zxy() const { return { z, x, y }; }
inline constexpr Vector3Int Vector3Int::zxz() const { return { z, x, z }; }
inline constexpr Vector3Int Vector3Int::zyx() const { return { z, y, x }; }
inline constexpr Vector3Int Vector3Int::zyy() const { return { z, y, y }; }
inline constexpr Vector3Int Vector3Int::zyz() const { return { z, y, z }; }
inline constexpr Vector3Int Vector3Int::zzx() const { return { z, z, x }; }
inline constexpr Vector3Int Vector3Int::zzy() const { return { z, z, y }; }
inline constexpr Vector3Int Vector3Int::zzz() const { return { z, z, z }; }
#pragma endregion

inline constexpr Vector3Int::operator Vector2() const { return { (F32)x, (F32)y }; }
inline constexpr Vector3Int::operator Vector3() const { return { (F32)x, (F32)y, (F32)z }; }
inline constexpr Vector3Int::operator Vector4() const { return { (F32)x, (F32)y, (F32)z, 0.0f }; }
inline constexpr Vector3Int::operator Vector2Int() const { return { x, y }; }
inline constexpr Vector3Int::operator Vector4Int() const { return { x, y, z, 0 }; }
#pragma endregion

#pragma region Vector4Int
inline constexpr Vector4Int::Vector4Int() : x(0), y(0), z(0), w(0) {}
inline constexpr Vector4Int::Vector4Int(I32 i) : x(i), y(i), z(i), w(i) {}
inline constexpr Vector4Int::Vector4Int(I32 x, I32 y, I32 z, I32 w) : x(x), y(y), z(z), w(w) {}
inline constexpr Vector4Int::Vector4Int(const Vector4Int& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
inline constexpr Vector4Int::Vector4Int(Vector4Int&& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) {}

inline constexpr Vector4Int& Vector4Int::operator=(I32 i) { x = i; y = i; z = i; w = i; return *this; }
inline constexpr Vector4Int& Vector4Int::operator=(const Vector4Int& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
inline constexpr Vector4Int& Vector4Int::operator=(Vector4Int&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

inline constexpr Vector4Int& Vector4Int::operator+=(I32 i) { x += i; y += i; z += i; w += i; return *this; }
inline constexpr Vector4Int& Vector4Int::operator-=(I32 i) { x -= i; y -= i; z -= i; w -= i; return *this; }
inline constexpr Vector4Int& Vector4Int::operator*=(I32 i) { x *= i; y *= i; z *= i; w *= i; return *this; }
inline constexpr Vector4Int& Vector4Int::operator/=(I32 i) { x /= i; y /= i; z /= i; w /= i; return *this; }
inline constexpr Vector4Int& Vector4Int::operator%=(I32 i) { x &= i; y %= i; z %= i; w %= i; return *this; }
inline constexpr Vector4Int& Vector4Int::operator+=(F32 f) { x += (I32)f; y += (I32)f; z += (I32)f; w += (I32)f; return *this; }
inline constexpr Vector4Int& Vector4Int::operator-=(F32 f) { x -= (I32)f; y -= (I32)f; z -= (I32)f; w -= (I32)f; return *this; }
inline constexpr Vector4Int& Vector4Int::operator*=(F32 f) { x *= (I32)f; y *= (I32)f; z *= (I32)f; w *= (I32)f; return *this; }
inline constexpr Vector4Int& Vector4Int::operator/=(F32 f) { x /= (I32)f; y /= (I32)f; z /= (I32)f; w /= (I32)f; return *this; }
inline constexpr Vector4Int& Vector4Int::operator+=(const Vector4Int& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
inline constexpr Vector4Int& Vector4Int::operator-=(const Vector4Int& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
inline constexpr Vector4Int& Vector4Int::operator*=(const Vector4Int& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
inline constexpr Vector4Int& Vector4Int::operator/=(const Vector4Int& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
inline constexpr Vector4Int& Vector4Int::operator%=(const Vector4Int& v) { x %= v.x; y %= v.y; z %= v.z; w %= v.w; return *this; }

inline constexpr Vector4Int Vector4Int::operator+(I32 i) const { return { x + i, y + i, z + i, w + i }; }
inline constexpr Vector4Int Vector4Int::operator-(I32 i) const { return { x - i, y - i, z - i, w - i }; }
inline constexpr Vector4Int Vector4Int::operator*(I32 i) const { return { x * i, y * i, z * i, w * i }; }
inline constexpr Vector4Int Vector4Int::operator/(I32 i) const { return { x / i, y / i, z / i, w / i }; }
inline constexpr Vector4Int Vector4Int::operator%(I32 i) const { return { x % i, y % i, z % i, w % i }; }
inline constexpr Vector4Int Vector4Int::operator+(F32 f) const { return { (I32)(x + f), (I32)(y + f), (I32)(z + f), (I32)(w + f) }; }
inline constexpr Vector4Int Vector4Int::operator-(F32 f) const { return { (I32)(x - f), (I32)(y - f), (I32)(z - f), (I32)(w - f) }; }
inline constexpr Vector4Int Vector4Int::operator*(F32 f) const { return { (I32)(x * f), (I32)(y * f), (I32)(z * f), (I32)(w * f) }; }
inline constexpr Vector4Int Vector4Int::operator/(F32 f) const { return { (I32)(x / f), (I32)(y / f), (I32)(z / f), (I32)(w / f) }; }
inline constexpr Vector4Int Vector4Int::operator+(const Vector4Int& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
inline constexpr Vector4Int Vector4Int::operator-(const Vector4Int& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
inline constexpr Vector4Int Vector4Int::operator*(const Vector4Int& v) const { return { x * v.x, y * v.y, z * v.z, w * v.w }; }
inline constexpr Vector4Int Vector4Int::operator/(const Vector4Int& v) const { return { x / v.x, y / v.y, z / v.z, w / v.w }; }
inline constexpr Vector4Int Vector4Int::operator%(const Vector4Int& v) const { return { x % v.x, y % v.y, z % v.z, w % v.w }; }

inline constexpr bool Vector4Int::operator==(const Vector4Int& v) const { return !(x - v.x) && !(y - v.y) && (z - v.z) && !(w - v.w); }
inline constexpr bool Vector4Int::operator!=(const Vector4Int& v) const { return (x - v.x) || (y - v.y) || (z - v.z) || (w - v.w); }
inline constexpr bool Vector4Int::operator>(const Vector4Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector4Int::operator<(const Vector4Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector4Int::operator>=(const Vector4Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
inline constexpr bool Vector4Int::operator<=(const Vector4Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
inline constexpr bool Vector4Int::IsZero() const { return !x && !y && !z && !w; }

inline constexpr Vector4Int Vector4Int::operator-() const { return { -x, -y, -z, -w }; }
inline constexpr Vector4Int Vector4Int::operator~() const { return { !x, !y, !z, !w }; }
inline constexpr Vector4Int Vector4Int::operator!() const { return { !x, !y, !z, !w }; }

inline constexpr I32 Vector4Int::SqrMagnitude() const { return x * x + y * y + z * z + w * w; }
inline constexpr F32 Vector4Int::Magnitude() const { return Math::Sqrt((F32)(x * x + y * y + z * z + w * w)); }
inline constexpr I32 Vector4Int::Dot(const Vector4Int& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
inline constexpr I32 Vector4Int::Dot(I32 vx, I32 vy, I32 vz, I32 vw) const { return x * vx + y * vy + z * vz + w * vw; }
inline constexpr Vector4Int& Vector4Int::Clamp(const Vector4Int& min, const Vector4Int& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	w = Math::Clamp(w, min.w, max.w);
	return *this;
}
inline constexpr Vector4Int Vector4Int::Clamped(const Vector4Int& min, const Vector4Int& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z),
		Math::Clamp(w, min.w, max.w)
	};
}
inline constexpr Vector4Int& Vector4Int::SetClosest(const Vector4Int& min, const Vector4Int& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	w = Math::Closest(w, min.w, max.w);
	return *this;
}
inline constexpr Vector4Int Vector4Int::Closest(const Vector4Int& min, const Vector4Int& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z),
		Math::Closest(w, min.w, max.w)
	};
}

inline I32& Vector4Int::operator[] (U64 i) { return (&x)[i]; }
inline const I32& Vector4Int::operator[] (U64 i) const { return (&x)[i]; }

inline I32* Vector4Int::Data() { return &x; }
inline const I32* Vector4Int::Data() const { return &x; }

#pragma region SwizzleFunctions
inline constexpr Vector2Int Vector4Int::xx() const { return { x, x }; }
inline constexpr Vector2Int Vector4Int::xy() const { return { x, y }; }
inline constexpr Vector2Int Vector4Int::xz() const { return { x, z }; }
inline constexpr Vector2Int Vector4Int::xw() const { return { x, w }; }
inline constexpr Vector2Int Vector4Int::yx() const { return { y, x }; }
inline constexpr Vector2Int Vector4Int::yy() const { return { y, y }; }
inline constexpr Vector2Int Vector4Int::yz() const { return { y, z }; }
inline constexpr Vector2Int Vector4Int::yw() const { return { y, w }; }
inline constexpr Vector2Int Vector4Int::zx() const { return { z, x }; }
inline constexpr Vector2Int Vector4Int::zy() const { return { z, y }; }
inline constexpr Vector2Int Vector4Int::zz() const { return { z, z }; }
inline constexpr Vector2Int Vector4Int::zw() const { return { z, w }; }
inline constexpr Vector2Int Vector4Int::wx() const { return { w, x }; }
inline constexpr Vector2Int Vector4Int::wy() const { return { w, y }; }
inline constexpr Vector2Int Vector4Int::wz() const { return { w, z }; }
inline constexpr Vector2Int Vector4Int::ww() const { return { w, w }; }
inline constexpr Vector3Int Vector4Int::xxx() const { return { x, x, x }; }
inline constexpr Vector3Int Vector4Int::xxy() const { return { x, x, y }; }
inline constexpr Vector3Int Vector4Int::xxz() const { return { x, x, z }; }
inline constexpr Vector3Int Vector4Int::xxw() const { return { x, x, w }; }
inline constexpr Vector3Int Vector4Int::xyx() const { return { x, y, x }; }
inline constexpr Vector3Int Vector4Int::xyy() const { return { x, y, y }; }
inline constexpr Vector3Int Vector4Int::xyz() const { return { x, y, z }; }
inline constexpr Vector3Int Vector4Int::xyw() const { return { x, y, w }; }
inline constexpr Vector3Int Vector4Int::xzx() const { return { x, z, x }; }
inline constexpr Vector3Int Vector4Int::xzy() const { return { x, z, y }; }
inline constexpr Vector3Int Vector4Int::xzz() const { return { x, z, z }; }
inline constexpr Vector3Int Vector4Int::xzw() const { return { x, z, w }; }
inline constexpr Vector3Int Vector4Int::xwx() const { return { x, w, x }; }
inline constexpr Vector3Int Vector4Int::xwy() const { return { x, w, y }; }
inline constexpr Vector3Int Vector4Int::xwz() const { return { x, w, z }; }
inline constexpr Vector3Int Vector4Int::xww() const { return { x, w, w }; }
inline constexpr Vector3Int Vector4Int::yxx() const { return { y, x, x }; }
inline constexpr Vector3Int Vector4Int::yxy() const { return { y, x, y }; }
inline constexpr Vector3Int Vector4Int::yxz() const { return { y, x, z }; }
inline constexpr Vector3Int Vector4Int::yxw() const { return { y, x, w }; }
inline constexpr Vector3Int Vector4Int::yyx() const { return { y, y, x }; }
inline constexpr Vector3Int Vector4Int::yyy() const { return { y, y, y }; }
inline constexpr Vector3Int Vector4Int::yyz() const { return { y, y, z }; }
inline constexpr Vector3Int Vector4Int::yyw() const { return { y, y, w }; }
inline constexpr Vector3Int Vector4Int::yzx() const { return { y, z, x }; }
inline constexpr Vector3Int Vector4Int::yzy() const { return { y, z, y }; }
inline constexpr Vector3Int Vector4Int::yzz() const { return { y, z, z }; }
inline constexpr Vector3Int Vector4Int::yzw() const { return { y, z, w }; }
inline constexpr Vector3Int Vector4Int::ywx() const { return { y, w, x }; }
inline constexpr Vector3Int Vector4Int::ywy() const { return { y, w, y }; }
inline constexpr Vector3Int Vector4Int::ywz() const { return { y, w, z }; }
inline constexpr Vector3Int Vector4Int::yww() const { return { y, w, w }; }
inline constexpr Vector3Int Vector4Int::zxx() const { return { z, x, x }; }
inline constexpr Vector3Int Vector4Int::zxy() const { return { z, x, y }; }
inline constexpr Vector3Int Vector4Int::zxz() const { return { z, x, z }; }
inline constexpr Vector3Int Vector4Int::zxw() const { return { z, x, w }; }
inline constexpr Vector3Int Vector4Int::zyx() const { return { z, y, x }; }
inline constexpr Vector3Int Vector4Int::zyy() const { return { z, y, y }; }
inline constexpr Vector3Int Vector4Int::zyz() const { return { z, y, z }; }
inline constexpr Vector3Int Vector4Int::zyw() const { return { z, y, w }; }
inline constexpr Vector3Int Vector4Int::zzx() const { return { z, z, x }; }
inline constexpr Vector3Int Vector4Int::zzy() const { return { z, z, y }; }
inline constexpr Vector3Int Vector4Int::zzz() const { return { z, z, z }; }
inline constexpr Vector3Int Vector4Int::zzw() const { return { z, z, w }; }
inline constexpr Vector3Int Vector4Int::zwx() const { return { z, w, x }; }
inline constexpr Vector3Int Vector4Int::zwy() const { return { z, w, y }; }
inline constexpr Vector3Int Vector4Int::zwz() const { return { z, w, z }; }
inline constexpr Vector3Int Vector4Int::zww() const { return { z, w, w }; }
inline constexpr Vector3Int Vector4Int::wxx() const { return { w, x, x }; }
inline constexpr Vector3Int Vector4Int::wxy() const { return { w, x, y }; }
inline constexpr Vector3Int Vector4Int::wxz() const { return { w, x, z }; }
inline constexpr Vector3Int Vector4Int::wxw() const { return { w, x, w }; }
inline constexpr Vector3Int Vector4Int::wyx() const { return { w, y, x }; }
inline constexpr Vector3Int Vector4Int::wyy() const { return { w, y, y }; }
inline constexpr Vector3Int Vector4Int::wyz() const { return { w, y, z }; }
inline constexpr Vector3Int Vector4Int::wyw() const { return { w, y, w }; }
inline constexpr Vector3Int Vector4Int::wzx() const { return { w, z, x }; }
inline constexpr Vector3Int Vector4Int::wzy() const { return { w, z, y }; }
inline constexpr Vector3Int Vector4Int::wzz() const { return { w, z, z }; }
inline constexpr Vector3Int Vector4Int::wzw() const { return { w, z, w }; }
inline constexpr Vector3Int Vector4Int::wwx() const { return { w, w, x }; }
inline constexpr Vector3Int Vector4Int::wwy() const { return { w, w, y }; }
inline constexpr Vector3Int Vector4Int::wwz() const { return { w, w, z }; }
inline constexpr Vector3Int Vector4Int::www() const { return { w, w, w }; }
inline constexpr Vector4Int Vector4Int::xxxx() const { return { x, x, x, x }; }
inline constexpr Vector4Int Vector4Int::xxxy() const { return { x, x, x, y }; }
inline constexpr Vector4Int Vector4Int::xxxz() const { return { x, x, x, z }; }
inline constexpr Vector4Int Vector4Int::xxxw() const { return { x, x, x, w }; }
inline constexpr Vector4Int Vector4Int::xxyx() const { return { x, x, y, x }; }
inline constexpr Vector4Int Vector4Int::xxyy() const { return { x, x, y, y }; }
inline constexpr Vector4Int Vector4Int::xxyz() const { return { x, x, y, z }; }
inline constexpr Vector4Int Vector4Int::xxyw() const { return { x, x, y, w }; }
inline constexpr Vector4Int Vector4Int::xxzx() const { return { x, x, z, x }; }
inline constexpr Vector4Int Vector4Int::xxzy() const { return { x, x, z, y }; }
inline constexpr Vector4Int Vector4Int::xxzz() const { return { x, x, z, z }; }
inline constexpr Vector4Int Vector4Int::xxzw() const { return { x, x, z, w }; }
inline constexpr Vector4Int Vector4Int::xxwx() const { return { x, x, w, x }; }
inline constexpr Vector4Int Vector4Int::xxwy() const { return { x, x, w, y }; }
inline constexpr Vector4Int Vector4Int::xxwz() const { return { x, x, w, z }; }
inline constexpr Vector4Int Vector4Int::xxww() const { return { x, x, w, w }; }
inline constexpr Vector4Int Vector4Int::xyxx() const { return { x, y, x, x }; }
inline constexpr Vector4Int Vector4Int::xyxy() const { return { x, y, x, y }; }
inline constexpr Vector4Int Vector4Int::xyxz() const { return { x, y, x, z }; }
inline constexpr Vector4Int Vector4Int::xyxw() const { return { x, y, x, w }; }
inline constexpr Vector4Int Vector4Int::xyyx() const { return { x, y, y, x }; }
inline constexpr Vector4Int Vector4Int::xyyy() const { return { x, y, y, y }; }
inline constexpr Vector4Int Vector4Int::xyyz() const { return { x, y, y, z }; }
inline constexpr Vector4Int Vector4Int::xyyw() const { return { x, y, y, w }; }
inline constexpr Vector4Int Vector4Int::xyzx() const { return { x, y, z, x }; }
inline constexpr Vector4Int Vector4Int::xyzy() const { return { x, y, z, y }; }
inline constexpr Vector4Int Vector4Int::xyzz() const { return { x, y, z, z }; }
inline constexpr Vector4Int Vector4Int::xyzw() const { return { x, y, z, w }; }
inline constexpr Vector4Int Vector4Int::xywx() const { return { x, y, w, x }; }
inline constexpr Vector4Int Vector4Int::xywy() const { return { x, y, w, y }; }
inline constexpr Vector4Int Vector4Int::xywz() const { return { x, y, w, z }; }
inline constexpr Vector4Int Vector4Int::xyww() const { return { x, y, w, w }; }
inline constexpr Vector4Int Vector4Int::xzxx() const { return { x, z, x, x }; }
inline constexpr Vector4Int Vector4Int::xzxy() const { return { x, z, x, y }; }
inline constexpr Vector4Int Vector4Int::xzxz() const { return { x, z, x, z }; }
inline constexpr Vector4Int Vector4Int::xzxw() const { return { x, z, x, w }; }
inline constexpr Vector4Int Vector4Int::xzyx() const { return { x, z, y, x }; }
inline constexpr Vector4Int Vector4Int::xzyy() const { return { x, z, y, y }; }
inline constexpr Vector4Int Vector4Int::xzyz() const { return { x, z, y, z }; }
inline constexpr Vector4Int Vector4Int::xzyw() const { return { x, z, y, w }; }
inline constexpr Vector4Int Vector4Int::xzzx() const { return { x, z, z, x }; }
inline constexpr Vector4Int Vector4Int::xzzy() const { return { x, z, z, y }; }
inline constexpr Vector4Int Vector4Int::xzzz() const { return { x, z, z, z }; }
inline constexpr Vector4Int Vector4Int::xzzw() const { return { x, z, z, w }; }
inline constexpr Vector4Int Vector4Int::xzwx() const { return { x, z, w, x }; }
inline constexpr Vector4Int Vector4Int::xzwy() const { return { x, z, w, y }; }
inline constexpr Vector4Int Vector4Int::xzwz() const { return { x, z, w, z }; }
inline constexpr Vector4Int Vector4Int::xzww() const { return { x, z, w, w }; }
inline constexpr Vector4Int Vector4Int::xwxx() const { return { x, w, x, x }; }
inline constexpr Vector4Int Vector4Int::xwxy() const { return { x, w, x, y }; }
inline constexpr Vector4Int Vector4Int::xwxz() const { return { x, w, x, z }; }
inline constexpr Vector4Int Vector4Int::xwxw() const { return { x, w, x, w }; }
inline constexpr Vector4Int Vector4Int::xwyx() const { return { x, w, y, x }; }
inline constexpr Vector4Int Vector4Int::xwyy() const { return { x, w, y, y }; }
inline constexpr Vector4Int Vector4Int::xwyz() const { return { x, w, y, z }; }
inline constexpr Vector4Int Vector4Int::xwyw() const { return { x, w, y, w }; }
inline constexpr Vector4Int Vector4Int::xwzx() const { return { x, w, z, x }; }
inline constexpr Vector4Int Vector4Int::xwzy() const { return { x, w, z, y }; }
inline constexpr Vector4Int Vector4Int::xwzz() const { return { x, w, z, z }; }
inline constexpr Vector4Int Vector4Int::xwzw() const { return { x, w, z, w }; }
inline constexpr Vector4Int Vector4Int::xwwx() const { return { x, w, w, x }; }
inline constexpr Vector4Int Vector4Int::xwwy() const { return { x, w, w, y }; }
inline constexpr Vector4Int Vector4Int::xwwz() const { return { x, w, w, z }; }
inline constexpr Vector4Int Vector4Int::xwww() const { return { x, w, w, w }; }
inline constexpr Vector4Int Vector4Int::yxxx() const { return { y, x, x, x }; }
inline constexpr Vector4Int Vector4Int::yxxy() const { return { y, x, x, y }; }
inline constexpr Vector4Int Vector4Int::yxxz() const { return { y, x, x, z }; }
inline constexpr Vector4Int Vector4Int::yxxw() const { return { y, x, x, w }; }
inline constexpr Vector4Int Vector4Int::yxyx() const { return { y, x, y, x }; }
inline constexpr Vector4Int Vector4Int::yxyy() const { return { y, x, y, y }; }
inline constexpr Vector4Int Vector4Int::yxyz() const { return { y, x, y, z }; }
inline constexpr Vector4Int Vector4Int::yxyw() const { return { y, x, y, w }; }
inline constexpr Vector4Int Vector4Int::yxzx() const { return { y, x, z, x }; }
inline constexpr Vector4Int Vector4Int::yxzy() const { return { y, x, z, y }; }
inline constexpr Vector4Int Vector4Int::yxzz() const { return { y, x, z, z }; }
inline constexpr Vector4Int Vector4Int::yxzw() const { return { y, x, z, w }; }
inline constexpr Vector4Int Vector4Int::yxwx() const { return { y, x, w, x }; }
inline constexpr Vector4Int Vector4Int::yxwy() const { return { y, x, w, y }; }
inline constexpr Vector4Int Vector4Int::yxwz() const { return { y, x, w, z }; }
inline constexpr Vector4Int Vector4Int::yxww() const { return { y, x, w, w }; }
inline constexpr Vector4Int Vector4Int::yyxx() const { return { y, y, x, x }; }
inline constexpr Vector4Int Vector4Int::yyxy() const { return { y, y, x, y }; }
inline constexpr Vector4Int Vector4Int::yyxz() const { return { y, y, x, z }; }
inline constexpr Vector4Int Vector4Int::yyxw() const { return { y, y, x, w }; }
inline constexpr Vector4Int Vector4Int::yyyx() const { return { y, y, y, x }; }
inline constexpr Vector4Int Vector4Int::yyyy() const { return { y, y, y, y }; }
inline constexpr Vector4Int Vector4Int::yyyz() const { return { y, y, y, z }; }
inline constexpr Vector4Int Vector4Int::yyyw() const { return { y, y, y, w }; }
inline constexpr Vector4Int Vector4Int::yyzx() const { return { y, y, z, x }; }
inline constexpr Vector4Int Vector4Int::yyzy() const { return { y, y, z, y }; }
inline constexpr Vector4Int Vector4Int::yyzz() const { return { y, y, z, z }; }
inline constexpr Vector4Int Vector4Int::yyzw() const { return { y, y, z, w }; }
inline constexpr Vector4Int Vector4Int::yywx() const { return { y, y, w, x }; }
inline constexpr Vector4Int Vector4Int::yywy() const { return { y, y, w, y }; }
inline constexpr Vector4Int Vector4Int::yywz() const { return { y, y, w, z }; }
inline constexpr Vector4Int Vector4Int::yyww() const { return { y, y, w, w }; }
inline constexpr Vector4Int Vector4Int::yzxx() const { return { y, z, x, x }; }
inline constexpr Vector4Int Vector4Int::yzxy() const { return { y, z, x, y }; }
inline constexpr Vector4Int Vector4Int::yzxz() const { return { y, z, x, z }; }
inline constexpr Vector4Int Vector4Int::yzxw() const { return { y, z, x, w }; }
inline constexpr Vector4Int Vector4Int::yzyx() const { return { y, z, y, x }; }
inline constexpr Vector4Int Vector4Int::yzyy() const { return { y, z, y, y }; }
inline constexpr Vector4Int Vector4Int::yzyz() const { return { y, z, y, z }; }
inline constexpr Vector4Int Vector4Int::yzyw() const { return { y, z, y, w }; }
inline constexpr Vector4Int Vector4Int::yzzx() const { return { y, z, z, x }; }
inline constexpr Vector4Int Vector4Int::yzzy() const { return { y, z, z, y }; }
inline constexpr Vector4Int Vector4Int::yzzz() const { return { y, z, z, z }; }
inline constexpr Vector4Int Vector4Int::yzzw() const { return { y, z, z, w }; }
inline constexpr Vector4Int Vector4Int::yzwx() const { return { y, z, w, x }; }
inline constexpr Vector4Int Vector4Int::yzwy() const { return { y, z, w, y }; }
inline constexpr Vector4Int Vector4Int::yzwz() const { return { y, z, w, z }; }
inline constexpr Vector4Int Vector4Int::yzww() const { return { y, z, w, w }; }
inline constexpr Vector4Int Vector4Int::ywxx() const { return { y, w, x, x }; }
inline constexpr Vector4Int Vector4Int::ywxy() const { return { y, w, x, y }; }
inline constexpr Vector4Int Vector4Int::ywxz() const { return { y, w, x, z }; }
inline constexpr Vector4Int Vector4Int::ywxw() const { return { y, w, x, w }; }
inline constexpr Vector4Int Vector4Int::ywyx() const { return { y, w, y, x }; }
inline constexpr Vector4Int Vector4Int::ywyy() const { return { y, w, y, y }; }
inline constexpr Vector4Int Vector4Int::ywyz() const { return { y, w, y, z }; }
inline constexpr Vector4Int Vector4Int::ywyw() const { return { y, w, y, w }; }
inline constexpr Vector4Int Vector4Int::ywzx() const { return { y, w, z, x }; }
inline constexpr Vector4Int Vector4Int::ywzy() const { return { y, w, z, y }; }
inline constexpr Vector4Int Vector4Int::ywzz() const { return { y, w, z, z }; }
inline constexpr Vector4Int Vector4Int::ywzw() const { return { y, w, z, w }; }
inline constexpr Vector4Int Vector4Int::ywwx() const { return { y, w, w, x }; }
inline constexpr Vector4Int Vector4Int::ywwy() const { return { y, w, w, y }; }
inline constexpr Vector4Int Vector4Int::ywwz() const { return { y, w, w, z }; }
inline constexpr Vector4Int Vector4Int::ywww() const { return { y, w, w, w }; }
inline constexpr Vector4Int Vector4Int::zxxx() const { return { z, x, x, x }; }
inline constexpr Vector4Int Vector4Int::zxxy() const { return { z, x, x, y }; }
inline constexpr Vector4Int Vector4Int::zxxz() const { return { z, x, x, z }; }
inline constexpr Vector4Int Vector4Int::zxxw() const { return { z, x, x, w }; }
inline constexpr Vector4Int Vector4Int::zxyx() const { return { z, x, y, x }; }
inline constexpr Vector4Int Vector4Int::zxyy() const { return { z, x, y, y }; }
inline constexpr Vector4Int Vector4Int::zxyz() const { return { z, x, y, z }; }
inline constexpr Vector4Int Vector4Int::zxyw() const { return { z, x, y, w }; }
inline constexpr Vector4Int Vector4Int::zxzx() const { return { z, x, z, x }; }
inline constexpr Vector4Int Vector4Int::zxzy() const { return { z, x, z, y }; }
inline constexpr Vector4Int Vector4Int::zxzz() const { return { z, x, z, z }; }
inline constexpr Vector4Int Vector4Int::zxzw() const { return { z, x, z, w }; }
inline constexpr Vector4Int Vector4Int::zxwx() const { return { z, x, w, x }; }
inline constexpr Vector4Int Vector4Int::zxwy() const { return { z, x, w, y }; }
inline constexpr Vector4Int Vector4Int::zxwz() const { return { z, x, w, z }; }
inline constexpr Vector4Int Vector4Int::zxww() const { return { z, x, w, w }; }
inline constexpr Vector4Int Vector4Int::zyxx() const { return { z, y, x, x }; }
inline constexpr Vector4Int Vector4Int::zyxy() const { return { z, y, x, y }; }
inline constexpr Vector4Int Vector4Int::zyxz() const { return { z, y, x, z }; }
inline constexpr Vector4Int Vector4Int::zyxw() const { return { z, y, x, w }; }
inline constexpr Vector4Int Vector4Int::zyyx() const { return { z, y, y, x }; }
inline constexpr Vector4Int Vector4Int::zyyy() const { return { z, y, y, y }; }
inline constexpr Vector4Int Vector4Int::zyyz() const { return { z, y, y, z }; }
inline constexpr Vector4Int Vector4Int::zyyw() const { return { z, y, y, w }; }
inline constexpr Vector4Int Vector4Int::zyzx() const { return { z, y, z, x }; }
inline constexpr Vector4Int Vector4Int::zyzy() const { return { z, y, z, y }; }
inline constexpr Vector4Int Vector4Int::zyzz() const { return { z, y, z, z }; }
inline constexpr Vector4Int Vector4Int::zyzw() const { return { z, y, z, w }; }
inline constexpr Vector4Int Vector4Int::zywx() const { return { z, y, w, x }; }
inline constexpr Vector4Int Vector4Int::zywy() const { return { z, y, w, y }; }
inline constexpr Vector4Int Vector4Int::zywz() const { return { z, y, w, z }; }
inline constexpr Vector4Int Vector4Int::zyww() const { return { z, y, w, w }; }
inline constexpr Vector4Int Vector4Int::zzxx() const { return { z, z, x, x }; }
inline constexpr Vector4Int Vector4Int::zzxy() const { return { z, z, x, y }; }
inline constexpr Vector4Int Vector4Int::zzxz() const { return { z, z, x, z }; }
inline constexpr Vector4Int Vector4Int::zzxw() const { return { z, z, x, w }; }
inline constexpr Vector4Int Vector4Int::zzyx() const { return { z, z, y, x }; }
inline constexpr Vector4Int Vector4Int::zzyy() const { return { z, z, y, y }; }
inline constexpr Vector4Int Vector4Int::zzyz() const { return { z, z, y, z }; }
inline constexpr Vector4Int Vector4Int::zzyw() const { return { z, z, y, w }; }
inline constexpr Vector4Int Vector4Int::zzzx() const { return { z, z, z, x }; }
inline constexpr Vector4Int Vector4Int::zzzy() const { return { z, z, z, y }; }
inline constexpr Vector4Int Vector4Int::zzzz() const { return { z, z, z, z }; }
inline constexpr Vector4Int Vector4Int::zzzw() const { return { z, z, z, w }; }
inline constexpr Vector4Int Vector4Int::zzwx() const { return { z, z, w, x }; }
inline constexpr Vector4Int Vector4Int::zzwy() const { return { z, z, w, y }; }
inline constexpr Vector4Int Vector4Int::zzwz() const { return { z, z, w, z }; }
inline constexpr Vector4Int Vector4Int::zzww() const { return { z, z, w, w }; }
inline constexpr Vector4Int Vector4Int::zwxx() const { return { z, w, x, x }; }
inline constexpr Vector4Int Vector4Int::zwxy() const { return { z, w, x, y }; }
inline constexpr Vector4Int Vector4Int::zwxz() const { return { z, w, x, z }; }
inline constexpr Vector4Int Vector4Int::zwxw() const { return { z, w, x, w }; }
inline constexpr Vector4Int Vector4Int::zwyx() const { return { z, w, y, x }; }
inline constexpr Vector4Int Vector4Int::zwyy() const { return { z, w, y, y }; }
inline constexpr Vector4Int Vector4Int::zwyz() const { return { z, w, y, z }; }
inline constexpr Vector4Int Vector4Int::zwyw() const { return { z, w, y, w }; }
inline constexpr Vector4Int Vector4Int::zwzx() const { return { z, w, z, x }; }
inline constexpr Vector4Int Vector4Int::zwzy() const { return { z, w, z, y }; }
inline constexpr Vector4Int Vector4Int::zwzz() const { return { z, w, z, z }; }
inline constexpr Vector4Int Vector4Int::zwzw() const { return { z, w, z, w }; }
inline constexpr Vector4Int Vector4Int::zwwx() const { return { z, w, w, x }; }
inline constexpr Vector4Int Vector4Int::zwwy() const { return { z, w, w, y }; }
inline constexpr Vector4Int Vector4Int::zwwz() const { return { z, w, w, z }; }
inline constexpr Vector4Int Vector4Int::zwww() const { return { z, w, w, w }; }
inline constexpr Vector4Int Vector4Int::wxxx() const { return { w, x, x, x }; }
inline constexpr Vector4Int Vector4Int::wxxy() const { return { w, x, x, y }; }
inline constexpr Vector4Int Vector4Int::wxxz() const { return { w, x, x, z }; }
inline constexpr Vector4Int Vector4Int::wxxw() const { return { w, x, x, w }; }
inline constexpr Vector4Int Vector4Int::wxyx() const { return { w, x, y, x }; }
inline constexpr Vector4Int Vector4Int::wxyy() const { return { w, x, y, y }; }
inline constexpr Vector4Int Vector4Int::wxyz() const { return { w, x, y, z }; }
inline constexpr Vector4Int Vector4Int::wxyw() const { return { w, x, y, w }; }
inline constexpr Vector4Int Vector4Int::wxzx() const { return { w, x, z, x }; }
inline constexpr Vector4Int Vector4Int::wxzy() const { return { w, x, z, y }; }
inline constexpr Vector4Int Vector4Int::wxzz() const { return { w, x, z, z }; }
inline constexpr Vector4Int Vector4Int::wxzw() const { return { w, x, z, w }; }
inline constexpr Vector4Int Vector4Int::wxwx() const { return { w, x, w, x }; }
inline constexpr Vector4Int Vector4Int::wxwy() const { return { w, x, w, y }; }
inline constexpr Vector4Int Vector4Int::wxwz() const { return { w, x, w, z }; }
inline constexpr Vector4Int Vector4Int::wxww() const { return { w, x, w, w }; }
inline constexpr Vector4Int Vector4Int::wyxx() const { return { w, y, x, x }; }
inline constexpr Vector4Int Vector4Int::wyxy() const { return { w, y, x, y }; }
inline constexpr Vector4Int Vector4Int::wyxz() const { return { w, y, x, z }; }
inline constexpr Vector4Int Vector4Int::wyxw() const { return { w, y, x, w }; }
inline constexpr Vector4Int Vector4Int::wyyx() const { return { w, y, y, x }; }
inline constexpr Vector4Int Vector4Int::wyyy() const { return { w, y, y, y }; }
inline constexpr Vector4Int Vector4Int::wyyz() const { return { w, y, y, z }; }
inline constexpr Vector4Int Vector4Int::wyyw() const { return { w, y, y, w }; }
inline constexpr Vector4Int Vector4Int::wyzx() const { return { w, y, z, x }; }
inline constexpr Vector4Int Vector4Int::wyzy() const { return { w, y, z, y }; }
inline constexpr Vector4Int Vector4Int::wyzz() const { return { w, y, z, z }; }
inline constexpr Vector4Int Vector4Int::wyzw() const { return { w, y, z, w }; }
inline constexpr Vector4Int Vector4Int::wywx() const { return { w, y, w, x }; }
inline constexpr Vector4Int Vector4Int::wywy() const { return { w, y, w, y }; }
inline constexpr Vector4Int Vector4Int::wywz() const { return { w, y, w, z }; }
inline constexpr Vector4Int Vector4Int::wyww() const { return { w, y, w, w }; }
inline constexpr Vector4Int Vector4Int::wzxx() const { return { w, z, x, x }; }
inline constexpr Vector4Int Vector4Int::wzxy() const { return { w, z, x, y }; }
inline constexpr Vector4Int Vector4Int::wzxz() const { return { w, z, x, z }; }
inline constexpr Vector4Int Vector4Int::wzxw() const { return { w, z, x, w }; }
inline constexpr Vector4Int Vector4Int::wzyx() const { return { w, z, y, x }; }
inline constexpr Vector4Int Vector4Int::wzyy() const { return { w, z, y, y }; }
inline constexpr Vector4Int Vector4Int::wzyz() const { return { w, z, y, z }; }
inline constexpr Vector4Int Vector4Int::wzyw() const { return { w, z, y, w }; }
inline constexpr Vector4Int Vector4Int::wzzx() const { return { w, z, z, x }; }
inline constexpr Vector4Int Vector4Int::wzzy() const { return { w, z, z, y }; }
inline constexpr Vector4Int Vector4Int::wzzz() const { return { w, z, z, z }; }
inline constexpr Vector4Int Vector4Int::wzzw() const { return { w, z, z, w }; }
inline constexpr Vector4Int Vector4Int::wzwx() const { return { w, z, w, x }; }
inline constexpr Vector4Int Vector4Int::wzwy() const { return { w, z, w, y }; }
inline constexpr Vector4Int Vector4Int::wzwz() const { return { w, z, w, z }; }
inline constexpr Vector4Int Vector4Int::wzww() const { return { w, z, w, w }; }
inline constexpr Vector4Int Vector4Int::wwxx() const { return { w, w, x, x }; }
inline constexpr Vector4Int Vector4Int::wwxy() const { return { w, w, x, y }; }
inline constexpr Vector4Int Vector4Int::wwxz() const { return { w, w, x, z }; }
inline constexpr Vector4Int Vector4Int::wwxw() const { return { w, w, x, w }; }
inline constexpr Vector4Int Vector4Int::wwyx() const { return { w, w, y, x }; }
inline constexpr Vector4Int Vector4Int::wwyy() const { return { w, w, y, y }; }
inline constexpr Vector4Int Vector4Int::wwyz() const { return { w, w, y, z }; }
inline constexpr Vector4Int Vector4Int::wwyw() const { return { w, w, y, w }; }
inline constexpr Vector4Int Vector4Int::wwzx() const { return { w, w, z, x }; }
inline constexpr Vector4Int Vector4Int::wwzy() const { return { w, w, z, y }; }
inline constexpr Vector4Int Vector4Int::wwzz() const { return { w, w, z, z }; }
inline constexpr Vector4Int Vector4Int::wwzw() const { return { w, w, z, w }; }
inline constexpr Vector4Int Vector4Int::wwwx() const { return { w, w, w, x }; }
inline constexpr Vector4Int Vector4Int::wwwy() const { return { w, w, w, y }; }
inline constexpr Vector4Int Vector4Int::wwwz() const { return { w, w, w, z }; }
inline constexpr Vector4Int Vector4Int::wwww() const { return { w, w, w, w }; }
#pragma endregion

inline constexpr Vector4Int::operator Vector2() const { return { (F32)x, (F32)y }; }
inline constexpr Vector4Int::operator Vector3() const { return { (F32)x, (F32)y, (F32)z }; }
inline constexpr Vector4Int::operator Vector4() const { return { (F32)x, (F32)y, (F32)z, (F32)w }; }
inline constexpr Vector4Int::operator Vector2Int() const { return { x, y }; }
inline constexpr Vector4Int::operator Vector3Int() const { return { x, y, z }; }
#pragma endregion

#pragma region Matrix2
inline constexpr Matrix2::Matrix2() : a(1.0f, 0.0f), b(0.0f, 1.0f) {}
inline constexpr Matrix2::Matrix2(F32 ax, F32 ay, F32 bx, F32 by) : a(ax, ay), b(bx, by) {}
inline constexpr Matrix2::Matrix2(const Vector2& v) : a(v.x, 0.0f), b(0.0f, v.y) {}
inline constexpr Matrix2::Matrix2(const Vector2& a, const Vector2& b) : a(a), b(b) {}
inline constexpr Matrix2::Matrix2(Vector2&& a, Vector2&& b) noexcept : a(a), b(b) {}
inline constexpr Matrix2::Matrix2(const Matrix2& m) : a(m.a), b(m.b) {}
inline constexpr Matrix2::Matrix2(Matrix2&& m) noexcept : a(m.a), b(m.b) {}

inline constexpr Matrix2& Matrix2::operator= (const Matrix2& m) { a = m.a; b = m.b; return *this; }
inline constexpr Matrix2& Matrix2::operator= (Matrix2&& m) noexcept { a = m.a; b = m.b; return *this; }

inline constexpr Matrix2& Matrix2::operator+= (const Matrix2& m) { a += m.a; b += m.b; return *this; }
inline constexpr Matrix2& Matrix2::operator-= (const Matrix2& m) { a -= m.a; b -= m.b; return *this; }
inline constexpr Matrix2& Matrix2::operator*= (const Matrix2& m)
{
	a.x = a.x * m.a.x + b.x * m.a.y;
	a.y = a.y * m.a.x + b.y * m.a.y;
	b.x = a.x * m.b.x + b.x * m.b.y;
	b.y = a.y * m.b.x + b.y * m.b.y;

	return *this;
}

inline constexpr Matrix2 Matrix2::operator+(const Matrix2& m) const { return { a + m.a, b + m.b }; }
inline constexpr Matrix2 Matrix2::operator-(const Matrix2& m) const { return { a - m.a, b - m.b }; }
inline constexpr Matrix2 Matrix2::operator*(const Matrix2& m) const
{
	return {
		a.x * m.a.x + b.x * m.a.y,
		a.y * m.a.x + b.y * m.a.y,
		a.x * m.b.x + b.x * m.b.y,
		a.y * m.b.x + b.y * m.b.y,
	};
}
inline constexpr Vector2 Matrix2::operator*(const Vector2& v) const
{
	return {
		a.x * v.x + b.x * v.y,
		a.y * v.x + b.y * v.y,
	};
}

inline constexpr Vector2 Matrix2::Solve(const Vector2& v) const
{
	F32 det = a.x * b.y - b.x * a.y;
	if (!Math::IsZero(det)) { det = 1.0f / det; }
	return { det * (b.y * b.x - b.x * b.y), det * (a.x * b.y - a.y * b.x) };
}

inline constexpr Matrix2 Matrix2::Inverse() const
{
	F32 determinant = a.x * b.y - b.x * a.y;

	if (Math::IsZero(determinant)) { return Matrix2{ }; }
	F32 f = 1.0f / determinant;

	return {
		 b.y * f, -a.y * f,
		-b.x * f,  a.x * f
	};
}
inline constexpr Matrix2& Matrix2::Inversed()
{
	F32 determinant = a.x * b.y - b.x * a.y;

	if (Math::IsZero(determinant)) { return *this = Matrix2{}; }
	F32 f = 1.0f / determinant;

	F32 ax = b.y * f;

	a.x = ax;
	a.y = -a.y * f;
	b.x = -b.x * f;
	b.y = a.x * f;

	return *this;
}
inline constexpr Matrix2 Matrix2::Transpose() const
{
	return {
		a.x, b.x,
		a.y, b.y
	};
}
inline constexpr Matrix2& Matrix2::Transposed()
{
	F32 bx = a.y;

	a.y = b.x;
	b.x = bx;

	return *this;
}

inline constexpr Matrix2 Matrix2::operator-() const { return { -a, -b }; }
inline constexpr Matrix2 Matrix2::operator~() const { return { ~a, ~b }; }
inline constexpr Matrix2 Matrix2::operator!() const { return { !a, !b }; }

inline constexpr bool Matrix2::operator==(const Matrix2& m) const { return a == m.a && b == m.b; }
inline constexpr bool Matrix2::operator!=(const Matrix2& m) const { return a != m.a || b != m.b; }

inline const Vector2& Matrix2::operator[] (U8 i) const { return (&a)[i]; }
inline Vector2& Matrix2::operator[] (U8 i) { return (&a)[i]; }

inline F32* Matrix2::Data() { return a.Data(); }
inline const F32* Matrix2::Data() const { return a.Data(); }
#pragma endregion

#pragma region Matrix3
inline constexpr Matrix3::Matrix3() : a(1.0f, 0.0f, 0.0f), b(0.0f, 1.0f, 0.0f), c(0.0f, 0.0f, 1.0f) {}
inline constexpr Matrix3::Matrix3(F32 ax, F32 ay, F32 az, F32 bx, F32 by, F32 bz, F32 cx, F32 cy, F32 cz) : a(ax, ay, az), b(bx, by, bz), c(cx, cy, cz) {}
inline constexpr Matrix3::Matrix3(const Vector3& v) : a(v.x, 0.0f, 0.0f), b(0.0f, v.y, 0.0f), c(0.0f, 0.0f, v.z) {}
inline constexpr Matrix3::Matrix3(const Vector3& a, const Vector3& b, const Vector3& c) : a(a), b(b), c(c) {}
inline constexpr Matrix3::Matrix3(Vector3&& v1, Vector3&& v2, Vector3&& v3) noexcept : a(v1), b(v2), c(v3) {}
inline constexpr Matrix3::Matrix3(const Matrix3& m) : a(m.a), b(m.b), c(m.c) {}
inline constexpr Matrix3::Matrix3(Matrix3&& m) noexcept : a(m.a), b(m.b), c(m.c) {}
inline constexpr Matrix3::Matrix3(const Vector2& position, const F32& rotation, const Vector2& scale)
{
	F32 cos = Math::Cos(rotation * (F32)DegToRad);
	F32 sin = Math::Sin(rotation * (F32)DegToRad);
	a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
	a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
	a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
}
inline constexpr Matrix3::Matrix3(const Vector2& position, const Quaternion2& rotation, const Vector2& scale)
{
	a.x = rotation.y * scale.x;	b.x = -rotation.x;			c.x = position.x;
	a.y = rotation.x;			b.y = rotation.y * scale.y;	c.y = position.y;
	a.z = 0.0f;					b.z = 0.0f;					c.z = 1.0f;
}

inline constexpr Matrix3& Matrix3::operator= (const Matrix3& m) { a = m.a; b = m.b; c = m.c; return *this; }
inline constexpr Matrix3& Matrix3::operator= (Matrix3&& m) noexcept { a = m.a; b = m.b; c = m.c; return *this; }
inline constexpr void Matrix3::Set(const Vector2& position, const F32& rotation, const Vector2& scale)
{
	F32 cos = Math::Cos(rotation * (F32)DegToRad);
	F32 sin = Math::Sin(rotation * (F32)DegToRad);
	a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
	a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
	a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
}
inline constexpr void Matrix3::Set(const Vector2& position, const Quaternion2& rotation, const Vector2& scale)
{
	a.x = rotation.y * scale.x;	b.x = -rotation.x;			c.x = position.x;
	a.y = rotation.x;			b.y = rotation.y * scale.y;	c.y = position.y;
	a.z = 0.0f;					b.z = 0.0f;					c.z = 1.0f;
}

inline constexpr Matrix3& Matrix3::operator+= (const Matrix3& m) { a += m.a; b += m.b; c += m.c; return *this; }
inline constexpr Matrix3& Matrix3::operator-= (const Matrix3& m) { a -= m.a; b -= m.b; c -= m.c; return *this; }
inline constexpr Matrix3& Matrix3::operator*= (const Matrix3& m)
{
	a.x = a.x * m.a.x + b.x * m.a.y + c.x * m.a.z;
	a.y = a.y * m.a.x + b.y * m.a.y + c.y * m.a.z;
	a.z = a.z * m.a.x + b.z * m.a.y + c.z * m.a.z;
	b.x = a.x * m.b.x + b.x * m.b.y + c.x * m.b.z;
	b.y = a.y * m.b.x + b.y * m.b.y + c.y * m.b.z;
	b.z = a.z * m.b.x + b.z * m.b.y + c.z * m.b.z;
	c.x = a.x * m.c.x + b.x * m.c.y + c.x * m.c.z;
	c.y = a.y * m.c.x + b.y * m.c.y + c.y * m.c.z;
	c.z = a.z * m.c.x + b.z * m.c.y + c.z * m.c.z;

	return *this;
}

inline constexpr Matrix3 Matrix3::operator+(const Matrix3& m) const { return { a + m.a, b + m.b, c + m.c }; }
inline constexpr Matrix3 Matrix3::operator-(const Matrix3& m) const { return { a - m.a, b - m.b, c - m.c }; }
inline constexpr Matrix3 Matrix3::operator*(const Matrix3& m) const
{
	return {
		a.x * m.a.x + b.x * m.a.y + c.x * m.a.z,
		a.y * m.a.x + b.y * m.a.y + c.y * m.a.z,
		a.z * m.a.x + b.z * m.a.y + c.z * m.a.z,
		a.x * m.b.x + b.x * m.b.y + c.x * m.b.z,
		a.y * m.b.x + b.y * m.b.y + c.y * m.b.z,
		a.z * m.b.x + b.z * m.b.y + c.z * m.b.z,
		a.x * m.c.x + b.x * m.c.y + c.x * m.c.z,
		a.y * m.c.x + b.y * m.c.y + c.y * m.c.z,
		a.z * m.c.x + b.z * m.c.y + c.z * m.c.z
	};
}
inline constexpr Vector3 Matrix3::operator*(const Vector3& v) const
{
	return {
		a.x * v.x + b.x * v.y + c.x * v.z,
		a.y * v.x + b.y * v.y + c.y * v.z,
		a.z * v.x + b.z * v.y + c.z * v.z
	};
}

inline constexpr Matrix3 Matrix3::Inverse() const
{
	F32 determinant = a.x * b.y * c.z - c.y * b.z -
		a.y * b.x * c.z - b.z * c.x +
		a.z * b.x * c.y - b.y * c.x;

	if (Math::IsZero(determinant)) { return Matrix3{ }; }
	F32 f = 1.0f / determinant;

	return {
		(b.y * c.z - c.y * b.z) * f, (a.z * c.y - a.y * c.z) * f, (a.y * b.z - a.z * b.y) * f,
		(b.z * c.x - b.x * c.z) * f, (a.x * c.z - a.z * c.x) * f, (b.x * a.z - a.x * b.z) * f,
		(b.x * c.y - c.x * b.y) * f, (c.x * a.y - a.x * c.y) * f, (a.x * b.y - b.x * a.y) * f
	};
}
inline constexpr Matrix3& Matrix3::Inversed()
{
	F32 determinant = a.x * b.y * c.z - c.y * b.z -
		a.y * b.x * c.z - b.z * c.x +
		a.z * b.x * c.y - b.y * c.x;

	if (Math::IsZero(determinant)) { return *this = Matrix3{}; }
	F32 f = 1.0f / determinant;

	F32 ax = (b.y * c.z - c.y * b.z) * f;
	F32 ay = (a.z * c.y - a.y * c.z) * f;
	F32 az = (a.y * b.z - a.z * b.y) * f;
	F32 bx = (b.z * c.x - b.x * c.z) * f;
	F32 by = (a.x * c.z - a.z * c.x) * f;
	F32 bz = (b.x * a.z - a.x * b.z) * f;
	F32 cx = (b.x * c.y - c.x * b.y) * f;
	F32 cy = (c.x * a.y - a.x * c.y) * f;
	F32 cz = (a.x * b.y - b.x * a.y) * f;

	a.x = ax;
	a.y = ay;
	a.z = az;
	b.x = bx;
	b.y = by;
	b.z = bz;
	c.x = cx;
	c.y = cy;
	c.z = cz;

	return *this;
}
inline constexpr Matrix3 Matrix3::Transpose() const
{
	return {
		a.x, b.x, c.x,
		a.y, b.y, c.y,
		a.z, b.z, c.z
	};
}
inline constexpr Matrix3& Matrix3::Transposed()
{
	F32 bx = a.y;
	F32 cx = a.z;
	F32 cy = b.z;

	a.y = b.x;
	a.z = c.x;
	b.z = c.y;

	b.x = bx;
	c.x = cx;
	c.y = cy;

	return *this;
}

inline constexpr Matrix3 Matrix3::operator-() const { return { -a, -b, -c }; }
inline constexpr Matrix3 Matrix3::operator~() const { return { ~a, ~b, ~c }; }
inline constexpr Matrix3 Matrix3::operator!() const { return { !a, !b, !c }; }

inline constexpr bool Matrix3::operator==(const Matrix3& m) const { return a == m.a && b == m.b && c == m.c; }
inline constexpr bool Matrix3::operator!=(const Matrix3& m) const { return a != m.a || b != m.b || c != m.c; }

inline const Vector3& Matrix3::operator[] (U8 i) const { return (&a)[i]; }
inline Vector3& Matrix3::operator[] (U8 i) { return (&a)[i]; }

inline F32* Matrix3::Data() { return a.Data(); }
inline const F32* Matrix3::Data() const { return a.Data(); }
#pragma endregion

#pragma region Matrix4
inline constexpr Matrix4 Matrix4::Translate(const Vector3& position)
{
	return {
		1.0f, 0.0f, 0.0f, position.x,
		0.0f, 1.0f, 0.0f, position.y,
		0.0f, 0.0f, 1.0f, position.z,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}
inline constexpr Matrix4 Matrix4::Rotate(const Vector3& rotation)
{
	Quaternion3 q = rotation;

	return q.ToMatrix4();
}
inline constexpr Matrix4 Matrix4::Rotate(const Matrix4& m, F32 angle, const Vector3& axis)
{
	F32 a = angle * (F32)DegToRad;

	F32 y = Math::Cos(a);
	F32 z = Math::Sin(a);

	Vector3 ax = axis.Normalized();
	Vector3 temp = ax * (1.0f - y);

	Matrix4 rotate;
	rotate.a.x = y + temp.x * ax.x;
	rotate.a.y = temp.x * ax.y + z * ax.z;
	rotate.a.z = temp.x * ax.z - z * ax.y;

	rotate.b.x = temp.y * ax.x - z * ax.z;
	rotate.b.y = y + temp.y * ax.y;
	rotate.b.z = temp.y * ax.z + z * ax.x;

	rotate.c.x = temp.z * ax.x + z * ax.y;
	rotate.c.y = temp.z * ax.y - z * ax.x;
	rotate.c.z = y + temp.z * ax.z;

	return {
		m.a * rotate.a.x + m.b * rotate.a.y + m.c * rotate.a.z,
		m.a * rotate.b.x + m.b * rotate.b.y + m.c * rotate.b.z,
		m.a * rotate.c.x + m.b * rotate.c.y + m.c * rotate.c.z,
		m.d
	};
}
inline constexpr Matrix4 Matrix4::Rotate(const Quaternion3& rotation)
{
	return rotation.ToMatrix4();
}
inline constexpr Matrix4 Matrix4::Scale(const Vector3& scale)
{
	return {
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

inline constexpr Matrix4::Matrix4() : a(1.0f, 0.0f, 0.0f, 0.0f), b(0.0f, 1.0f, 0.0f, 0.0f), c(0.0f, 0.0f, 1.0f, 0.0f), d(0.0f, 0.0f, 0.0f, 1.0f) {}
inline constexpr Matrix4::Matrix4(F32 ax, F32 ay, F32 az, F32 aw, F32 bx, F32 by, F32 bz, F32 bw, F32 cx, F32 cy, F32 cz, F32 cw, F32 dx, F32 dy, F32 dz, F32 dw) :
	a(ax, ay, az, aw), b(bx, by, bz, bw), c(cx, cy, cz, cw), d(dx, dy, dz, dw)
{
}
inline constexpr Matrix4::Matrix4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d) : a(a), b(b), c(c), d(d) {}
inline constexpr Matrix4::Matrix4(Vector4&& a, Vector4&& b, Vector4&& c, Vector4&& d) noexcept : a(a), b(b), c(c), d(d) {}
inline constexpr Matrix4::Matrix4(const Matrix4& m) : a(m.a), b(m.b), c(m.c), d(m.d) {}
inline constexpr Matrix4::Matrix4(const Matrix3& m) : a(m.a.x, m.a.y, 0.0f, m.a.z), b(m.b.x, m.b.y, 0.0f, m.b.z), c(0.0f, 0.0f, 1.0f, m.b.z), d(m.c.x, m.c.y, 0.0f, 1.0f) {}
inline constexpr Matrix4::Matrix4(Matrix4&& m) noexcept : a(m.a), b(m.b), c(m.c), d(m.d) {}
inline constexpr Matrix4::Matrix4(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	Quaternion3 q = rotation;

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}
inline constexpr Matrix4::Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
{
	Quaternion3 q = rotation.Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}

inline constexpr Matrix4& Matrix4::operator= (const Matrix4& m) { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
inline constexpr Matrix4& Matrix4::operator= (Matrix4&& m) noexcept { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
inline constexpr Matrix4& Matrix4::operator+= (const Matrix4& m) { a += m.a; b += m.b; c += m.c; d += m.d; return *this; }
inline constexpr Matrix4& Matrix4::operator-= (const Matrix4& m) { a -= m.a; b -= m.b; c -= m.c; d -= m.d; return *this; }
inline constexpr Matrix4& Matrix4::operator*= (const Matrix4& m)
{
	a.x = a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + c.x * m.a.w;
	a.y = a.y * m.a.x + b.y * m.a.y + c.y * m.a.z + c.y * m.a.w;
	a.z = a.z * m.a.x + b.z * m.a.y + c.z * m.a.z + c.z * m.a.w;
	a.w = a.w * m.a.x + b.w * m.a.y + c.w * m.a.z + c.w * m.a.w;
	b.x = a.x * m.b.x + b.x * m.b.y + c.x * m.b.z + c.x * m.b.w;
	b.y = a.y * m.b.x + b.y * m.b.y + c.y * m.b.z + c.y * m.b.w;
	b.z = a.z * m.b.x + b.z * m.b.y + c.z * m.b.z + c.z * m.b.w;
	b.w = a.w * m.b.x + b.w * m.b.y + c.w * m.b.z + c.w * m.b.w;
	c.x = a.x * m.c.x + b.x * m.c.y + c.x * m.c.z + c.x * m.c.w;
	c.y = a.y * m.c.x + b.y * m.c.y + c.y * m.c.z + c.y * m.c.w;
	c.z = a.z * m.c.x + b.z * m.c.y + c.z * m.c.z + c.z * m.c.w;
	c.w = a.w * m.c.x + b.w * m.c.y + c.w * m.c.z + c.w * m.c.w;
	d.x = a.x * m.d.x + b.x * m.d.y + c.x * m.d.z + c.x * m.d.w;
	d.y = a.y * m.d.x + b.y * m.d.y + c.y * m.d.z + c.y * m.d.w;
	d.z = a.z * m.d.x + b.z * m.d.y + c.z * m.d.z + c.z * m.d.w;
	d.w = a.w * m.d.x + b.w * m.d.y + c.w * m.d.z + c.w * m.d.w;

	return *this;
}

inline constexpr Matrix4 Matrix4::operator+(const Matrix4& m) const { return { a + m.a, b + m.b, c + m.c, d + m.d }; }
inline constexpr Matrix4 Matrix4::operator-(const Matrix4& m) const { return { a - m.a, b - m.b, c - m.c, d + m.d }; }
inline constexpr Matrix4 Matrix4::operator*(const Matrix4& m) const
{
	return {
		a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + d.x * m.a.w,
		a.y * m.a.x + b.y * m.a.y + c.y * m.a.z + d.y * m.a.w,
		a.z * m.a.x + b.z * m.a.y + c.z * m.a.z + d.z * m.a.w,
		a.w * m.a.x + b.w * m.a.y + c.w * m.a.z + d.w * m.a.w,
		a.x * m.b.x + b.x * m.b.y + c.x * m.b.z + d.x * m.b.w,
		a.y * m.b.x + b.y * m.b.y + c.y * m.b.z + d.y * m.b.w,
		a.z * m.b.x + b.z * m.b.y + c.z * m.b.z + d.z * m.b.w,
		a.w * m.b.x + b.w * m.b.y + c.w * m.b.z + d.w * m.b.w,
		a.x * m.c.x + b.x * m.c.y + c.x * m.c.z + d.x * m.c.w,
		a.y * m.c.x + b.y * m.c.y + c.y * m.c.z + d.y * m.c.w,
		a.z * m.c.x + b.z * m.c.y + c.z * m.c.z + d.z * m.c.w,
		a.w * m.c.x + b.w * m.c.y + c.w * m.c.z + d.w * m.c.w,
		a.x * m.d.x + b.x * m.d.y + c.x * m.d.z + d.x * m.d.w,
		a.y * m.d.x + b.y * m.d.y + c.y * m.d.z + d.y * m.d.w,
		a.z * m.d.x + b.z * m.d.y + c.z * m.d.z + d.z * m.d.w,
		a.w * m.d.x + b.w * m.d.y + c.w * m.d.z + d.w * m.d.w
	};
}
inline constexpr Vector2 Matrix4::operator*(const Vector2& v) const
{
	return {
		a.x * v.x + b.x * v.y,
		a.y * v.x + b.y * v.y
	};
}
inline constexpr Vector3 Matrix4::operator*(const Vector3& v) const
{
	return {
		a.x * v.x + b.x * v.y + c.x * v.z,
		a.y * v.x + b.y * v.y + c.y * v.z,
		a.z * v.x + b.z * v.y + c.z * v.z
	};
}
inline constexpr Vector4 Matrix4::operator*(const Vector4& v) const
{
	return {
		a.x * v.x + b.x * v.y + c.x * v.z + c.x * v.w,
		a.y * v.x + b.y * v.y + c.y * v.z + c.y * v.w,
		a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.w,
		a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.w
	};
}

inline constexpr Matrix4 Matrix4::Inverse() const
{
	F32 t0 = c.z * d.w;
	F32 t1 = d.z * c.w;
	F32 t2 = b.z * d.w;
	F32 t3 = d.z * b.w;
	F32 t4 = b.z * c.w;
	F32 t5 = c.z * b.w;
	F32 t6 = a.z * d.w;
	F32 t7 = d.z * a.w;
	F32 t8 = a.z * c.w;
	F32 t9 = c.z * a.w;
	F32 t10 = a.z * b.w;
	F32 t11 = b.z * a.w;
	F32 t12 = c.x * d.y;
	F32 t13 = d.x * c.y;
	F32 t14 = b.x * d.y;
	F32 t15 = d.x * b.y;
	F32 t16 = b.x * c.y;
	F32 t17 = c.x * b.y;
	F32 t18 = a.x * d.y;
	F32 t19 = d.x * a.y;
	F32 t20 = a.x * c.y;
	F32 t21 = c.x * a.y;
	F32 t22 = a.x * b.y;
	F32 t23 = b.x * a.y;

	Matrix4 m;

	m.a.x = (t0 * b.y + t3 * c.y + t4 * d.y) - (t1 * b.y + t2 * c.y + t5 * d.y);
	m.a.y = (t1 * a.y + t6 * c.y + t9 * d.y) - (t0 * a.y + t7 * c.y + t8 * d.y);
	m.a.z = (t2 * a.y + t7 * b.y + t10 * d.y) - (t3 * a.y + t6 * b.y + t11 * d.y);
	m.a.w = (t5 * a.y + t8 * b.y + t11 * c.y) - (t4 * a.y + t9 * b.y + t10 * c.y);

	F32 determinant = (a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + d.x * m.a.w);
	if (Math::IsZero(determinant)) { return Matrix4{ }; }
	F32 f = 1.0f / determinant;

	m.a.x = f * m.a.x;
	m.a.y = f * m.a.y;
	m.a.z = f * m.a.z;
	m.a.w = f * m.a.w;
	m.b.x = f * ((t1 * b.x + t2 * c.x + t5 * d.x) - (t0 * b.x + t3 * c.x + t4 * d.x));
	m.b.y = f * ((t0 * a.x + t7 * c.x + t8 * d.x) - (t1 * a.x + t6 * c.x + t9 * d.x));
	m.b.z = f * ((t3 * a.x + t6 * b.x + t11 * d.x) - (t2 * a.x + t7 * b.x + t10 * d.x));
	m.b.w = f * ((t4 * a.x + t9 * b.x + t10 * c.x) - (t5 * a.x + t8 * b.x + t11 * c.x));
	m.c.x = f * ((t12 * b.w + t15 * c.w + t16 * d.w) - (t13 * b.w + t14 * c.w + t17 * d.w));
	m.c.y = f * ((t13 * a.w + t18 * c.w + t21 * d.w) - (t12 * a.w + t19 * c.w + t20 * d.w));
	m.c.z = f * ((t14 * a.w + t19 * b.w + t22 * d.w) - (t15 * a.w + t18 * b.w + t23 * d.w));
	m.c.w = f * ((t17 * a.w + t20 * b.w + t23 * c.w) - (t16 * a.w + t21 * b.w + t22 * c.w));
	m.d.x = f * ((t14 * c.z + t17 * d.z + t13 * b.z) - (t16 * d.z + t12 * b.z + t15 * c.z));
	m.d.y = f * ((t20 * d.z + t12 * a.z + t19 * c.z) - (t18 * c.z + t21 * d.z + t13 * a.z));
	m.d.z = f * ((t18 * b.z + t23 * d.z + t15 * a.z) - (t22 * d.z + t14 * a.z + t19 * b.z));
	m.d.w = f * ((t22 * c.z + t16 * a.z + t21 * b.z) - (t20 * b.z + t23 * c.z + t17 * a.z));

	return m;
}
inline constexpr Matrix4& Matrix4::Inversed()
{
	F32 t0 = c.z * d.w;
	F32 t1 = d.z * c.w;
	F32 t2 = b.z * d.w;
	F32 t3 = d.z * b.w;
	F32 t4 = b.z * c.w;
	F32 t5 = c.z * b.w;
	F32 t6 = a.z * d.w;
	F32 t7 = d.z * a.w;
	F32 t8 = a.z * c.w;
	F32 t9 = c.z * a.w;
	F32 t10 = a.z * b.w;
	F32 t11 = b.z * a.w;
	F32 t12 = c.x * d.y;
	F32 t13 = d.x * c.y;
	F32 t14 = b.x * d.y;
	F32 t15 = d.x * b.y;
	F32 t16 = b.x * c.y;
	F32 t17 = c.x * b.y;
	F32 t18 = a.x * d.y;
	F32 t19 = d.x * a.y;
	F32 t20 = a.x * c.y;
	F32 t21 = c.x * a.y;
	F32 t22 = a.x * b.y;
	F32 t23 = b.x * a.y;

	F32 ax = (t0 * b.y + t3 * c.y + t4 * d.y) - (t1 * b.y + t2 * c.y + t5 * d.y);
	F32 ay = (t1 * a.y + t6 * c.y + t9 * d.y) - (t0 * a.y + t7 * c.y + t8 * d.y);
	F32 az = (t2 * a.y + t7 * b.y + t10 * d.y) - (t3 * a.y + t6 * b.y + t11 * d.y);
	F32 aw = (t5 * a.y + t8 * b.y + t11 * c.y) - (t4 * a.y + t9 * b.y + t10 * c.y);

	F32 determinant = (a.x * ax + b.x * ay + c.x * az + d.x * aw);
	if (Math::IsZero(determinant)) { return *this = Matrix4{}; }
	F32 f = 1.0f / determinant;

	a.x = f * ax;
	a.y = f * ay;
	a.z = f * az;
	a.w = f * aw;

	F32 bx = f * ((t1 * b.x + t2 * c.x + t5 * d.x) - (t0 * b.x + t3 * c.x + t4 * d.x));
	F32 by = f * ((t0 * a.x + t7 * c.x + t8 * d.x) - (t1 * a.x + t6 * c.x + t9 * d.x));
	F32 bz = f * ((t3 * a.x + t6 * b.x + t11 * d.x) - (t2 * a.x + t7 * b.x + t10 * d.x));
	F32 bw = f * ((t4 * a.x + t9 * b.x + t10 * c.x) - (t5 * a.x + t8 * b.x + t11 * c.x));
	F32 cx = f * ((t12 * b.w + t15 * c.w + t16 * d.w) - (t13 * b.w + t14 * c.w + t17 * d.w));
	F32 cy = f * ((t13 * a.w + t18 * c.w + t21 * d.w) - (t12 * a.w + t19 * c.w + t20 * d.w));
	F32 cz = f * ((t14 * a.w + t19 * b.w + t22 * d.w) - (t15 * a.w + t18 * b.w + t23 * d.w));
	F32 cw = f * ((t17 * a.w + t20 * b.w + t23 * c.w) - (t16 * a.w + t21 * b.w + t22 * c.w));
	F32 dx = f * ((t14 * c.z + t17 * d.z + t13 * b.z) - (t16 * d.z + t12 * b.z + t15 * c.z));
	F32 dy = f * ((t20 * d.z + t12 * a.z + t19 * c.z) - (t18 * c.z + t21 * d.z + t13 * a.z));
	F32 dz = f * ((t18 * b.z + t23 * d.z + t15 * a.z) - (t22 * d.z + t14 * a.z + t19 * b.z));
	F32 dw = f * ((t22 * c.z + t16 * a.z + t21 * b.z) - (t20 * b.z + t23 * c.z + t17 * a.z));

	b.x = bx;
	b.y = by;
	b.z = bz;
	b.w = bw;
	c.x = cx;
	c.y = cy;
	c.z = cz;
	c.w = cw;
	d.x = dx;
	d.y = dy;
	d.z = dz;
	d.w = dw;

	return *this;
}
inline constexpr Matrix4 Matrix4::Invert() const
{
	Matrix4 m;

	F32 t0 = c.z * d.w;
	F32 t1 = d.z * c.w;
	F32 t2 = b.z * d.w;
	F32 t3 = d.z * b.w;
	F32 t4 = b.z * c.w;
	F32 t5 = c.z * b.w;
	F32 t6 = a.z * d.w;
	F32 t7 = d.z * a.w;
	F32 t8 = a.z * c.w;
	F32 t9 = c.z * a.w;
	F32 t10 = a.z * b.w;
	F32 t11 = b.z * a.w;
	F32 t12 = c.x * d.y;
	F32 t13 = d.x * c.y;
	F32 t14 = b.x * d.y;
	F32 t15 = d.x * b.y;
	F32 t16 = b.x * c.y;
	F32 t17 = c.x * b.y;
	F32 t18 = a.x * d.y;
	F32 t19 = d.x * a.y;
	F32 t20 = a.x * c.y;
	F32 t21 = c.x * a.y;
	F32 t22 = a.x * b.y;
	F32 t23 = b.x * a.y;

	F32 ax = (t0 * b.y + t3 * c.y + t4 * d.y) - (t1 * b.y + t2 * c.y + t5 * d.y);
	F32 ay = (t1 * a.y + t6 * c.y + t9 * d.y) - (t0 * a.y + t7 * c.y + t8 * d.y);
	F32 az = (t2 * a.y + t7 * b.y + t10 * d.y) - (t3 * a.y + t6 * b.y + t11 * d.y);
	F32 aw = (t5 * a.y + t8 * b.y + t11 * c.y) - (t4 * a.y + t9 * b.y + t10 * c.y);

	F32 f = 1.0f / (a.x * ax + b.x * ay + c.x * az + d.x * aw);

	return {
		f * ax, f * ay, f * az, f * aw,
		f * ((t1 * b.x + t2 * c.x + t5 * d.x) - (t0 * b.x + t3 * c.x + t4 * d.x)),
		f * ((t0 * a.x + t7 * c.x + t8 * d.x) - (t1 * a.x + t6 * c.x + t9 * d.x)),
		f * ((t3 * a.x + t6 * b.x + t11 * d.x) - (t2 * a.x + t7 * b.x + t10 * d.x)),
		f * ((t4 * a.x + t9 * b.x + t10 * c.x) - (t5 * a.x + t8 * b.x + t11 * c.x)),
		f * ((t12 * b.w + t15 * c.w + t16 * d.w) - (t13 * b.w + t14 * c.w + t17 * d.w)),
		f * ((t13 * a.w + t18 * c.w + t21 * d.w) - (t12 * a.w + t19 * c.w + t20 * d.w)),
		f * ((t14 * a.w + t19 * b.w + t22 * d.w) - (t15 * a.w + t18 * b.w + t23 * d.w)),
		f * ((t17 * a.w + t20 * b.w + t23 * c.w) - (t16 * a.w + t21 * b.w + t22 * c.w)),
		f * ((t14 * c.z + t17 * d.z + t13 * b.z) - (t16 * d.z + t12 * b.z + t15 * c.z)),
		f * ((t20 * d.z + t12 * a.z + t19 * c.z) - (t18 * c.z + t21 * d.z + t13 * a.z)),
		f * ((t18 * b.z + t23 * d.z + t15 * a.z) - (t22 * d.z + t14 * a.z + t19 * b.z)),
		f * ((t22 * c.z + t16 * a.z + t21 * b.z) - (t20 * b.z + t23 * c.z + t17 * a.z))
	};
}
inline constexpr Matrix4& Matrix4::Inverted()
{
	Matrix4 m;

	F32 t0 = c.z * d.w;
	F32 t1 = d.z * c.w;
	F32 t2 = b.z * d.w;
	F32 t3 = d.z * b.w;
	F32 t4 = b.z * c.w;
	F32 t5 = c.z * b.w;
	F32 t6 = a.z * d.w;
	F32 t7 = d.z * a.w;
	F32 t8 = a.z * c.w;
	F32 t9 = c.z * a.w;
	F32 t10 = a.z * b.w;
	F32 t11 = b.z * a.w;
	F32 t12 = c.x * d.y;
	F32 t13 = d.x * c.y;
	F32 t14 = b.x * d.y;
	F32 t15 = d.x * b.y;
	F32 t16 = b.x * c.y;
	F32 t17 = c.x * b.y;
	F32 t18 = a.x * d.y;
	F32 t19 = d.x * a.y;
	F32 t20 = a.x * c.y;
	F32 t21 = c.x * a.y;
	F32 t22 = a.x * b.y;
	F32 t23 = b.x * a.y;

	F32 ax = (t0 * b.y + t3 * c.y + t4 * d.y) - (t1 * b.y + t2 * c.y + t5 * d.y);
	F32 ay = (t1 * a.y + t6 * c.y + t9 * d.y) - (t0 * a.y + t7 * c.y + t8 * d.y);
	F32 az = (t2 * a.y + t7 * b.y + t10 * d.y) - (t3 * a.y + t6 * b.y + t11 * d.y);
	F32 aw = (t5 * a.y + t8 * b.y + t11 * c.y) - (t4 * a.y + t9 * b.y + t10 * c.y);

	F32 f = 1.0f / (a.x * ax + b.x * ay + c.x * az + d.x * aw);

	m.a.x = f * ax;
	m.a.y = f * ay;
	m.a.z = f * az;
	m.a.w = f * aw;
	m.b.x = f * ((t1 * b.x + t2 * c.x + t5 * d.x) - (t0 * b.x + t3 * c.x + t4 * d.x));
	m.b.y = f * ((t0 * a.x + t7 * c.x + t8 * d.x) - (t1 * a.x + t6 * c.x + t9 * d.x));
	m.b.z = f * ((t3 * a.x + t6 * b.x + t11 * d.x) - (t2 * a.x + t7 * b.x + t10 * d.x));
	m.b.w = f * ((t4 * a.x + t9 * b.x + t10 * c.x) - (t5 * a.x + t8 * b.x + t11 * c.x));
	m.c.x = f * ((t12 * b.w + t15 * c.w + t16 * d.w) - (t13 * b.w + t14 * c.w + t17 * d.w));
	m.c.y = f * ((t13 * a.w + t18 * c.w + t21 * d.w) - (t12 * a.w + t19 * c.w + t20 * d.w));
	m.c.z = f * ((t14 * a.w + t19 * b.w + t22 * d.w) - (t15 * a.w + t18 * b.w + t23 * d.w));
	m.c.w = f * ((t17 * a.w + t20 * b.w + t23 * c.w) - (t16 * a.w + t21 * b.w + t22 * c.w));
	m.d.x = f * ((t14 * c.z + t17 * d.z + t13 * b.z) - (t16 * d.z + t12 * b.z + t15 * c.z));
	m.d.y = f * ((t20 * d.z + t12 * a.z + t19 * c.z) - (t18 * c.z + t21 * d.z + t13 * a.z));
	m.d.z = f * ((t18 * b.z + t23 * d.z + t15 * a.z) - (t22 * d.z + t14 * a.z + t19 * b.z));
	m.d.w = f * ((t22 * c.z + t16 * a.z + t21 * b.z) - (t20 * b.z + t23 * c.z + t17 * a.z));

	a = m.a;
	b = m.b;
	c = m.c;
	d = m.d;

	return *this;
}
inline constexpr Matrix4 Matrix4::Transpose() const
{
	return {
		a.x, b.x, c.x, d.x,
		a.y, b.y, c.y, d.y,
		a.z, b.z, c.z, d.z,
		a.w, b.w, c.w, d.w
	};
}
inline constexpr Matrix4& Matrix4::Transposed()
{
	F32 bx = a.y;
	F32 cx = a.z;
	F32 cy = b.z;
	F32 dx = a.w;
	F32 dy = b.w;
	F32 dz = c.w;

	a.y = b.x;
	a.z = c.x;
	a.w = d.x;
	b.z = c.y;
	b.w = d.y;
	c.w = d.z;

	b.x = bx;
	c.x = cx;
	c.y = cy;
	d.x = dx;
	d.y = dy;
	d.z = dz;

	return *this;
}

inline constexpr void Matrix4::Set(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	Quaternion3 q = rotation;

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}
inline constexpr void Matrix4::Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
{
	Quaternion3 q = rotation.Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}
inline constexpr void Matrix4::SetPerspective(F32 fov, F32 aspect, F32 nearPlane, F32 farPlane)
{
	F32 yScale = 1.0f / Math::Tan(fov * (F32)DegToRad * 0.5f);
	F32 xScale = yScale / aspect;
	F32 nearFar = 1.0f / (nearPlane - farPlane);

	a.x = xScale;
	a.y = 0.0f;
	a.z = 0.0f;
	a.w = 0.0f;

	b.x = 0.0f;
	b.y = -yScale;
	b.z = 0.0f;
	b.w = 0.0f;

	c.x = 0.0f;
	c.y = 0.0f;
	c.z = -farPlane * nearFar;
	c.w = 1.0f;

	d.x = 0.0f;
	d.y = 0.0f;
	d.z = farPlane * nearPlane * nearFar;
	d.w = 0.0f;
}
inline constexpr void Matrix4::SetOrthographic(F32 left, F32 right, F32 bottom, F32 top, F32 nearPlane, F32 farPlane)
{
	F32 rightLeft = 1.0f / (right - left);
	F32 bottomTop = 1.0f / (bottom - top);
	F32 farNear = 1.0f / (farPlane - nearPlane);

	a.x = 2.0f * rightLeft;
	a.y = 0.0f;
	a.z = 0.0f;
	a.w = 0.0f;

	b.x = 0.0f;
	b.y = 2.0f * bottomTop;
	b.z = 0.0f;
	b.w = 0.0f;

	c.x = 0.0f;
	c.y = 0.0f;
	c.z = 2.0f * farNear;
	c.w = 0.0f;

	d.x = -(right + left) * rightLeft;
	d.y = -(bottom + top) * bottomTop;
	d.z = -(farPlane + nearPlane) * farNear;
	d.w = 1.0f;
}
inline constexpr void Matrix4::SetPosition(const Vector3& position)
{
	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
}

inline constexpr void Matrix4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	Vector3 zAxis = (center - eye).Normalized();
	Vector3 xAxis = (up.Cross(zAxis)).Normalized();
	Vector3 yAxis = zAxis.Cross(xAxis);

	a.x = xAxis.x;
	a.y = yAxis.x;
	a.z = zAxis.x;
	a.w = 0.0f;

	b.x = xAxis.y;
	b.y = yAxis.y;
	b.z = zAxis.y;
	b.w = 0.0f;

	c.x = xAxis.z;
	c.y = yAxis.z;
	c.z = zAxis.z;
	c.w = 0.0f;

	d.x = -xAxis.Dot(eye);
	d.y = -yAxis.Dot(eye);
	d.z = -zAxis.Dot(eye);
	d.w = 1.0f;
}

inline constexpr Vector3 Matrix4::Forward() { return Vector3(-a.z, -b.z, -c.z).Normalize(); }
inline constexpr Vector3 Matrix4::Back() { return Vector3(a.z, b.z, c.z).Normalize(); }
inline constexpr Vector3 Matrix4::Right() { return Vector3(a.x, b.x, c.x).Normalize(); }
inline constexpr Vector3 Matrix4::Left() { return Vector3(-a.x, -b.x, -c.x).Normalize(); }
inline constexpr Vector3 Matrix4::Up() { return Vector3(a.y, b.y, c.y).Normalize(); }
inline constexpr Vector3 Matrix4::Down() { return Vector3(-a.y, -b.y, -c.y).Normalize(); }

inline constexpr Matrix4 Matrix4::operator-() { return { -a, -b, -c, -d }; }
inline constexpr Matrix4 Matrix4::operator~() { return { ~a, ~b, ~c, ~d }; }
inline constexpr Matrix4 Matrix4::operator!() { return { !a, !b, !c, !d }; }

inline constexpr bool Matrix4::operator==(const Matrix4& m) const { return a == m.a && b == m.b && c == m.c && d == m.d; }
inline constexpr bool Matrix4::operator!=(const Matrix4& m) const { return a != m.a || b != m.b || c != m.c || d != m.d; }

inline Vector4& Matrix4::operator[] (U8 i) { return (&a)[i]; }
inline const Vector4& Matrix4::operator[] (U8 i) const { return (&a)[i]; }

inline F32* Matrix4::Data() { return a.Data(); }
inline const F32* Matrix4::Data() const { return a.Data(); }
#pragma endregion

#pragma region Quaternion2
inline constexpr Quaternion2::Quaternion2() : x(0.0f), y(1.0f) {}
inline constexpr Quaternion2::Quaternion2(F32 x, F32 y) : x(x), y(y) {}
inline constexpr Quaternion2::Quaternion2(F32 angle) { F32 a = angle * (F32)DegToRad; x = Math::Sin(a); y = Math::Cos(a); }
inline constexpr Quaternion2::Quaternion2(const Matrix2& mat) : x(Math::Sqrt((mat.a.x - 1.0f) * -0.5f)), y(Math::Sqrt(mat.a.y * 0.5f)) {}
inline constexpr Quaternion2::Quaternion2(const Quaternion2& q) : x(q.x), y(q.y) {}
inline constexpr Quaternion2::Quaternion2(Quaternion2&& q) noexcept : x(q.x), y(q.y) {}

inline constexpr Quaternion2& Quaternion2::operator=(F32 angle) { F32 a = angle * (F32)DegToRad; x = Math::Sin(a); y = Math::Cos(a); return *this; }
inline constexpr Quaternion2& Quaternion2::operator=(const Quaternion2& q) { x = q.x; y = q.y; return *this; }
inline constexpr Quaternion2& Quaternion2::operator=(Quaternion2&& q) noexcept { x = q.x; y = q.y; return *this; }

inline constexpr Quaternion2& Quaternion2::operator+=(const Quaternion2& q) { x += q.x; y += q.y; return *this; }
inline constexpr Quaternion2& Quaternion2::operator-=(const Quaternion2& q) { x -= q.x; y -= q.y; return *this; }
inline constexpr Quaternion2& Quaternion2::operator*=(const Quaternion2& q) { x = y * q.x + x * q.y; y = y * q.y - x * q.x; return *this; }
inline constexpr Quaternion2& Quaternion2::operator/=(const Quaternion2& q)
{
	F32 n2 = 1.0f / q.SqrNormal();

	x = (-y * q.x + x * q.y) * n2;
	y = (y * q.y + x * q.x) * n2;

	return *this;
}

inline constexpr Quaternion2 Quaternion2::operator*(F32 f) const { return { x * f, y * f }; }
inline constexpr Quaternion2 Quaternion2::operator/(F32 f) const { return { x / f, y / f }; }
inline constexpr Quaternion2 Quaternion2::operator+(const Quaternion2& q) const { return { x + q.x, y + q.y }; }
inline constexpr Quaternion2 Quaternion2::operator-(const Quaternion2& q) const { return { x - q.x, y - q.y }; }
inline constexpr Quaternion2 Quaternion2::operator*(const Quaternion2& q) const { return { x * q.y + y * q.x, y * q.y - x * q.x, }; }
inline constexpr Quaternion2 Quaternion2::operator^(const Quaternion2& q) const { return { y * q.x - x * q.y, y * q.y + x * q.x, }; }
inline constexpr Quaternion2 Quaternion2::operator/(const Quaternion2& q) const
{
	F32 n2 = 1.0f / q.SqrNormal();

	return {
		(x * q.y - y * q.x) * n2,
		(y * q.y + x * q.x) * n2
	};
}

inline constexpr void Quaternion2::Set(F32 angle) { F32 a = angle * (F32)DegToRad; x = Math::Sin(a); y = Math::Cos(a); }
inline constexpr void Quaternion2::Rotate(F32 angle) { F32 a = angle * (F32)DegToRad; x += Math::Sin(a); y += Math::Cos(a); }
inline constexpr F32 Quaternion2::Angle() const { return Math::ASin(x) * (F32)DegToRad; }

inline constexpr Matrix2 Quaternion2::ToMatrix2() const
{
	Quaternion2 q = Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.y * q.y;

	return {
		1.0f - xx, xy,
		-xy, 1.0f - xx
	};
}
inline constexpr Matrix3 Quaternion2::ToMatrix3() const
{
	Quaternion2 q = Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.y * q.y;

	return {
		1.0f - xx, xy, 0.0f,
		-xy, 1.0f - xx, 0.0f,
		0.0f, 0.0f, 1.0f
	};
}
inline constexpr Matrix4 Quaternion2::RotationMatrix(Vector2 center) const
{
	Matrix4 matrix{};

	F32 zz = x * x;
	F32 ww = y * y;

	matrix.a.x = -zz + ww;
	matrix.b.x = 0.0f;
	matrix.c.x = 0.0f;
	matrix.d.x = center.x - center.x * matrix.a.x - center.y * matrix.b.x;

	matrix.a.y = 0.0f;
	matrix.b.y = -zz + ww;
	matrix.c.y = 0.0f;
	matrix.d.y = center.y - center.x * matrix.a.y - center.y * matrix.b.y;

	matrix.a.z = 0.0f;
	matrix.b.z = 0.0f;
	matrix.c.z = zz + ww;
	matrix.d.z = -center.x * matrix.a.z - center.y * matrix.b.z;

	return matrix;
}

inline constexpr Quaternion2 Quaternion2::Slerp(const Quaternion2& q, F32 t) const
{
	constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion2 v0 = Normalize();
	Quaternion2 v1 = q.Normalize();

	F32 dot = v0.Dot(v1);

	if (dot < 0.0f)
	{
		v1.x = -v1.x;
		v1.y = -v1.y;
		dot = -dot;
	}

	if (dot > DOT_THRESHOLD)
	{
		Quaternion2 out{
			v0.x + ((v1.x - v0.x) * t),
			v0.y + ((v1.y - v0.y) * t)
		};

		return out.Normalize();
	}

	F32 theta0 = Math::ACos(dot);
	F32 theta = theta0 * t;
	F32 sinTheta = Math::Sin(theta);
	F32 sinTheta0 = Math::Sin(theta0);

	F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
	F32 s1 = sinTheta / sinTheta0;

	return {
		v0.x * s0 + v1.x * s1,
		v0.y * s0 + v1.y * s1
	};
}
inline constexpr Quaternion2& Quaternion2::Slerped(const Quaternion2& q, F32 t)
{
	constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion2 v0 = Normalize();
	Quaternion2 v1 = q.Normalize();

	F32 dot = v0.Dot(v1);

	if (dot < 0.0f)
	{
		v1.x = -v1.x;
		v1.y = -v1.y;
		dot = -dot;
	}

	if (dot > DOT_THRESHOLD)
	{
		x = v0.x + ((v1.x - v0.x) * t);
		y = v0.y + ((v1.y - v0.y) * t);

		return Normalized();
	}

	F32 theta0 = Math::ACos(dot);
	F32 theta = theta0 * t;
	F32 sinTheta = Math::Sin(theta);
	F32 sinTheta0 = Math::Sin(theta0);

	F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
	F32 s1 = sinTheta / sinTheta0;

	x = v0.x * s0 + v1.x * s1;
	y = v0.y * s0 + v1.y * s1;

	return *this;
}
inline constexpr Quaternion2 Quaternion2::NLerp(const Quaternion2& q, F32 t) const
{
	F32 omt = 1.0f - t;
	Quaternion2 quat = {
		omt * x + t * q.x,
		omt * y + t * q.y
	};

	return quat.Normalize();
}
inline constexpr Quaternion2& Quaternion2::NLerped(const Quaternion2& q, F32 t)
{
	F32 omt = 1.0f - t;
	x = omt * x + t * q.x;
	y = omt * y + t * q.y;

	return Normalized();
}

inline constexpr F32 Quaternion2::RelativeAngle(const Quaternion2& q) const { return 0.0f; }
inline constexpr F32 Quaternion2::Dot(const Quaternion2& q) const { return x * q.x + y * q.y; }
inline constexpr F32 Quaternion2::SqrNormal() const { return x * x + y * y; }
inline constexpr F32 Quaternion2::Normal() const { return Math::Sqrt(x * x + y * y); }
inline constexpr Quaternion2 Quaternion2::Normalize() const { F32 n = 1.0f / Normal(); return { x * n, y * n }; }
inline constexpr Quaternion2& Quaternion2::Normalized() { F32 n = 1.0f / Normal(); x *= n; y *= n; return *this; }
inline constexpr Quaternion2 Quaternion2::Conjugate() const { return { -x, y }; }
inline constexpr Quaternion2& Quaternion2::Conjugated() { x = -x; return *this; }
inline constexpr Quaternion2 Quaternion2::Inverse() const { F32 n = 1.0f / Math::Sqrt(x * x + y * y); return { -x * n, y * n }; }
inline constexpr Quaternion2& Quaternion2::Inversed() { return Conjugated().Normalized(); }
inline constexpr Quaternion2 Quaternion2::Integrate(F32 deltaAngle)
{
	Quaternion2 q2 = { x + deltaAngle * y, y - deltaAngle * x };
	F32 mag = q2.Normal();
	F32 invMag = mag > 0.0 ? 1.0f / mag : 0.0f;
	return { q2.x * invMag, q2.y * invMag };
}
inline constexpr Quaternion2& Quaternion2::Integrated(F32 deltaAngle)
{
	Quaternion2 q2 = { x + deltaAngle * y, y - deltaAngle * x };
	F32 mag = q2.Normal();
	F32 invMag = mag > 0.0 ? 1.0f / mag : 0.0f;
	Quaternion2 qn = { q2.x * invMag, q2.y * invMag };
	x = q2.x * invMag;
	y = q2.y * invMag;

	return *this;
}

inline F32& Quaternion2::operator[] (U8 i) { return (&x)[i]; }
inline const F32& Quaternion2::operator[] (U8 i) const { return (&x)[i]; }

inline F32* Quaternion2::Data() { return &x; }
inline const F32* Quaternion2::Data() const { return &x; }

inline constexpr Quaternion2::operator Quaternion3() const { return Quaternion3{ 0.0f, 0.0f, x, y }; }
#pragma endregion

#pragma region Quaternion3
inline constexpr Quaternion3::Quaternion3() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
inline constexpr Quaternion3::Quaternion3(F32 x, F32 y, F32 z, F32 w) : x(x), y(y), z(z), w(w) {}
inline constexpr Quaternion3::Quaternion3(const Vector3& euler)
{
	F32 hx = euler.x * (F32)DegToRad * 0.5f;
	F32 hy = euler.y * (F32)DegToRad * 0.5f;
	F32 hz = euler.z * (F32)DegToRad * 0.5f;

	F32 cx = Math::Cos(hx);
	F32 sx = Math::Sin(hx);
	F32 cy = Math::Cos(hy);
	F32 sy = Math::Sin(hy);
	F32 cz = Math::Cos(hz);
	F32 sz = Math::Sin(hz);

	F32 cycx = cx * cy;
	F32 sxcy = sx * cy;
	F32 cxsy = cx * sy;
	F32 sxsy = sx * sy;

	w = cycx * cz + sxsy * sz;
	x = sxcy * cz - cxsy * sz;
	y = cxsy * cz + sxcy * sz;
	z = cycx * sz - sxsy * cz;
}
inline constexpr Quaternion3::Quaternion3(const Vector3& axis, F32 angle)
{
	const F32 halfAngle = angle * (F32)DegToRad * 0.5f;
	F32 s = Math::Sin(halfAngle);
	F32 c = Math::Cos(halfAngle);

	Vector3 a = axis.Normalized();

	x = s * a.x;
	y = s * a.y;
	z = s * a.z;
	w = c;
}
inline constexpr Quaternion3::Quaternion3(const Quaternion3& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
inline constexpr Quaternion3::Quaternion3(const Quaternion2& q) : x(0.0f), y(0.0f), z(q.x), w(q.y) {}
inline constexpr Quaternion3::Quaternion3(Quaternion3&& q) noexcept : x(q.x), y(q.y), z(q.z), w(q.w) {}

inline constexpr Quaternion3& Quaternion3::operator=(const Vector3& euler)
{
	F32 hx = euler.x * (F32)DegToRad * 0.5f;
	F32 hy = euler.y * (F32)DegToRad * 0.5f;
	F32 hz = euler.z * (F32)DegToRad * 0.5f;

	F32 cx = Math::Cos(hx);
	F32 sx = Math::Sin(hx);
	F32 cy = Math::Cos(hy);
	F32 sy = Math::Sin(hy);
	F32 cz = Math::Cos(hz);
	F32 sz = Math::Sin(hz);

	F32 cycx = cx * cy;
	F32 sxcy = sx * cy;
	F32 cxsy = cx * sy;
	F32 sxsy = sx * sy;

	w = cycx * cz + sxsy * sz;
	x = sxcy * cz - cxsy * sz;
	y = cxsy * cz + sxcy * sz;
	z = cycx * sz - sxsy * cz;

	return *this;
}

inline constexpr Quaternion3& Quaternion3::operator=(const Quaternion3& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
inline constexpr Quaternion3& Quaternion3::operator=(Quaternion3&& q) noexcept { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }

inline constexpr Quaternion3& Quaternion3::operator+=(const Quaternion3& q)
{
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;

	return *this;
}
inline constexpr Quaternion3& Quaternion3::operator-=(const Quaternion3& q)
{
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;

	return *this;
}
inline constexpr Quaternion3& Quaternion3::operator*=(const Quaternion3& q)
{
	F32 tx = w * q.x + x * q.w + y * q.z - z * q.y;
	F32 ty = w * q.y - x * q.z + y * q.w + z * q.x;
	F32 tz = w * q.z + x * q.y - y * q.x + z * q.w;
	F32 tw = w * q.w - x * q.x - y * q.y - z * q.z;

	x = tx;
	y = ty;
	z = tz;
	w = tw;

	return *this;
}
inline constexpr Quaternion3& Quaternion3::operator/=(const Quaternion3& q)
{
	F32 n2 = 1.0f / q.SqrNormal();

	F32 tx = -w * q.x + x * q.w - y * q.z + z * q.y;
	F32 ty = -w * q.y + x * q.z + y * q.w - z * q.x;
	F32 tz = -w * q.z - x * q.y + y * q.x + z * q.w;
	F32 tw = w * q.w + x * q.x + y * q.y + z * q.z;

	x = tx * n2;
	y = ty * n2;
	x = tz * n2;
	w = tw * n2;

	return *this;
}

inline constexpr Quaternion3 Quaternion3::operator+(const Quaternion3& q) const
{
	return {
		x + q.x,
		y + q.y,
		z + q.z,
		w + q.w
	};
}
inline constexpr Quaternion3 Quaternion3::operator-(const Quaternion3& q) const
{
	return {
		x - q.x,
		y - q.y,
		z - q.z,
		w - q.w
	};
}
inline constexpr Quaternion3 Quaternion3::operator*(const Quaternion3& q) const
{
	F32 tx = w * q.x + x * q.w + y * q.z - z * q.y;
	F32 ty = w * q.y - x * q.z + y * q.w + z * q.x;
	F32 tz = w * q.z + x * q.y - y * q.x + z * q.w;
	F32 tw = w * q.w - x * q.x - y * q.y - z * q.z;

	return {
		tx,
		ty,
		tz,
		tw
	};
}
inline constexpr Quaternion3 Quaternion3::operator/(const Quaternion3& q) const
{
	F32 n2 = 1.0f / q.SqrNormal();

	F32 tx = -w * q.x + x * q.w - y * q.z + z * q.y;
	F32 ty = -w * q.y + x * q.z + y * q.w - z * q.x;
	F32 tz = -w * q.z - x * q.y + y * q.x + z * q.w;
	F32 tw = w * q.w + x * q.x + y * q.y + z * q.z;

	return {
		tx * n2,
		ty * n2,
		tz * n2,
		tw * n2
	};
}

inline constexpr Matrix3 Quaternion3::ToMatrix3() const
{
	Quaternion3 q = Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	return {
		1.0f - yy - zz, xy + zw, xz - yw,
		xy - zw, 1.0f - xx - zz, yz + xw,
		xz + yw, yz - xw, 1.0f - xx - yy
	};
}
inline constexpr Matrix4 Quaternion3::ToMatrix4() const
{
	Quaternion3 q = Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	return {
		1.0f - yy - zz, xy + zw, xz - yw, 0.0f,
		xy - zw, 1.0f - xx - zz, yz + xw, 0.0f,
		xz + yw, yz - xw, 1.0f - xx - yy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

inline constexpr Matrix4 Quaternion3::RotationMatrix(Vector3 center) const
{
	Matrix4 matrix{};

	F32 xx = x * x;
	F32 xy = x * y;
	F32 xz = x * z;
	F32 xw = x * w;
	F32 yy = y * y;
	F32 yz = y * z;
	F32 yw = y * w;
	F32 zz = z * z;
	F32 ww = w * w;

	matrix.a.x = xx - yy - zz + ww;
	matrix.b.x = 2.0f * (xy + xw);
	matrix.c.x = 2.0f * (xz - yw);
	matrix.d.x = center.x - center.x * matrix.a.x - center.y * matrix.b.x - center.z * matrix.c.x;

	matrix.a.y = 2.0f * (xy - xw);
	matrix.b.y = -xx + yy - zz + ww;
	matrix.c.y = 2.0f * (yz + xw);
	matrix.d.y = center.y - center.x * matrix.a.y - center.y * matrix.b.y - center.z * matrix.c.y;

	matrix.a.z = 2.0f * (xz + yw);
	matrix.b.z = 2.0f * (yz - xw);
	matrix.c.z = -xx - yy + zz + ww;
	matrix.d.z = center.z - center.x * matrix.a.z - center.y * matrix.b.z - center.z * matrix.c.z;

	return matrix;
}

inline constexpr Vector3 Quaternion3::Euler() const
{
	F32 v = x * y + z * w;

	if (Math::Abs(v - 0.5f) < Traits<F32>::Epsilon) { return { 2.0f * Math::ATan2(x, w) * (F32)RadToDeg, HalfPi * RadToDeg, 0.0f }; }
	if (Math::Abs(v + 0.5f) < Traits<F32>::Epsilon) { return { -2.0f * Math::ATan2(x, w) * (F32)RadToDeg, -HalfPi * RadToDeg, 0.0f }; }

	return {
		Math::ASin(2.0f * v) * (F32)RadToDeg,
		Math::ATan2(2.0f * (w * y - x * z), 1.0f - 2.0f * (y * y + z * z)) * (F32)RadToDeg,
		Math::ATan2(2.0f * (w * x - y * z), 1.0f - 2.0f * (x * x + z * z)) * (F32)RadToDeg
	};
}

inline constexpr Quaternion3 Quaternion3::Slerp(const Quaternion3& q, F32 t) const
{
	constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion3 v0 = Normalize();
	Quaternion3 v1 = q.Normalize();

	F32 dot = v0.Dot(v1);

	if (dot < 0.0f)
	{
		v1.x = -v1.x;
		v1.y = -v1.y;
		v1.z = -v1.z;
		v1.w = -v1.w;
		dot = -dot;
	}

	if (dot > DOT_THRESHOLD)
	{
		Quaternion3 out{
			v0.x + ((v1.x - v0.x) * t),
			v0.y + ((v1.y - v0.y) * t),
			v0.z + ((v1.z - v0.z) * t),
			v0.w + ((v1.w - v0.w) * t)
		};

		return out.Normalize();
	}

	F32 theta0 = Math::ACos(dot);
	F32 theta = theta0 * t;
	F32 sinTheta = Math::Sin(theta);
	F32 sinTheta0 = Math::Sin(theta0);

	F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
	F32 s1 = sinTheta / sinTheta0;

	return {
		v0.x * s0 + v1.x * s1,
		v0.y * s0 + v1.y * s1,
		v0.z * s0 + v1.z * s1,
		v0.w * s0 + v1.w * s1
	};
}
inline constexpr Quaternion3& Quaternion3::Slerped(const Quaternion3& q, F32 t)
{
	constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion3 v0 = Normalize();
	Quaternion3 v1 = q.Normalize();

	F32 dot = v0.Dot(v1);

	if (dot < 0.0f)
	{
		v1.x = -v1.x;
		v1.y = -v1.y;
		v1.z = -v1.z;
		v1.w = -v1.w;
		dot = -dot;
	}

	if (dot > DOT_THRESHOLD)
	{
		x = v0.x + ((v1.x - v0.x) * t);
		y = v0.y + ((v1.y - v0.y) * t);
		z = v0.z + ((v1.z - v0.z) * t);
		w = v0.w + ((v1.w - v0.w) * t);

		return Normalized();
	}

	F32 theta0 = Math::ACos(dot);
	F32 theta = theta0 * t;
	F32 sinTheta = Math::Sin(theta);
	F32 sinTheta0 = Math::Sin(theta0);

	F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
	F32 s1 = sinTheta / sinTheta0;

	x = v0.x * s0 + v1.x * s1;
	y = v0.y * s0 + v1.y * s1;
	z = v0.z * s0 + v1.z * s1;
	w = v0.w * s0 + v1.w * s1;

	return *this;
}

inline constexpr F32 Quaternion3::Dot(const Quaternion3& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
inline constexpr F32 Quaternion3::Dot(const Vector3& q) const { return x * q.x + y * q.y + z * q.z; }
inline constexpr F32 Quaternion3::SqrNormal() const { return x * x + y * y + z * z + w * w; }
inline constexpr F32 Quaternion3::Normal() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
inline constexpr Quaternion3 Quaternion3::Cross(const Quaternion3& q) const { Vector3 v = Vector3{ x, y, z }.Cross({ q.x, q.y, q.z }); return { v.x, v.y, v.z, -w * q.w }; }
inline constexpr Quaternion3 Quaternion3::Normalize() const { F32 n = 1.0f / Normal(); return { x * n, y * n, z * n, w * n }; }
inline constexpr Quaternion3& Quaternion3::Normalized() { F32 n = 1.0f / Normal(); x *= n; y *= n; z *= n; w *= n; return *this; }
inline constexpr Quaternion3 Quaternion3::Conjugate() const { return { -x, -y, -z, w }; }
inline constexpr Quaternion3& Quaternion3::Conjugated() { x = -x; y = -y; z = -z; return *this; }
inline constexpr Quaternion3 Quaternion3::Inverse() const
{
	F32 n = 1.0f / Math::Sqrt(x * x + y * y + z * z + w * w);
	return { -x * n, -y * n, -z * n, w * n };
}
inline constexpr Quaternion3& Quaternion3::Inversed() { return Conjugated().Normalized(); }

inline F32& Quaternion3::operator[] (U8 i) { return (&x)[i]; }
inline const F32& Quaternion3::operator[] (U8 i) const { return (&x)[i]; }

inline F32* Quaternion3::Data() { return &x; }
inline const F32* Quaternion3::Data() const { return &x; }

inline constexpr Quaternion3::operator Quaternion2() const { return { z, w }; }
inline constexpr Quaternion3::operator Vector3() const { return { x, y, z }; }
#pragma endregion