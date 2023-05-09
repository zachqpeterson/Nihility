#pragma once

#include "MathDefines.hpp"

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

class Math
{
public:
	//TRIGONOMETRY
	template <FloatingPoint Type> static Type Sin(Type f);
	template <FloatingPoint Type> static Type Cos(Type f);
	template <FloatingPoint Type> static Type Tan(Type f);

	template <FloatingPoint Type> static Type Asin(Type f);
	template <FloatingPoint Type> static Type Acos(Type f);
	template <FloatingPoint Type> static Type Atan(Type f);

	template <FloatingPoint Type> static Type Log(Type f);
	template <FloatingPoint Type> static Type Log10(Type f);
	template <FloatingPoint Type> static Type LogN(Type f);

	template <FloatingPoint Type> static Type DegToRad(Type ded) { return (Type)(deg * DEG_TO_RAD); }
	template <FloatingPoint Type> static Type RadToDeg(Type rad) { return (Type)(rad * RAD_TO_DEG); }

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

	template <FloatingPoint Type> static Type DegToRad(const Type& deg) { return (Type)(deg * DEG2RAD); }
	template <FloatingPoint Type> static Type RadToDeg(const Type& rad) { return (Type)(rad * RAD2DEG); }

	static bool Zero(F32 f) { return f < FLOAT_EPSILON && f > -FLOAT_EPSILON; }
	static bool Zero(F64 f) { return f < DOUBLE_EPSILON && f > -DOUBLE_EPSILON; }
	static bool NaN(F32 f);
	static bool NaN(F64 f);
	static bool Inf(F32 f);
	static bool Inf(F64 f);

	//INTERPOLATION
	template <FloatingPoint Type> static Type Lerp(Type a, Type b, Type t) { return a + (b - a) * t; }
	static Vector2 Lerp(const Vector2& a, const Vector2& b, F32 t);
	static Vector3 Lerp(const Vector3& a, const Vector3& b, F32 t);
	static Vector4 Lerp(const Vector4& a, const Vector4& b, F32 t);
	template <FloatingPoint Type> static Type InvLerp(Type a, Type b, Type t) { return (t - a) / (b - a); }
	template <FloatingPoint Type> static Type MoveTowards(Type a, Type b, Type t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }

private:

	STATIC_CLASS(Math);
};

struct Vector2
{
public:
	Vector2() : x{ 0.0f }, y{ 0.0f } {}
	Vector2(F32 f) : x{ f }, y{ f } {}
	Vector2(F32 x, F32 y) : x{ x }, y{ y } {}
	Vector2(const Vector2& v) : x{ v.x }, y{ v.y } {}
	Vector2(Vector2&& v) noexcept : x{ v.x }, y{ v.y } {}

	Vector2& operator=(F32 f) { x = f; y = f; return *this; }
	Vector2& operator=(const Vector2& v) { x = v.x; y = v.y; return *this; }
	Vector2& operator=(Vector2&& v) noexcept { x = v.x; y = v.y; return *this; }

	Vector2& operator+=(F32 f) { x += f; y += f; return *this; }
	Vector2& operator-=(F32 f) { x -= f; y -= f; return *this; }
	Vector2& operator*=(F32 f) { x *= f; y *= f; return *this; }
	Vector2& operator/=(F32 f) { x /= f; y /= f; return *this; }
	Vector2& operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); return *this; }
	Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
	Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
	Vector2& operator*=(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
	Vector2& operator/=(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
	Vector2& operator%=(const Vector2& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); return *this; }

	Vector2 operator+(F32 f) { return { x + f, y + f }; }
	Vector2 operator-(F32 f) { return { x - f, y - f }; }
	Vector2 operator*(F32 f) { return { x * f, y * f }; }
	Vector2 operator/(F32 f) { return { x / f, y / f }; }
	Vector2 operator%(F32 f) { return { Math::Mod(x, f), Math::Mod(y, f) }; }
	Vector2 operator+(const Vector2& v) { return { x + v.x, y + v.y }; }
	Vector2 operator-(const Vector2& v) { return { x - v.x, y - v.y }; }
	Vector2 operator*(const Vector2& v) { return { x * v.x, y * v.y }; }
	Vector2 operator/(const Vector2& v) { return { x / v.x, y / v.y }; }
	Vector2 operator%(const Vector2& v) { return { Math::Mod(x, v.x), Math::Mod(y, v.y) }; }

	bool operator==(const Vector2& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y); }
	bool operator!=(const Vector2& v) const { return !Math::Zero(x - v.x) || !Math::Zero(y - v.y); }

public:
	F32 x, y;

	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 Left;
	static const Vector2 Right;
	static const Vector2 Up;
	static const Vector2 Down;
};