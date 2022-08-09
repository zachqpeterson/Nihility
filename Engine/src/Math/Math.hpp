#pragma once

#include "Defines.hpp"

#define PI 3.14159265358979323846
#define TWO_PI 2.0 * PI
#define HALF_PI 0.5 * PI
#define QUARTER_PI 0.25 * PI
#define ONE_OVER_PI 1.0 / PI
#define ONE_OVER_TWO_PI 1.0 / TWO_PI
#define SQRT_TWO 1.41421356237309504880
#define SQRT_TWO_H 1.5811388300841898
#define SQRT_THREE 1.73205080756887729352
#define SQRT_ONE_OVER_TWO 0.70710678118654752440
#define SQRT_ONE_OVER_THREE 0.57735026918962576450
#define DEG2RAD_MULTIPLIER PI / 180.0
#define RAD2DEG_MULTIPLIER 180.0 / PI
#define SEC_TO_MS_MULTIPLIER 1000.0
#define MS_TO_SEC_MULTIPLIER 0.001
#define FLOAT_EPSILON 1.192092896e-07F
#define DOUBLE_EPSILON 2.22045e-16
#define LONG_DOUBLE_EPSILON 1.0842e-19

struct Vector2;
struct Vector3;
struct Vector4;
struct Vector2Int;
struct Vector3Int;
struct Vector4Int;
struct Matrix2;
struct Matrix3;
struct Matrix4;
struct Quaternion3D;
struct Quaternion2D;

class NH_API Math
{
public:
	//TRIGONOMETRY
	static F32 Sin(F32 f);
	static F64 Sin(F64 f);
	static F32 Cos(F32 f);
	static F64 Cos(F64 f);
	static F32 Tan(F32 f);
	static F64 Tan(F64 f);

	static F32 Asin(F32 f);
	static F64 Asin(F64 f);
	static F32 Acos(F32 f);
	static F64 Acos(F64 f);
	static F32 Atan(F32 f);
	static F64 Atan(F64 f);

	static F32 Log2(F32 f);
	static F64 Log2(F64 f);
	static F32 Log10(F32 f);
	static F64 Log10(F64 f);
	static F32 LogN(F32 f);
	static F64 LogN(F64 f);

	static F32 DegToRad(F32 d) { return (F32)(d * DEG2RAD_MULTIPLIER); }
	static F64 DegToRad(F64 d) { return d * DEG2RAD_MULTIPLIER; }
	static F32 RadToDeg(F32 r) { return (F32)(r * RAD2DEG_MULTIPLIER); }
	static F64 RadToDeg(F64 r) { return r * RAD2DEG_MULTIPLIER; }

	//COMPARISON
	static F32 Abs(F32 n) { return n < 0.f ? -n : n; }
	static F64 Abs(F64 n) { return n < 0. ? -n : n; }
	static I8  Abs(I8  n) { return n < 0 ? -n : n; }
	static I16 Abs(I16 n) { return n < 0 ? -n : n; }
	static I32 Abs(I32 n) { return n < 0 ? -n : n; }
	static I64 Abs(I64 n) { return n < 0 ? -n : n; }

	static F32 Min(F32 a, F32 b) { return a < b ? a : b; }
	static F64 Min(F64 a, F64 b) { return a < b ? a : b; }
	static I8  Min(I8  a, I8  b) { return a < b ? a : b; }
	static I16 Min(I16 a, I16 b) { return a < b ? a : b; }
	static I32 Min(I32 a, I32 b) { return a < b ? a : b; }
	static I64 Min(I64 a, I64 b) { return a < b ? a : b; }
	static U8  Min(U8  a, U8  b) { return a < b ? a : b; }
	static U16 Min(U16 a, U16 b) { return a < b ? a : b; }
	static U32 Min(U32 a, U32 b) { return a < b ? a : b; }
	static U64 Min(U64 a, U64 b) { return a < b ? a : b; }
	static Vector2 Min(const Vector2& a, const Vector2& b);
	static Vector3 Min(const Vector3& a, const Vector3& b);
	static Vector4 Min(const Vector4& a, const Vector4& b);
	static Vector2Int Min(const Vector2Int& a, const Vector2Int& b);
	static Vector3Int Min(const Vector3Int& a, const Vector3Int& b);
	static Vector4Int Min(const Vector4Int& a, const Vector4Int& b);

	static F32 Max(F32 a, F32 b) { return a > b ? a : b; }
	static F64 Max(F64 a, F64 b) { return a > b ? a : b; }
	static I8  Max(I8  a, I8  b) { return a > b ? a : b; }
	static I16 Max(I16 a, I16 b) { return a > b ? a : b; }
	static I32 Max(I32 a, I32 b) { return a > b ? a : b; }
	static I64 Max(I64 a, I64 b) { return a > b ? a : b; }
	static U8  Max(U8  a, U8  b) { return a > b ? a : b; }
	static U16 Max(U16 a, U16 b) { return a > b ? a : b; }
	static U32 Max(U32 a, U32 b) { return a > b ? a : b; }
	static U64 Max(U64 a, U64 b) { return a > b ? a : b; }
	static Vector2 Max(const Vector2& a, const Vector2& b);
	static Vector3 Max(const Vector3& a, const Vector3& b);
	static Vector4 Max(const Vector4& a, const Vector4& b);
	static Vector2Int Max(const Vector2Int& a, const Vector2Int& b);
	static Vector3Int Max(const Vector3Int& a, const Vector3Int& b);
	static Vector4Int Max(const Vector4Int& a, const Vector4Int& b);

	static F32 Clamp(F32 n, F32 a, F32 b) { return n < a ? a : n > b ? b : n; }
	static F64 Clamp(F64 n, F64 a, F64 b) { return n < a ? a : n > b ? b : n; }
	static I8  Clamp(I8  n, I8  a, I8  b) { return n < a ? a : n > b ? b : n; }
	static I16 Clamp(I16 n, I16 a, I16 b) { return n < a ? a : n > b ? b : n; }
	static I32 Clamp(I32 n, I32 a, I32 b) { return n < a ? a : n > b ? b : n; }
	static I64 Clamp(I64 n, I64 a, I64 b) { return n < a ? a : n > b ? b : n; }
	static U8  Clamp(U8  n, U8  a, U8  b) { return n < a ? a : n > b ? b : n; }
	static U16 Clamp(U16 n, U16 a, U16 b) { return n < a ? a : n > b ? b : n; }
	static U32 Clamp(U32 n, U32 a, U32 b) { return n < a ? a : n > b ? b : n; }
	static U64 Clamp(U64 n, U64 a, U64 b) { return n < a ? a : n > b ? b : n; }

	//NEGATIVE BITSHIFTING ROUNDS TOWARDS -INFINITY
	static F32 Closest(F32 n, F32 a, F32 b) { return n < (b + a) * 0.5f ? a : b; }
	static F64 Closest(F64 n, F64 a, F64 b) { return n < (b + a) * 0.5 ? a : b; }
	static I8  Closest(I8  n, I8  a, I8  b) { return n < (b + a) >> 1 ? a : b; }
	static I16 Closest(I16 n, I16 a, I16 b) { return n < (b + a) >> 1 ? a : b; }
	static I32 Closest(I32 n, I32 a, I32 b) { return n < (b + a) >> 1 ? a : b; }
	static I64 Closest(I64 n, I64 a, I64 b) { return n < (b + a) >> 1 ? a : b; }
	static U8  Closest(U8  n, U8  a, U8  b) { return n < (b + a) >> 1 ? a : b; }
	static U16 Closest(U16 n, U16 a, U16 b) { return n < (b + a) >> 1 ? a : b; }
	static U32 Closest(U32 n, U32 a, U32 b) { return n < (b + a) >> 1 ? a : b; }
	static U64 Closest(U64 n, U64 a, U64 b) { return n < (b + a) >> 1 ? a : b; }

	static I32 Floor(F32 n) { return n >= 0 ? (I32)n : (I32)n - 1; }
	static I64 Floor(F64 n) { return n >= 0 ? (I64)n : (I64)n - 1; }
	static F32 FloorF(F32 n) { return (F32)(n >= 0 ? (I32)n : (I32)n - 1); }
	static F64 FloorF(F64 n) { return (F64)(n >= 0 ? (I64)n : (I64)n - 1); }

	static I32 Ceiling(F32 n) { return n > 0 ? (I32)n + 1 : (I32)n; }
	static I64 Ceiling(F64 n) { return n > 0 ? (I64)n + 1 : (I64)n; }
	static F32 CeilingF(F32 n) { return (F32)(n > 0 ? (I32)n + 1 : (I32)n); }
	static F64 CeilingF(F64 n) { return (F64)(n > 0 ? (I64)n + 1 : (I64)n); }

	static bool Zero(F32 f) { return f < FLOAT_EPSILON && f > -FLOAT_EPSILON; }
	static bool Zero(F64 f) { return f < FLOAT_EPSILON && f > -FLOAT_EPSILON; }

	//FLOATING-POINT
	static F32 Round(F32 f) { return (F32)(I32)(f + 0.5f); }
	static F64 Round(F64 f) { return (F64)(I64)(f + 0.5); }
	static F32 Mod(F32 f, F32 d) { return f - d * Floor(f / d); }
	static F64 Mod(F64 f, F64 d) { return f - d * Floor(f / d); }

	//LINEAR
	static F32 Sqrt(F32 f);
	static F64 Sqrt(F64 f);
	static F32 InvSqrt(F32 f);
	static F64 InvSqrt(F64 f);
	static Vector2 Rotate(const Vector2& vector, const Quaternion2D& quat);

	//INTERPOLATION
	static F32 Lerp(F32 a, F32 b, F32 t) { return a + t * (b - a); }
	static F64 Lerp(F64 a, F64 b, F64 t) { return a + t * (b - a); }
	static Vector2 Lerp(const Vector2& a, const Vector2& b, F32 t);
	static Vector3 Lerp(const Vector3& a, const Vector3& b, F32 t);
	static Vector4 Lerp(const Vector4& a, const Vector4& b, F32 t);
	static F32 InvLerp(F32 a, F32 b, F32 t) { return (t - a) / (b - a); }
	static F64 InvLerp(F64 a, F64 b, F64 t) { return (t - a) / (b - a); }
	static F32 MoveTowards(F32 a, F32 b, F32 t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }
	static F64 MoveTowards(F64 a, F64 b, F64 t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }

	//NOISE
	static F64 Simplex1(F64 x);
	static F64 Simplex2(F64 x, F64 y);
	static F64 Simplex3(F64 x, F64 y, F64 z);
private:
	static F64 Dot(I8 g[2], F64 x, F64 y);
	static F64 Dot(I8 g[3], F64 x, F64 y, F64 z);
	static F64 Grad(I64 hash, F64 x);
public:

	//RANDOM
	static I64 Random(U32 seed = 0);
	static I64 RandomRange(I64 min, I64 max, U32 seed = 0);
	static F32 RandomF(U32 seed = 0);
	static F32 RandomRangeF(F32 min, F32 max, U32 seed = 0);

	//OTHER
	static bool PowerOfTwo(U8  n) { return (n != 0) && ((n & (n - 1)) == 0); }
	static bool PowerOfTwo(U16 n) { return (n != 0) && ((n & (n - 1)) == 0); }
	static bool PowerOfTwo(U32 n) { return (n != 0) && ((n & (n - 1)) == 0); }
	static bool PowerOfTwo(U64 n) { return (n != 0) && ((n & (n - 1)) == 0); }

	static U32 Length(I32 i) { return ((bool)i * (U32)Log10((F32)Abs(i)) + 1); }
	static U32 Length(U32 i) { return ((bool)i * (U32)Log10((F32)i) + 1); }
	static U32 Length(I64 i) { return ((bool)i * (U32)Log10((F32)Abs(i)) + 1); }
	static U32 Length(U64 i) { return ((bool)i * (U32)Log10((F32)i) + 1); }
	static U32 Length(F32 f) { return ((bool)(I64)f * (U32)Log10(Abs(f)) + 1); }
	static U32 Length(F64 f) { return ((bool)(I64)f * (U32)Log10(Abs(f)) + 1); }

	//HASHING
	static U64 Hash(const struct String& str, U64 max);
	static U64 Hash(U64 value, U64 max);
private:

	Math() = delete;
};

struct NH_API Vector2
{
	F32 x, y;

	Vector2() : x{ 0.0f }, y{ 0.0f } {}
	Vector2(F32 f) : x{ f }, y{ f } {}
	Vector2(F32 x, F32 y) : x{ x }, y{ y } {}
	Vector2(const Vector2& v) : x{ v.x }, y{ v.y } {}
	Vector2(Vector2&& v) noexcept : x{ v.x }, y{ v.y } {}
	Vector2(const struct String& str);

	Vector2& operator=(const Vector2& v) { x = v.x; y = v.y; return *this; }
	Vector2& operator=(Vector2&& v) noexcept { x = v.x; y = v.y; return *this; }
	Vector2& operator=(const struct String& str);

	Vector2& operator+= (const Vector2& v) { x += v.x; y += v.y;	        return *this; }
	Vector2& operator-= (const Vector2& v) { x -= v.x; y -= v.y;	        return *this; }
	Vector2& operator*= (F32 f) { x *= f; y *= f;					        return *this; }
	Vector2& operator/= (F32 f) { x /= f; y /= f;					        return *this; }
	Vector2& operator%= (F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); return *this; }

	Vector2 operator+ (const Vector2& v) const { return Vector2{ x + v.x, y + v.y }; }
	Vector2 operator- (const Vector2& v) const { return Vector2{ x - v.x, y - v.y }; }
	Vector2 operator* (F32 f) const { return Vector2{ x * f, y * f }; }
	Vector2 operator/ (F32 f) const { return Vector2{ x / f, y / f }; }
	Vector2 operator+ (F32 f) const { return Vector2{ x + f, y + f }; }
	Vector2 operator- (F32 f) const { return Vector2{ x - f, y - f }; }
	Vector2 operator% (F32 f) const { return Vector2{ Math::Mod(x, f), Math::Mod(y, f) }; }

	bool operator== (const Vector2& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y); }
	bool operator!= (const Vector2& v) const { return Math::Zero(x - v.x) || Math::Zero(y - v.y); }
	bool operator<  (const Vector2& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	bool operator>  (const Vector2& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	friend Vector2 operator- (const Vector2& v);

	NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

	NH_INLINE F32 Dot(const Vector2& v) const { return v.x * x + v.y * y; }
	NH_INLINE F32 Magnitude() const { return Math::Sqrt(Dot(*this)); }
	NH_INLINE F32 SqrMagnitude() const { return Dot(*this); }
	NH_INLINE Vector2& Normalize() { Vector2 v = Normalized(); x = v.x; y = v.y; return *this; }
	NH_INLINE Vector2 Normalized() const { return Math::Zero(x) && Math::Zero(y) ? Vector2::ZERO : (*this) / Magnitude(); }
	NH_INLINE F32 AngleBetween(const Vector2& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
	NH_INLINE Vector2 Projection(const Vector2& v) const { return v * (Dot(v) / v.Dot(v)); }
	NH_INLINE Vector2 OrthoProjection(const Vector2& v) const { return *this - Projection(v); }
	NH_INLINE void Rotate(const Vector2& center, F32 angle)
	{
		F32 cos = Math::Cos(angle);
		F32 sin = Math::Sin(angle);
		F32 temp = cos * (x - center.x) - sin * (y - center.y) + center.x;
		y = sin * (x - center.x) + cos * (y - center.y) + center.y;
		x = temp;
	}
	NH_INLINE Vector2 Rotated(const Vector2& center, F32 angle) const
	{
		F32 cos = Math::Cos(angle);
		F32 sin = Math::Sin(angle);
		return Vector2{ cos * (x - center.x) - sin * (y - center.y) + center.x,
		sin * (x - center.x) + cos * (y - center.y) + center.y };
	}
	NH_INLINE Vector2& Clamp(const Vector2& xBound, const Vector2& yBound)
	{
		x = Math::Clamp(x, xBound.x, xBound.y);
		y = Math::Clamp(y, yBound.x, yBound.y);
		return *this;
	}
	NH_INLINE Vector2 Clamped(const Vector2& xBound, const Vector2& yBound) const
	{
		return {
			Math::Clamp(x, xBound.x, xBound.y),
			Math::Clamp(y, yBound.x, yBound.y)
		};
	}
	NH_INLINE Vector2& SetClosest(const Vector2& xBound, const Vector2& yBound)
	{
		x = Math::Closest(x, xBound.x, xBound.y);
		y = Math::Closest(y, yBound.x, yBound.y);
		return *this;
	}
	NH_INLINE Vector2 Closest(const Vector2& xBound, const Vector2& yBound) const
	{
		return {
			Math::Closest(x, xBound.x, xBound.y),
			Math::Closest(y, yBound.x, yBound.y)
		};
	}

	NH_INLINE F32* Data() { return &x; }
	NH_INLINE const F32* Data() const { return &x; }

	static const Vector2 ONE;
	static const Vector2 ZERO;
	static const Vector2 RIGHT;
	static const Vector2 LEFT;
	static const Vector2 UP;
	static const Vector2 DOWN;
};

struct NH_API Vector3
{
	F32 x, y, z;

	Vector3() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f } {}
	Vector3(F32 f) : x{ f }, y{ f }, z{ f } {}
	Vector3(F32 x, F32 y, F32 z) : x{ x }, y{ y }, z{ z } {}
	Vector3(const Vector3& v) : x{ v.x }, y{ v.y }, z{ v.z } {}
	Vector3(Vector3&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z } {}
	Vector3(const struct String& str);

	Vector3& operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z;											return *this; }
	Vector3& operator=(Vector3&& v) noexcept { x = v.x; y = v.y; z = v.z;										return *this; }
	Vector3& operator=(const struct String& str);

	Vector3& operator+= (const Vector3& v) { x += v.x; y += v.y; z += v.z;										return *this; }
	Vector3& operator-= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z;										return *this; }
	Vector3& operator*= (F32 f) { x *= f; y *= f; z *= f;														return *this; }
	Vector3& operator/= (F32 f) { x /= f; y /= f; z /= f;														return *this; }
	Vector3& operator%= (F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f);				return *this; }
	Vector3& operator*= (F64 f) { x *= (F32)f; y *= (F32)f; z *= (F32)f;										return *this; }
	Vector3& operator/= (F64 f) { x /= (F32)f; y /= (F32)f; z /= (F32)f;										return *this; }
	Vector3& operator%= (F64 f) { x = Math::Mod(x, (F32)f); y = Math::Mod(y, (F32)f); z = Math::Mod(z, (F32)f);	return *this; }

	Vector3 operator+ (const Vector3& v) const { return Vector3{ x + v.x, y + v.y, z + v.z }; }
	Vector3 operator- (const Vector3& v) const { return Vector3{ x - v.x, y - v.y, z - v.z }; }
	Vector3 operator* (F32 f) const { return Vector3{ x * f, y * f, z * f }; }
	Vector3 operator/ (F32 f) const { return Vector3{ x / f, y / f, z / f }; }
	Vector3 operator% (F32 f) const { return Vector3{ Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f) }; }

	bool operator== (const Vector3& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y) && Math::Zero(z - v.z); }
	bool operator!= (const Vector3& v) const { return Math::Zero(x - v.x) || Math::Zero(y - v.y) || Math::Zero(z - v.z); }
	bool operator<  (const Vector3& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	bool operator>  (const Vector3& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	friend Vector3 operator- (Vector3& v) { return Vector3{ -v.x, -v.y, -v.z }; }

	NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

	NH_INLINE F32 Dot(const Vector3& v) const { return v.x * x + v.y * y + v.z * z; }
	NH_INLINE Vector3 Cross(const Vector3& v) const { return Vector3{ y * v.z - v.y * z, z * v.x - v.z * x, x * v.y - v.x * y }; }
	NH_INLINE F32 Magnitude() const { return Math::Sqrt(Dot(*this)); }
	NH_INLINE F32 SqrMagnitude() const { return Dot(*this); }
	NH_INLINE void Normalize() { Vector3 v = Normalized(); x = v.x; y = v.y; z = v.z; }
	NH_INLINE Vector3 Normalized() const { return Math::Zero(x) && Math::Zero(y) && Math::Zero(z) ? Vector3::ZERO : (*this) / Magnitude(); }
	NH_INLINE F32 AngleBetween(const Vector3& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
	NH_INLINE Vector3 Projection(const Vector3& v) const { return v * (Dot(v) / v.Dot(v)); }
	NH_INLINE Vector3 OrthoProjection(const Vector3& v) const { return *this - Projection(v); }
	NH_INLINE Vector3& Clamp(const Vector2& xBound, const Vector2& yBound, const Vector2& zBound)
	{
		x = Math::Clamp(x, xBound.x, xBound.y);
		y = Math::Clamp(y, yBound.x, yBound.y);
		z = Math::Clamp(z, zBound.x, zBound.y);
		return *this;
	}

	NH_INLINE F32* Data() { return &x; }
	NH_INLINE const F32* Data() const { return &x; }

	static const Vector3 ONE;
	static const Vector3 ZERO;
	static const Vector3 RIGHT;
	static const Vector3 LEFT;
	static const Vector3 UP;
	static const Vector3 DOWN;
	static const Vector3 FORWARD;
	static const Vector3 BACK;
};

struct NH_API Vector4
{
	F32 x, y, z, w;

	Vector4() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 0.0f } {}
	Vector4(F32 f) : x{ f }, y{ f }, z{ f }, w{ f } {}
	Vector4(F32 x, F32 y, F32 z, F32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
	Vector4(const Vector4& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
	Vector4(const Vector3& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ 0.0f } {}
	Vector4(Vector4&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
	Vector4(const struct String& str);

	Vector4& operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
	Vector4& operator=(Vector4&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
	Vector4& operator=(const struct String& str);

	Vector4& operator+= (const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	Vector4& operator-= (const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	Vector4& operator*= (F32 f) { x *= f; y *= f; z *= f; w *= f;					return *this; }
	Vector4& operator/= (F32 f) { x /= f; y /= f; z /= f; w /= f;					return *this; }
	Vector4& operator%= (F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); w = Math::Mod(w, f);  return *this; }

	Vector4 operator+ (const Vector4& v) const { return Vector4{ x + v.x, y + v.y, z + v.z, w + v.w }; }
	Vector4 operator- (const Vector4& v) const { return Vector4{ x - v.x, y - v.y, z - v.z, w - v.w }; }
	Vector4 operator* (const Vector4& v) const { return Vector4{ x * v.x, y * v.y, z * v.z, w * v.w }; }
	Vector4 operator/ (const Vector4& v) const { return Vector4{ x / v.x, y / v.y, z / v.z, w / v.w }; }
	Vector4 operator+ (F32 f) const { return Vector4{ x + f, y + f, z + f, w + f }; }
	Vector4 operator- (F32 f) const { return Vector4{ x - f, y - f, z - f, w - f }; }
	Vector4 operator* (F32 f) const { return Vector4{ x * f, y * f, z * f, w * f }; }
	Vector4 operator/ (F32 f) const { return Vector4{ x / f, y / f, z / f, w / f }; }
	Vector4 operator% (F32 f) const { return Vector4{ Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f), Math::Mod(w, f) }; }

	bool operator== (const Vector4& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y) && Math::Zero(z - v.z) && Math::Zero(w - v.w); }
	bool operator!= (const Vector4& v) const { return Math::Zero(x - v.x) || Math::Zero(y - v.y) || Math::Zero(z - v.z) || Math::Zero(w - v.w); }
	bool operator<  (const Vector4& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	bool operator>  (const Vector4& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	friend Vector4 operator- (Vector4& v) { return Vector4{ -v.x, -v.y, -v.z, -v.w }; }

	NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

	NH_INLINE F32 Dot(const Vector4& v) const { return v.x * x + v.y * y + v.z * z + v.w * w; }
	NH_INLINE F32 Magnitude() const { return Math::Sqrt(Dot(*this)); }
	NH_INLINE F32 SqrMagnitude() const { return Dot(*this); }
	NH_INLINE void Normalize() { Vector4 v = Normalized(); x = v.x; y = v.y; z = v.z; w = v.w; }
	NH_INLINE Vector4 Normalized() const { return Math::Zero(x) && Math::Zero(y) && Math::Zero(z) && Math::Zero(w) ? Vector4::ZERO : (*this) / Magnitude(); }
	NH_INLINE F32 AngleBetween(const Vector4& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
	NH_INLINE Vector4 Projection(const Vector4& v) const { return v * (Dot(v) / v.Dot(v)); }
	NH_INLINE Vector4 OrthoProjection(const Vector4& v) const { return *this - Projection(v); }
	NH_INLINE Vector4& Clamp(const Vector2& xBound, const Vector2& yBound, const Vector2& zBound, const Vector2& wBound)
	{
		x = Math::Clamp(x, xBound.x, xBound.y);
		y = Math::Clamp(y, yBound.x, yBound.y);
		z = Math::Clamp(z, zBound.x, zBound.y);
		w = Math::Clamp(w, wBound.x, wBound.y);
		return *this;
	}

	NH_INLINE F32* Data() { return &x; }
	NH_INLINE const F32* Data() const { return &x; }

	static const Vector4 ONE;
	static const Vector4 ZERO;
	static const Vector4 RIGHT;
	static const Vector4 LEFT;
	static const Vector4 UP;
	static const Vector4 DOWN;
	static const Vector4 FORWARD;
	static const Vector4 BACK;
	static const Vector4 OUTWARD;
	static const Vector4 INWARD;
};

struct NH_API Vector2Int
{
	I32 x, y;

	Vector2Int() : x{ 0 }, y{ 0 } {}
	Vector2Int(I32 i) : x{ i }, y{ i } {}
	Vector2Int(I32 x, I32 y) : x{ x }, y{ y } {}
	Vector2Int(const Vector2Int& v) : x{ v.x }, y{ v.y } {}
	Vector2Int(Vector2Int&& v) noexcept : x{ v.x }, y{ v.y } {}
	Vector2Int(const struct String& str);

	Vector2Int& operator= (const Vector2Int& v) { x = v.x; y = v.y;	return *this; }
	Vector2Int& operator= (Vector2Int&& v) noexcept { x = v.x; y = v.y; return *this; }
	Vector2Int& operator= (const struct String& str);
	Vector2Int& operator= (I32 i) { x = i; y = i; return *this; }

	Vector2Int& operator+= (const Vector2Int& v) { x += v.x; y += v.y;	return *this; }
	Vector2Int& operator-= (const Vector2Int& v) { x -= v.x; y -= v.y;	return *this; }
	Vector2Int& operator*= (I32 i) { x *= i; y *= i;					return *this; }
	Vector2Int& operator/= (I32 i) { x /= i; y /= i;					return *this; }
	Vector2Int& operator%= (I32 i) { x %= i; y %= i;					return *this; }

	Vector2Int operator+ (const Vector2Int& v) const { return Vector2Int{ x + v.x, y + v.y }; }
	Vector2Int operator- (const Vector2Int& v) const { return Vector2Int{ x - v.x, y - v.y }; }
	Vector2Int operator* (I32 i) const { return Vector2Int{ x * i, y * i }; }
	Vector2Int operator/ (I32 i) const { return Vector2Int{ x / i, y / i }; }
	Vector2Int operator% (I32 i) const { return Vector2Int{ x % i, y % i }; }

	bool operator== (const Vector2Int& v) const { return x == v.x && y == v.y; }
	bool operator!= (const Vector2Int& v) const { return x != v.x || y != v.y; }
	bool operator<  (const Vector2Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	bool operator>  (const Vector2Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	friend Vector2Int operator- (Vector2Int& v) { return Vector2Int{ -v.x, -v.y }; }

	NH_INLINE const I32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE I32& operator[] (U8 i) { return ((&x)[i]); }

	NH_INLINE I32 Dot(const Vector2Int& v) const { return v.x * x + v.y * y; }
	NH_INLINE F32 Magnitude() const { return Math::Sqrt((F32)Dot(*this)); }
	NH_INLINE F32 SqrMagnitude() const { return (F32)Dot(*this); }
	NH_INLINE Vector2Int& Clamp(const Vector2Int& xBound, const Vector2Int& yBound)
	{
		x = Math::Clamp(x, xBound.x, xBound.y);
		y = Math::Clamp(y, yBound.x, yBound.y);
		return *this;
	}

	NH_INLINE I32* Data() { return &x; }
	NH_INLINE const I32* Data() const { return &x; }

	static const Vector2Int ONE;
	static const Vector2Int ZERO;
	static const Vector2Int RIGHT;
	static const Vector2Int LEFT;
	static const Vector2Int UP;
	static const Vector2Int DOWN;
};

struct NH_API Vector3Int
{
	I32 x, y, z;

	Vector3Int() : x{ 0 }, y{ 0 }, z{ 0 } {}
	Vector3Int(I32 i) : x{ i }, y{ i }, z{ i } {}
	Vector3Int(I32 x, I32 y, I32 z) : x{ x }, y{ y }, z{ z } {}
	Vector3Int(const Vector3Int& v) : x{ v.x }, y{ v.y }, z{ v.z } {}
	Vector3Int(Vector3Int&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z } {}
	Vector3Int(const struct String& str);

	Vector3Int& operator=  (const Vector3Int& v) { x = v.x; y = v.y; z = v.z;	return *this; }
	Vector3Int& operator=  (Vector3Int&& v) noexcept { x = v.x; y = v.y; z = v.z;         return *this; }
	Vector3Int& operator=(const struct String& str);

	Vector3Int& operator+= (const Vector3Int& v) { x += v.x; y += v.y; z += v.z;	return *this; }
	Vector3Int& operator-= (const Vector3Int& v) { x -= v.x; y -= v.y; z -= v.z;	return *this; }
	Vector3Int& operator*= (I32 i) { x *= i; y *= i; z *= i;					    return *this; }
	Vector3Int& operator/= (I32 i) { x /= i; y /= i; z /= i;					    return *this; }
	Vector3Int& operator%= (I32 i) { x %= i; y %= i;	z %= i;				        return *this; }

	Vector3Int operator+ (const Vector3Int& v) const { return Vector3Int{ x + v.x, y + v.y, z + v.z }; }
	Vector3Int operator- (const Vector3Int& v) const { return Vector3Int{ x - v.x, y - v.y, z - v.z }; }
	Vector3Int operator* (I32 i) const { return Vector3Int{ x * i, y * i, z * i }; }
	Vector3Int operator/ (I32 i) const { return Vector3Int{ x / i, y / i, z / i }; }
	Vector3Int operator% (I32 i) const { return Vector3Int{ x % i, y % i, z % i }; }

	bool operator== (const Vector3Int& v) const { return x == v.x && y == v.y && z == v.z; }
	bool operator!= (const Vector3Int& v) const { return x != v.x || y != v.y || z != v.z; }
	bool operator<  (const Vector3Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	bool operator>  (const Vector3Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	friend Vector3Int operator- (Vector3Int& v) { return Vector3Int{ -v.x, -v.y, -v.z }; }

	NH_INLINE const I32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE I32& operator[] (U8 i) { return ((&x)[i]); }

	NH_INLINE I32 Dot(const Vector3Int& v) const { return v.x * x + v.y * y + v.z * z; }
	NH_INLINE F32 Magnitude() const { return Math::Sqrt((F32)Dot(*this)); }
	NH_INLINE F32 SqrMagnitude() const { return (F32)Dot(*this); }
	NH_INLINE Vector3Int& Clamp(const Vector2Int& xBound, const Vector2Int& yBound, const Vector2Int& zBound)
	{
		x = Math::Clamp(x, xBound.x, xBound.y);
		y = Math::Clamp(y, yBound.x, yBound.y);
		z = Math::Clamp(z, zBound.x, zBound.y);
		return *this;
	}

	NH_INLINE I32* Data() { return &x; }
	NH_INLINE const I32* Data() const { return &x; }

	static const Vector3Int ONE;
	static const Vector3Int ZERO;
	static const Vector3Int RIGHT;
	static const Vector3Int LEFT;
	static const Vector3Int UP;
	static const Vector3Int DOWN;
	static const Vector3Int FORWARD;
	static const Vector3Int BACK;
};

struct Vector4Int
{
	I32 x, y, z, w;

	Vector4Int() : x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 } {}
	Vector4Int(I32 i) : x{ i }, y{ i }, z{ i }, w{ i } {}
	Vector4Int(I32 x, I32 y, I32 z, I32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
	Vector4Int(const Vector4Int& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
	Vector4Int(Vector4Int&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
	Vector4Int(const struct String& str);

	Vector4Int& operator=  (const Vector4Int& v) { x = v.x; y = v.y; z = v.z; w = v.w;	return *this; }
	Vector4Int& operator=  (Vector4Int&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w;        return *this; }
	Vector4Int& operator=(const struct String& str);

	Vector4Int& operator+= (const Vector4Int& v) { x += v.x; y += v.y; z += v.z; w += v.w;	return *this; }
	Vector4Int& operator-= (const Vector4Int& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w;	return *this; }
	Vector4Int& operator*= (I32 i) { x *= i; y *= i; z *= i;	w *= i;				    return *this; }
	Vector4Int& operator/= (I32 i) { x /= i; y /= i; z /= i;	w /= i;				    return *this; }
	Vector4Int& operator%= (I32 i) { x %= i; y %= i;	z %= i;	w %= i;			        return *this; }

	Vector4Int operator+ (const Vector4Int& v) const { return Vector4Int{ x + v.x, y + v.y, z + v.z, w + v.w }; }
	Vector4Int operator- (const Vector4Int& v) const { return Vector4Int{ x - v.x, y - v.y, z - v.z, w - v.w }; }
	Vector4Int operator* (I32 i) const { return Vector4Int{ x * i, y * i, z * i, w * i }; }
	Vector4Int operator/ (I32 i) const { return Vector4Int{ x / i, y / i, z / i, w / i }; }
	Vector4Int operator% (I32 i) const { return Vector4Int{ x % i, y % i, z % i, w % i }; }

	bool operator== (const Vector4Int& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
	bool operator!= (const Vector4Int& v) const { return x != v.x || y != v.y || z != v.z || w != v.w; }
	bool operator<  (const Vector4Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	bool operator>  (const Vector4Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	friend Vector4Int operator- (Vector4Int& v) { return Vector4Int{ -v.x, -v.y, -v.z, -v.w }; }

	NH_INLINE const I32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE I32& operator[] (U8 i) { return ((&x)[i]); }

	NH_INLINE I32 Dot(const Vector4Int& v) const { return v.x * x + v.y * y + v.z * z + v.w * w; }
	NH_INLINE F32 Magnitude() const { return Math::Sqrt((F32)Dot(*this)); }
	NH_INLINE F32 SqrMagnitude() const { return (F32)Dot(*this); }
	NH_INLINE Vector4Int& Clamp(const Vector2Int& xBound, const Vector2Int& yBound, const Vector2Int& zBound, const Vector2Int& wBound)
	{
		x = Math::Clamp(x, xBound.x, xBound.y);
		y = Math::Clamp(y, yBound.x, yBound.y);
		z = Math::Clamp(z, zBound.x, zBound.y);
		w = Math::Clamp(w, wBound.x, wBound.y);
		return *this;
	}

	NH_INLINE I32* Data() { return &x; }
	NH_INLINE const I32* Data() const { return &x; }

	static const Vector4Int ONE;
	static const Vector4Int ZERO;
	static const Vector4Int RIGHT;
	static const Vector4Int LEFT;
	static const Vector4Int UP;
	static const Vector4Int DOWN;
	static const Vector4Int FORWARD;
	static const Vector4Int BACK;
	static const Vector4Int OUTWARD;
	static const Vector4Int INWARD;
};

NH_INLINE Vector2 Math::Lerp(const Vector2& a, const Vector2& b, F32 t) { return a + (b - a) * t; }
NH_INLINE Vector3 Math::Lerp(const Vector3& a, const Vector3& b, F32 t) { return a + (b - a) * t; }
NH_INLINE Vector4 Math::Lerp(const Vector4& a, const Vector4& b, F32 t) { return a + (b - a) * t; }

struct NH_API Matrix2
{
	Vector2 a, b; //Columns

	Matrix2() : a{ 1.0f, 0.0f }, b{ 0.0f, 1.0f } {}
	Matrix2(F32 n) : a{ n, 0.0f }, b{ 0.0f, n } {}
	Matrix2(F32 ax, F32 ay, F32 bx, F32 by) : a{ ax, ay }, b{ bx, by } {}
	Matrix2(const Vector2& v1, const Vector2& v2) : a{ v1 }, b{ v2 } {}
	Matrix2(Vector2&& v1, Vector2&& v2) : a{ v1 }, b{ v2 } {}
	Matrix2(const Matrix2& m) : a{ m.a }, b{ m.b } {}
	Matrix2(Matrix2&& m) noexcept : a{ m.a }, b{ m.b } {}

	Matrix2& operator= (const Matrix2& m) { a = m.a; b = m.b; return *this; }
	Matrix2& operator= (Matrix2&& m) noexcept { a = m.a; b = m.b; return *this; }

	Matrix2& operator+= (const Matrix2& m) { a += m.a; b += m.b; return *this; }
	Matrix2& operator-= (const Matrix2& m) { a -= m.a; b -= m.b; return *this; }
	Matrix2& operator*= (const Matrix2& m)
	{
		a.x = a.x * m.a.x + b.x * m.a.y;
		a.y = a.y * m.a.x + b.y * m.a.y;
		b.x = a.x * m.b.x + b.x * m.b.y;
		b.y = a.y * m.b.x + b.y * m.b.y;

		return *this;
	}

	Matrix2 operator+(const Matrix2& m) const { return Matrix2{ a + m.a, b + m.b }; }
	Matrix2 operator-(const Matrix2& m) const { return Matrix2{ a - m.a, b - m.b }; }
	Matrix2 operator*(const Matrix2& m) const
	{
		return Matrix2{
			a.x * m.a.x + b.x * m.a.y,
			a.y * m.a.x + b.y * m.a.y,
			a.x * m.b.x + b.x * m.b.y,
			a.y * m.b.x + b.y * m.b.y
		};
	}
	Vector2 operator*(const Vector2& v) const
	{
		return Vector2{
			a.x * v.x + b.x * v.y,
			a.y * v.x + b.y * v.y
		};
	}

	friend Matrix2 operator- (Matrix2& m) { return Matrix2{ -m.a, -m.b }; }
	bool operator==(const Matrix2& m) const { return a == m.a && b == m.b; }
	bool operator!=(const Matrix2& m) const { return a != m.a || b != m.b; }

	NH_INLINE const Vector2& operator[] (U8 i) const { return ((&a)[i]); }
	NH_INLINE Vector2& operator[] (U8 i) { return ((&a)[i]); }

	NH_INLINE F32* Data() { return a.Data(); }
	NH_INLINE const F32* Data() const { return a.Data(); }

	static const Matrix2 IDENTITY;
};

struct NH_API Matrix3
{
	Vector3 a, b, c; //Columns

	Matrix3() : a{ 1.0f, 0.0f, 0.0f }, b{ 0.0f, 1.0f, 0.0f }, c{ 0.0f, 0.0f, 1.0f } {}
	Matrix3(F32 n) : a{ n, 0.0f, 0.0f }, b{ 0.0f, n, 0.0f }, c{ 0.0f, 0.0f, n } {}
	Matrix3(F32 ax, F32 ay, F32 az, F32 bx, F32 by, F32 bz, F32 cx, F32 cy, F32 cz) : a{ ax, ay, az }, b{ bx, by, bz }, c{ cx, cy, cz } {}
	Matrix3(const Vector3& a, const Vector3& b, const Vector3& c) : a{ a }, b{ b }, c{ c } {}
	Matrix3(Vector3&& v1, Vector3&& v2, Vector3&& v3) : a{ v1 }, b{ v2 }, c{ v3 } {}
	Matrix3(const Matrix3& m) : a{ m.a }, b{ m.b }, c{ m.c } {}
	Matrix3(Matrix3&& m) noexcept : a{ m.a }, b{ m.b }, c{ m.c } {}
	Matrix3(const Vector2& position, const Quaternion2D& rotation, const Vector2& scale);
	Matrix3(const Vector2& position, const F32& rotation, const Vector2& scale)
	{
		F32 cos = (F32)Math::Cos(rotation * DEG2RAD_MULTIPLIER);
		F32 sin = (F32)Math::Sin(rotation * DEG2RAD_MULTIPLIER);
		a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
		a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
		a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
	}


	Matrix3& operator= (const Matrix3& m) { a = m.a; b = m.b; c = m.c; return *this; }
	Matrix3& operator= (Matrix3&& m) noexcept { a = m.a; b = m.b; c = m.c; return *this; }

	Matrix3& operator+= (const Matrix3& m) { a += m.a; b += m.b; c += m.c; return *this; }
	Matrix3& operator-= (const Matrix3& m) { a -= m.a; b -= m.b; c -= m.c; return *this; }
	Matrix3& operator*= (const Matrix3& m)
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

	Matrix3 operator+(const Matrix3& m) const { return Matrix3{ a + m.a, b + m.b, c + m.c }; }
	Matrix3 operator-(const Matrix3& m) const { return Matrix3{ a - m.a, b - m.b, c - m.c }; }
	Matrix3 operator*(const Matrix3& m) const
	{
		return Matrix3{
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
	Vector3 operator*(const Vector3& v) const
	{
		return Vector3{
			a.x * v.x + b.x * v.y + c.x * v.z,
			a.y * v.x + b.y * v.y + c.y * v.z,
			a.z * v.x + b.z * v.y + c.z * v.z
		};
	}

	friend Matrix3 operator- (Matrix3& m) { return Matrix3{ -m.a, -m.b, -m.c }; }
	bool operator==(const Matrix3& m) const { return a == m.a && b == m.b && c == m.c; }
	bool operator!=(const Matrix3& m) const { return a != m.a || b != m.b || c != m.c; }

	NH_INLINE const Vector3& operator[] (U8 i) const { return ((&a)[i]); }
	NH_INLINE Vector3& operator[] (U8 i) { return ((&a)[i]); }

	NH_INLINE F32* Data() { return a.Data(); }
	NH_INLINE const F32* Data() const { return a.Data(); }

	static const Matrix3 IDENTITY;
};

struct NH_API Matrix4
{
	Vector4 a, b, c, d; //Columns

	Matrix4() : a{ 1.0f, 0.0f, 0.0f, 0.0f }, b{ 0.0f, 1.0f, 0.0f, 0.0f }, c{ 0.0f, 0.0f, 1.0f, 0.0f }, d{ 0.0f, 0.0f, 0.0f, 1.0f } {}
	Matrix4(F32 n) : a{ n, 0.0f, 0.0f, 0.0f }, b{ 0.0f, n, 0.0f, 0.0f }, c{ 0.0f, 0.0f, n, 0.0f }, d{ 0.0f, 0.0f, 0.0f, n } {}
	Matrix4(F32 ax, F32 ay, F32 az, F32 aw, F32 bx, F32 by, F32 bz, F32 bw, F32 cx, F32 cy, F32 cz, F32 cw, F32 dx, F32 dy, F32 dz, F32 dw) :
		a{ ax, ay, az, aw }, b{ bx, by, bz, bw }, c{ cx, cy, cz, cw }, d{ dx, dy, dz, dw }
	{
	}
	Matrix4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d) : a{ a }, b{ b }, c{ c }, d{ d } {}
	Matrix4(Vector4&& a, Vector4&& b, Vector4&& c, Vector4&& d) : a{ a }, b{ b }, c{ c }, d{ d } {}
	Matrix4(const Matrix4& m) : a{ m.a }, b{ m.b }, c{ m.c }, d{ m.d } {}
	Matrix4(const Matrix3& m)
	{
		a.x = m.a.x; b.x = m.b.x; c.x = 0.0f; d.x = m.c.x;
		a.y = m.a.y; b.y = m.b.y; c.y = 0.0f; d.y = m.c.y;
		a.z = 0.0f;  b.z = 0.0f;  c.z = 1.0f; d.z = 0.0f;
		a.w = 0.0f;  b.w = 0.0f;  c.w = 0.0f; d.w = 1.0f;
	}
	Matrix4(Matrix4&& m) noexcept : a{ m.a }, b{ m.b }, c{ m.c }, d{ m.d } {}
	Matrix4(const Vector3& position, const Vector3& rotation = Vector3::ZERO, const Vector3& scale = Vector3::ONE)
	{
		//TODO:
		a.x = 1.0f; b.x = 0.0f; c.x = 0.0f; d.x = position.x;
		a.y = 0.0f; b.y = 1.0f; c.y = 0.0f; d.y = position.y;
		a.z = 0.0f; b.z = 0.0f; c.z = 1.0f; d.z = position.z;
		a.w = 0.0f; b.w = 0.0f; c.w = 0.0f; d.w = 1.0f;
	}
	Matrix4(const Vector3& position, const Quaternion3D& rotation, const Vector3& scale = Vector3::ONE);

	Matrix4& operator= (const Matrix4& m) { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
	Matrix4& operator= (Matrix4&& m) noexcept { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }

	Matrix4& operator+= (const Matrix4& m) { a += m.a; b += m.b; c += m.c; d += m.d; return *this; }
	Matrix4& operator-= (const Matrix4& m) { a -= m.a; b -= m.b; c -= m.c; d -= m.d; return *this; }
	Matrix4& operator*= (const Matrix4& m)
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

	Matrix4 operator+(const Matrix4& m) const { return Matrix4{ a + m.a, b + m.b, c + m.c, d + m.d }; }
	Matrix4 operator-(const Matrix4& m) const { return Matrix4{ a - m.a, b - m.b, c - m.c, d + m.d }; }
	Matrix4 operator*(const Matrix4& m) const
	{
		return Matrix4{
			a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + c.x * m.a.w,
			a.y * m.a.x + b.y * m.a.y + c.y * m.a.z + c.y * m.a.w,
			a.z * m.a.x + b.z * m.a.y + c.z * m.a.z + c.z * m.a.w,
			a.w * m.a.x + b.w * m.a.y + c.w * m.a.z + c.w * m.a.w,
			a.x * m.b.x + b.x * m.b.y + c.x * m.b.z + c.x * m.b.w,
			a.y * m.b.x + b.y * m.b.y + c.y * m.b.z + c.y * m.b.w,
			a.z * m.b.x + b.z * m.b.y + c.z * m.b.z + c.z * m.b.w,
			a.w * m.b.x + b.w * m.b.y + c.w * m.b.z + c.w * m.b.w,
			a.x * m.c.x + b.x * m.c.y + c.x * m.c.z + c.x * m.c.w,
			a.y * m.c.x + b.y * m.c.y + c.y * m.c.z + c.y * m.c.w,
			a.z * m.c.x + b.z * m.c.y + c.z * m.c.z + c.z * m.c.w,
			a.w * m.c.x + b.w * m.c.y + c.w * m.c.z + c.w * m.c.w,
			a.x * m.d.x + b.x * m.d.y + c.x * m.d.z + c.x * m.d.w,
			a.y * m.d.x + b.y * m.d.y + c.y * m.d.z + c.y * m.d.w,
			a.z * m.d.x + b.z * m.d.y + c.z * m.d.z + c.z * m.d.w,
			a.w * m.d.x + b.w * m.d.y + c.w * m.d.z + c.w * m.d.w
		};
	}
	Vector4 operator*(const Vector4& v) const
	{
		return Vector4{
			a.x * v.x + b.x * v.y + c.x * v.z + c.x * v.z,
			a.y * v.x + b.y * v.y + c.y * v.z + c.y * v.z,
			a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.z,
			a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.z
		};
	}

	void SetTranslation(const Vector3& v)
	{
		d.x = v.x;
		d.y = v.y;
		d.z = v.z;
	}
	void Translate(const Vector3& v)
	{
		d.x += v.x;
		d.y += v.y;
		d.z += v.z;
	}
	void SetScale(const Vector3& v)
	{

	}
	void Scale(const Vector3& v)
	{

	}
	//TODO: Rotate

	Matrix4 Inverse() const
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

		F32 f = 1.0f / (a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + d.x * m.a.w);

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

	void Invert()
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

		F32 f = 1.0f / (a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + d.x * m.a.w);

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

		a = m.a;
		b = m.b;
		c = m.c;
		d = m.d;
	}

	void Transpose()
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
	}

	void SetPerspective(F32 fov, F32 aspectRatio, F32 near, F32 far)
	{
		F32 halfTanFov = Math::Tan(fov * 0.5f);

		a.x = 1.0f / (aspectRatio * halfTanFov);
		a.y = 0.0f;
		a.z = 0.0f;
		a.w = 0.0f;
		b.x = 0.0f;
		b.y = 1.0f / halfTanFov;
		b.z = 0.0f;
		b.w = 0.0f;
		c.x = 0.0f;
		c.y = 0.0f;
		c.z = -(far + near) / (far - near);
		c.w = -(2.0f * far * near) / (far - near);
		d.y = 0.0f;
		d.z = 0.0f;
		d.w = -1.0f;
		d.x = 0.0f;
	}

	void SetOrthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far)
	{
		a.x = 2.0f / (right - left);
		a.y = 0.0f;
		a.z = 0.0f;
		a.w = -(right + left) / (right - left);
		b.x = 0.0f;
		b.y = 2.0f / (top - bottom);
		b.z = 0.0f;
		b.w = -(top + bottom) / (top - bottom);
		c.x = 0.0f;
		c.y = 0.0f;
		c.z = -2.0f / (far - near);
		c.w = -(far + near) / (far - near);
		d.x = 0.0f;
		d.y = 0.0f;
		d.z = 0.0f;
		d.w = 1.0f;
	}

	Vector3 Forward() { return Vector3(-a.z, -b.z, -c.z).Normalized(); }
	Vector3 Back() { return Vector3(a.z, b.z, c.z).Normalized(); }
	Vector3 Right() { return Vector3(a.x, b.x, c.x).Normalized(); }
	Vector3 Left() { return Vector3(-a.x, -b.x, -c.x).Normalized(); }
	Vector3 Up() { return Vector3(a.y, b.y, c.y).Normalized(); }
	Vector3 Down() { return Vector3(-a.y, -b.y, -c.y).Normalized(); }

	friend Matrix4 operator- (Matrix4& m) { return Matrix4{ -m.a, -m.b, -m.c, -m.d }; }
	bool operator==(const Matrix4& m) const { return a == m.a && b == m.b && c == m.c && d == m.d; }
	bool operator!=(const Matrix4& m) const { return a != m.a || b != m.b || c != m.c || d != m.d; }

	NH_INLINE const Vector4& operator[] (U8 i) const { return ((&a)[i]); }
	NH_INLINE Vector4& operator[] (U8 i) { return ((&a)[i]); }

	NH_INLINE F32* Data() { return a.Data(); }
	NH_INLINE const F32* Data() const { return a.Data(); }

	static const Matrix4 IDENTITY;
};

struct NH_API Quaternion3D
{
	F32 x, y, z, w;

	Quaternion3D() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 1.0f } {}
	Quaternion3D(F32 x, F32 y, F32 z, F32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
	Quaternion3D(const Quaternion3D& q) : x{ q.x }, y{ q.y }, z{ q.z }, w{ q.w } {}
	Quaternion3D(Quaternion3D&& q) noexcept : x{ q.x }, y{ q.y }, z{ q.z }, w{ q.w } {}

	Quaternion3D& operator=(const Quaternion3D& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
	Quaternion3D& operator=(Quaternion3D&& q) noexcept { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }

	static NH_INLINE Quaternion3D AxisAngle(const Vector3& axis, F32 angle, bool normalize);
	static NH_INLINE Quaternion3D AxisAngle(const Vector2& axis, F32 angle, bool normalize);

	Quaternion3D operator* (const Quaternion3D& q) const
	{
		return Quaternion3D{ x * q.w + y * q.z - z * q.y + w * q.x,
				-x * q.z + y * q.w + z * q.x + w * q.y,
				x * q.y - y * q.x + z * q.w + w * q.z,
				-x * q.x - y * q.y - z * q.z + w * q.w };
	}

	NH_INLINE Matrix4 ToMatrix4() const;
	NH_INLINE Matrix3 ToMatrix3() const;
	NH_INLINE Matrix4 RotationMatrix(Vector3 center) const;
	NH_INLINE Quaternion3D Slerp(const Quaternion3D& q, F32 t) const;

	NH_INLINE F32 Dot(const Quaternion3D& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
	NH_INLINE F32 Normal() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
	NH_INLINE Quaternion3D Normalized() const { F32 normal = 1.0f / Normal(); return Quaternion3D{ x * normal, y * normal, z * normal, w * normal }; }
	NH_INLINE void Normalize() { F32 normal = 1.0f / Normal(); x *= normal; y *= normal; z *= normal; w *= normal; }
	NH_INLINE Quaternion3D Conjugate() const { return Quaternion3D{ -x, -y, -z, w }; }
	NH_INLINE Quaternion3D Inverse() const { return Conjugate().Normalized(); }

	NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
	NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

	static const Quaternion3D IDENTITY;
};

struct NH_API Quaternion2D
{
	F32 angle;
	F32 sin, cos;

	Quaternion2D() : angle{ 0.0f }, sin{ 0.0f }, cos{ 1.0f } {}
	Quaternion2D(F32 sin, F32 cos) : angle{ Math::Acos(cos) }, sin{ sin }, cos{ cos } {}
	Quaternion2D(const Quaternion2D& q) : angle{ q.angle }, sin{ q.sin }, cos{ q.cos } {}
	Quaternion2D(Quaternion2D&& q) noexcept : angle{ q.angle }, sin{ q.sin }, cos{ q.cos } {}
	explicit Quaternion2D(F32 angle) : angle{ (F32)(DEG2RAD_MULTIPLIER * angle) }, sin{ Math::Sin(this->angle) }, cos{ Math::Cos(this->angle) } {}

	Quaternion2D& operator=(F32 angle) { this->angle = (F32)(DEG2RAD_MULTIPLIER * angle); sin = Math::Sin(this->angle); cos = Math::Cos(this->angle); return *this; }
	Quaternion2D& operator=(const Quaternion2D& q) { sin = q.sin; cos = q.cos; angle = q.angle; return *this; }
	Quaternion2D& operator=(Quaternion2D&& q) noexcept { sin = q.sin; cos = q.cos; angle = q.angle; return *this; }

	void Set(F32 angle)
	{
		this->angle = (F32)(DEG2RAD_MULTIPLIER * angle);
		sin = Math::Sin(this->angle);
		cos = Math::Cos(this->angle);
	}
	void Rotate(F32 angle)
	{
		this->angle += (F32)(DEG2RAD_MULTIPLIER * angle);
		sin = Math::Sin(this->angle);
		cos = Math::Cos(this->angle);
	}

	NH_INLINE const F32& operator[] (U8 i) const { return ((&sin)[i]); }
	NH_INLINE F32& operator[] (U8 i) { return ((&sin)[i]); }

	static const Quaternion2D IDENTITY;
};

struct NH_API Vertex
{
	Vertex(Vector3 position, Vector2 uv, Vector3 normal = Vector3::ZERO, Vector4 color = Vector4::ONE, Vector4 tangent = Vector4::ZERO) :
		position{ position }, normal{ normal }, uv{ uv }, color{ color }, tangent{ tangent }
	{
	}

	Vector3 position;
	Vector3 normal;
	Vector2 uv;
	Vector4 color; //TODO: Color struct
	Vector4 tangent;
};

struct NH_API Transform3D
{
public:
	Transform3D* parent;

private:
	bool dirty;
	Vector3 position;
	Quaternion3D rotation;
	Vector3 scale;
	Matrix4 local;

public:
	Transform3D() : parent{ nullptr }, dirty{ false }, position{}, rotation{}, scale{}, local{} {}
	Transform3D(const Vector3& position) : parent{ nullptr }, dirty{ false }, position{ position }, rotation{}, scale{} { UpdateLocal(); }
	Transform3D(const Vector3& position, const Quaternion3D& rotation) :
		parent{ nullptr }, dirty{ false }, position{ position }, rotation{ rotation }, scale{} { UpdateLocal(); }
	Transform3D(const Vector3& position, const Quaternion3D& rotation, const Vector3& scale) :
		parent{ nullptr }, dirty{ false }, position{ position }, rotation{ rotation }, scale{ scale } { UpdateLocal(); }
	Transform3D(const Transform3D& other) : parent{ other.parent }, dirty{ other.dirty }, position{ other.position }, rotation{ other.rotation }, scale{ other.scale } {}
	Transform3D(Transform3D&& other) noexcept : parent{ other.parent }, dirty{ other.dirty }, position{ other.position }, rotation{ other.rotation }, scale{ other.scale } {}

	void* operator new(U64 size);
	void operator delete(void* ptr);

	Transform3D& operator=(const Transform3D& other) { parent = other.parent; dirty = other.dirty; position = other.position; rotation = other.rotation; scale = other.scale; return *this; }
	Transform3D& operator=(Transform3D&& other) noexcept { parent = other.parent; dirty = other.dirty; position = other.position; rotation = other.rotation; scale = other.scale; return *this; }

	const Vector3& Position() const { return position; }
	void SetPosition(const Vector3& v) { position = v; dirty = true; }
	const Quaternion3D& Rotation() const { return rotation; }
	void SetRotation(const Quaternion3D& q) { rotation = q; dirty = true; }
	const Vector3& Scale() const { return scale; }
	void SetScale(const Vector3& v) { scale = v; dirty = true; }

	const Matrix4& Local()
	{
		if (dirty) { UpdateLocal(); }
		return local;
	}

	//TODO: Reduce copying?
	NH_INLINE Matrix4 World()
	{
		if (parent) { return Local() * parent->Local(); }
		return Local();
	}

private:
	void UpdateLocal()
	{
		local = Move(Matrix4(position, rotation, scale));
		dirty = false;
	}
};

struct NH_API Transform2D
{
public:
	Transform2D* parent;

private:
	bool dirty;
	Vector2 position;
	Quaternion2D rotation;
	Vector2 scale;
	Matrix3 local;

public:
	Transform2D() : parent{ nullptr }, dirty{ false }, position{ Vector2::ZERO }, rotation{ 0.0f }, scale{ Vector2::ONE }, local{ position, rotation, scale } {}
	Transform2D(const Vector2& position) : parent{ nullptr }, dirty{ false }, position{ position }, rotation{}, scale{}, local{ position, rotation, scale } {}
	Transform2D(const Vector2& position, const F32& rotation) :
		parent{ nullptr }, dirty{ false }, position{ position }, rotation{ rotation }, scale{}, local{ position, rotation, scale } {}
	Transform2D(const Vector2& position, const F32& rotation, const Vector2& scale) :
		parent{ nullptr }, dirty{ false }, position{ position }, rotation{ rotation }, scale{ scale }, local{ position, rotation, scale } {}
	Transform2D(const Transform2D& other) : parent{ other.parent }, dirty{ other.dirty }, position{ other.position }, rotation{ other.rotation }, scale{ other.scale }, local{ other.local } {}
	Transform2D(Transform2D&& other) noexcept : parent{ other.parent }, dirty{ other.dirty }, position{ other.position }, rotation{ other.rotation }, scale{ other.scale }, local{ other.local } {}

	void* operator new(U64 size);
	void operator delete(void* ptr);

	Transform2D& operator=(const Transform2D& other) { parent = other.parent; dirty = other.dirty; position = other.position; rotation = other.rotation; scale = other.scale; return *this; }
	Transform2D& operator=(Transform2D&& other) noexcept { parent = other.parent; dirty = other.dirty; position = other.position; rotation = other.rotation; scale = other.scale; return *this; }

	const Vector2& Position() const { return position; }
	void SetPosition(const Vector2& v) { position = v; dirty = true; }
	void Translate(const Vector2& v) { position += v; dirty = true; }
	const Quaternion2D& Rotation() const { return rotation; }
	void SetRotation(const F32& angle) { rotation = angle; dirty = true; }
	void Rotate(const F32& angle) { rotation = rotation.angle + angle; dirty = true; }
	const Vector2& Scale() const { return scale; }
	void SetScale(const Vector2& v) { scale = v; dirty = true; }

	const Matrix3& Local()
	{
		if (dirty) { UpdateLocal(); }
		return local;
	}

	//TODO: Reduce copying?
	NH_INLINE Matrix3 World()
	{
		if (parent) { return parent->Local() * Local(); }
		return Local();
	}

private:
	void UpdateLocal()
	{
		local = Move(Matrix3(position, rotation, scale));
		dirty = false;
	}
};

enum ColorType
{
	COLOR_TYPE_RGB,
	COLOR_TYPE_HSV
};

struct Color
{
	ColorType currentType;

	union { F32 r, h; };
	union { F32 g, s; };
	union { F32 b, v; };
	F32 a;

	void ToHSV()
	{
		switch (currentType)
		{
		case COLOR_TYPE_RGB: {



		} break;
		case COLOR_TYPE_HSV:
		default: break;
		}
	}

	void ToRGB()
	{
		switch (currentType)
		{
		case COLOR_TYPE_HSV: {



		} break;
		case COLOR_TYPE_RGB:
		default: break;
		}
	}
};