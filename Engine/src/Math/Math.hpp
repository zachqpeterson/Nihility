#pragma once

#include "Defines.hpp"

#undef INFINITY

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
#define INFINITY 1e30
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
struct Quaternion;

class Math
{
public:
    //TRIGONOMETRY
    static NH_API NH_INLINE F32 Sin(F32 f);
    static NH_API NH_INLINE F64 Sin(F64 f);
    static NH_API NH_INLINE F32 Cos(F32 f);
    static NH_API NH_INLINE F64 Cos(F64 f);
    static NH_API NH_INLINE F32 Tan(F32 f);
    static NH_API NH_INLINE F64 Tan(F64 f);

    static NH_API NH_INLINE F32 Asin(F32 f);
    static NH_API NH_INLINE F64 Asin(F64 f);
    static NH_API NH_INLINE F32 Acos(F32 f);
    static NH_API NH_INLINE F64 Acos(F64 f);
    static NH_API NH_INLINE F32 Atan(F32 f);
    static NH_API NH_INLINE F64 Atan(F64 f);

    static NH_API NH_INLINE F32 Log2(F32 f);
    static NH_API NH_INLINE F64 Log2(F64 f);
    static NH_API NH_INLINE F32 Log10(F32 f);
    static NH_API NH_INLINE F64 Log10(F64 f);
    static NH_API NH_INLINE F32 LogN(F32 f);
    static NH_API NH_INLINE F64 LogN(F64 f);

    static NH_API NH_INLINE F32 DegToRad(F32 d) { return d * DEG2RAD_MULTIPLIER; }
    static NH_API NH_INLINE F64 DegToRad(F64 d) { return d * DEG2RAD_MULTIPLIER; }
    static NH_API NH_INLINE F32 RadToDeg(F32 r) { return r * RAD2DEG_MULTIPLIER; }
    static NH_API NH_INLINE F64 RadToDeg(F64 r) { return r * RAD2DEG_MULTIPLIER; }

    //COMPARISON
    static NH_API NH_INLINE F32 Abs(F32 n) { return n < 0.f ? -n : n; }
    static NH_API NH_INLINE F64 Abs(F64 n) { return n < 0. ? -n : n; }
    static NH_API NH_INLINE I8  Abs(I8  n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE I16 Abs(I16 n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE I32 Abs(I32 n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE I64 Abs(I64 n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE U8  Abs(U8  n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE U16 Abs(U16 n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE U32 Abs(U32 n) { return n < 0 ? -n : n; }
    static NH_API NH_INLINE U64 Abs(U64 n) { return n < 0 ? -n : n; }

    static NH_API NH_INLINE F32 Min(F32 a, F32 b) { return a < b ? a : b; }
    static NH_API NH_INLINE F64 Min(F64 a, F64 b) { return a < b ? a : b; }
    static NH_API NH_INLINE I8  Min(I8  a, I8  b) { return a < b ? a : b; }
    static NH_API NH_INLINE I16 Min(I16 a, I16 b) { return a < b ? a : b; }
    static NH_API NH_INLINE I32 Min(I32 a, I32 b) { return a < b ? a : b; }
    static NH_API NH_INLINE I64 Min(I64 a, I64 b) { return a < b ? a : b; }
    static NH_API NH_INLINE U8  Min(U8  a, U8  b) { return a < b ? a : b; }
    static NH_API NH_INLINE U16 Min(U16 a, U16 b) { return a < b ? a : b; }
    static NH_API NH_INLINE U32 Min(U32 a, U32 b) { return a < b ? a : b; }
    static NH_API NH_INLINE U64 Min(U64 a, U64 b) { return a < b ? a : b; }

    static NH_API NH_INLINE F32 Max(F32 a, F32 b) { return a > b ? a : b; }
    static NH_API NH_INLINE F64 Max(F64 a, F64 b) { return a > b ? a : b; }
    static NH_API NH_INLINE I8  Max(I8  a, I8  b) { return a > b ? a : b; }
    static NH_API NH_INLINE I16 Max(I16 a, I16 b) { return a > b ? a : b; }
    static NH_API NH_INLINE I32 Max(I32 a, I32 b) { return a > b ? a : b; }
    static NH_API NH_INLINE I64 Max(I64 a, I64 b) { return a > b ? a : b; }
    static NH_API NH_INLINE U8  Max(U8  a, U8  b) { return a > b ? a : b; }
    static NH_API NH_INLINE U16 Max(U16 a, U16 b) { return a > b ? a : b; }
    static NH_API NH_INLINE U32 Max(U32 a, U32 b) { return a > b ? a : b; }
    static NH_API NH_INLINE U64 Max(U64 a, U64 b) { return a > b ? a : b; }

    static NH_API NH_INLINE F32 Clamp(F32 n, F32 a, F32 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE F64 Clamp(F64 n, F64 a, F64 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE I8  Clamp(I8  n, I8  a, I8  b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE I16 Clamp(I16 n, I16 a, I16 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE I32 Clamp(I32 n, I32 a, I32 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE I64 Clamp(I64 n, I64 a, I64 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE U8  Clamp(U8  n, U8  a, U8  b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE U16 Clamp(U16 n, U16 a, U16 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE U32 Clamp(U32 n, U32 a, U32 b) { return n < a ? a : n > b ? b : n; }
    static NH_API NH_INLINE U64 Clamp(U64 n, U64 a, U64 b) { return n < a ? a : n > b ? b : n; }

    static NH_API NH_INLINE I32 Floor(F32 n) { return n >= 0 ? (I32)n : (I32)n - 1; }
    static NH_API NH_INLINE I64 Floor(F64 n) { return n >= 0 ? (I64)n : (I64)n - 1; }
    static NH_API NH_INLINE F32 FloorF(F32 n) { return n >= 0 ? (I32)n : (I32)n - 1; }
    static NH_API NH_INLINE F64 FloorF(F64 n) { return n >= 0 ? (I64)n : (I64)n - 1; }

    static NH_API NH_INLINE F32 Ceiling(F32 n) { return n > 0 ? (I32)n + 1 : (I32)n; }
    static NH_API NH_INLINE F64 Ceiling(F64 n) { return n > 0 ? (I64)n + 1 : (I64)n; }

    //FLOATING-POINT
    static NH_API NH_INLINE F32 Round(F32 f) { return (I32)(f + 0.5f); }
    static NH_API NH_INLINE F64 Round(F64 f) { return (I64)(f + 0.5); }
    static NH_API NH_INLINE F32 Mod(F32 f, F32 d) { return f - d * Floor(f / d); }
    static NH_API NH_INLINE F64 Mod(F64 f, F64 d) { return f - d * Floor(f / d); }

    //LINEAR
    static NH_API NH_INLINE F32 Sqrt(F32 f);
    static NH_API NH_INLINE F64 Sqrt(F64 f);
    static NH_API NH_INLINE F32 InvSqrt(F32 f);
    static NH_API NH_INLINE F64 InvSqrt(F64 f);
    static NH_API NH_INLINE Matrix4 Orthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far);
    static NH_API NH_INLINE Matrix4 Perspective(F32 fov, F32 aspect, F32 near, F32 far);

    //INTERPOLATION
    static NH_API NH_INLINE F32 Lerp(F32 a, F32 b, F32 t) { return a + t * (b - a); }
    static NH_API NH_INLINE F64 Lerp(F64 a, F64 b, F64 t) { return a + t * (b - a); }
    static NH_API NH_INLINE Vector2 Lerp(Vector2 a, Vector2 b, F32 t);
    static NH_API NH_INLINE Vector3 Lerp(Vector3 a, Vector3 b, F32 t);
    static NH_API NH_INLINE Vector4 Lerp(Vector4 a, Vector4 b, F32 t);
    static NH_API NH_INLINE F32 InvLerp(F32 a, F32 b, F32 t) { return (t - a) / (b - a); }
    static NH_API NH_INLINE F64 InvLerp(F64 a, F64 b, F64 t) { return (t - a) / (b - a); }
    static NH_API NH_INLINE F32 MoveTowards(F32 a, F32 b, F32 t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }
    static NH_API NH_INLINE F64 MoveTowards(F64 a, F64 b, F64 t) { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }

    //NOISE
    static NH_API NH_INLINE F64 Simplex1(F64 x);
    static NH_API NH_INLINE F64 Simplex2(F64 x, F64 y);
    static NH_API NH_INLINE F64 Simplex3(F64 x, F64 y, F64 z);
private:
    static NH_INLINE F64 Dot(I8 g[2], F64 x, F64 y);
    static NH_INLINE F64 Dot(I8 g[3], F64 x, F64 y, F64 z);
    static NH_INLINE F64 Grad(I64 hash, F64 x);
public:

    //RANDOM
    static NH_API NH_INLINE I64 Random(U32 seed = 0);
    static NH_API NH_INLINE I64 RandomRange(I64 min, I64 max, U32 seed = 0);
    static NH_API NH_INLINE F32 RandomF(U32 seed = 0);
    static NH_API NH_INLINE F32 RandomRangeF(F32 min, F32 max, U32 seed = 0);

    //OTHER
    static NH_API NH_INLINE bool PowerOfTwo(U8  n) { return (n != 0) && ((n & (n - 1)) == 0); }
    static NH_API NH_INLINE bool PowerOfTwo(U16 n) { return (n != 0) && ((n & (n - 1)) == 0); }
    static NH_API NH_INLINE bool PowerOfTwo(U32 n) { return (n != 0) && ((n & (n - 1)) == 0); }
    static NH_API NH_INLINE bool PowerOfTwo(U64 n) { return (n != 0) && ((n & (n - 1)) == 0); }

private:
    Math() = delete;
};

struct Vector2
{
    F32 x, y;

    NH_API Vector2() : x { 0.0f }, y{ 0.0f } {}
    NH_API Vector2(F32 f) : x{ f }, y{ f } {}
    NH_API Vector2(F32 x, F32 y) : x{ x }, y{ y } {}
    NH_API Vector2(const Vector2& v) : x{ v.x }, y{ v.y } {}
    NH_API Vector2(Vector2&& v) : x{ v.x }, y{ v.y } {}

    NH_API Vector2& operator=(const Vector2& v) { x = v.x; y = v.y; return *this; }
    NH_API Vector2& operator=(Vector2&& v) { x = v.x; y = v.y; return *this; }

    NH_API Vector2& operator+= (const Vector2& v) { x += v.x; y += v.y;	            return *this; }
    NH_API Vector2& operator-= (const Vector2& v) { x -= v.x; y -= v.y;	            return *this; }
    NH_API Vector2& operator*= (F32 f) { x *= f; y *= f;					        return *this; }
    NH_API Vector2& operator/= (F32 f) { x /= f; y /= f;					        return *this; }
    NH_API Vector2& operator%= (F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f);  return *this; }

    NH_API Vector2 operator+ (const Vector2& v) const { return { x + v.x, y + v.y }; }
    NH_API Vector2 operator- (const Vector2& v) const { return { x - v.x, y - v.y }; }
    NH_API Vector2 operator* (F32 f) const { return { x * f, y * f }; }
    NH_API Vector2 operator/ (F32 f) const { return { x / f, y / f }; }
    NH_API Vector2 operator% (F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f) }; }

    NH_API bool operator== (const Vector2& v) const { return x == v.x && y == v.y; }
    NH_API bool operator!= (const Vector2& v) const { return x != v.x || y != v.y; }
    NH_API bool operator<  (const Vector2& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
    NH_API bool operator>  (const Vector2& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
    friend NH_API Vector2 operator- (Vector2& v) { return { -v.x, -v.y }; }

    NH_API NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

    NH_API NH_INLINE F32 Dot(const Vector2& v) const { return v.x * x + v.y * y; }
    NH_API NH_INLINE F32 Magnitude() const { return Math::Sqrt(Dot(*this)); }
    NH_API NH_INLINE F32 SqrMagnitude() const { return Dot(*this); }
    NH_API NH_INLINE void Normalize() { Vector2 v = Normalized(); x = v.x; y = v.y; }
    NH_API NH_INLINE Vector2 Normalized() const { return (*this) / Magnitude(); }
    NH_API NH_INLINE F32 AngleBetween(const Vector2& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
    NH_API NH_INLINE Vector2 Projection(const Vector2& v) const { return v * (Dot(v) / v.Dot(v)); }
    NH_API NH_INLINE Vector2 OrthoProjection(const Vector2& v) const { return *this - Projection(v); }
    NH_API NH_INLINE void Rotate(const Vector2& center, F32 angle)
    {
        F32 cos = Math::Cos(angle);
        F32 sin = Math::Sin(angle);
        F32 temp = cos * (x - center.x) - sin * (y - center.y) + center.x;
        y = sin * (x - center.x) + cos * (y - center.y) + center.y;
        x = temp;
    }
    NH_API NH_INLINE Vector2 Rotated(const Vector2& center, F32 angle) const
    {
        F32 cos = Math::Cos(angle);
        F32 sin = Math::Sin(angle);
        return { cos * (x - center.x) - sin * (y - center.y) + center.x,
        sin * (x - center.x) + cos * (y - center.y) + center.y };
    }
    NH_API NH_INLINE Vector2& Clamp(Vector2 xBound, Vector2 yBound)
    {
        x = Math::Clamp(x, xBound.x, xBound.y);
        y = Math::Clamp(y, yBound.x, yBound.y);
        return *this;
    }

    static const Vector2 ONE;
    static const Vector2 ZERO;
    static const Vector2 RIGHT;
    static const Vector2 LEFT;
    static const Vector2 UP;
    static const Vector2 DOWN;
};

struct Vector3
{
    F32 x, y, z;

    NH_API Vector3() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f } {}
    NH_API Vector3(F32 f) : x{ f }, y{ f }, z{ f } {}
    NH_API Vector3(F32 x, F32 y, F32 z) : x{ x }, y{ y }, z{ z } {}
    NH_API Vector3(const Vector3& v) : x{ v.x }, y{ v.y }, z{ v.z } {}
    NH_API Vector3(Vector3&& v) : x{ v.x }, y{ v.y }, z{ v.z } {}

    NH_API Vector3& operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z; return *this; }
    NH_API Vector3& operator=(Vector3&& v) { x = v.x; y = v.y; z = v.z; return *this; }

    NH_API Vector3& operator+= (const Vector3& v) { x += v.x; y += v.y; z += v.z;   return *this; }
    NH_API Vector3& operator-= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z;   return *this; }
    NH_API Vector3& operator*= (F32 f) { x *= f; y *= f; z *= f;					return *this; }
    NH_API Vector3& operator/= (F32 f) { x /= f; y /= f; z /= f;					return *this; }
    NH_API Vector3& operator%= (F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f);  return *this; }

    NH_API Vector3 operator+ (const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    NH_API Vector3 operator- (const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    NH_API Vector3 operator* (F32 f) const { return { x * f, y * f, z * f }; }
    NH_API Vector3 operator/ (F32 f) const { return { x / f, y / f, z / f }; }
    NH_API Vector3 operator% (F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f) }; }

    NH_API bool operator== (const Vector3& v) const { return x == v.x && y == v.y && z == v.z; }
    NH_API bool operator!= (const Vector3& v) const { return x != v.x || y != v.y || z != v.z; }
    NH_API bool operator<  (const Vector3& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
    NH_API bool operator>  (const Vector3& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
    friend NH_API Vector3 operator- (Vector3& v) { return { -v.x, -v.y, -v.z }; }

    NH_API NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

    NH_API NH_INLINE F32 Dot(const Vector3& v) const { return v.x * x + v.y * y + v.z * z; }
    NH_API NH_INLINE Vector3 Cross(const Vector3& v) const { return { y * v.z - v.y * z, z * v.x - v.z * x, x * v.y - v.x * y }; }
    NH_API NH_INLINE F32 Magnitude() const { return Math::Sqrt(Dot(*this)); }
    NH_API NH_INLINE F32 SqrMagnitude() const { return Dot(*this); }
    NH_API NH_INLINE void Normalize() { Vector3 v = Normalized(); x = v.x; y = v.y; z = v.z; }
    NH_API NH_INLINE Vector3 Normalized() const { return *this / Magnitude(); }
    NH_API NH_INLINE F32 AngleBetween(const Vector3& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
    NH_API NH_INLINE Vector3 Projection(const Vector3& v) const { return v * (Dot(v) / v.Dot(v)); }
    NH_API NH_INLINE Vector3 OrthoProjection(const Vector3& v) const { return *this - Projection(v); }
    NH_API NH_INLINE void Rotate(const Vector3& center, F32 angle)
    {
        F32 cos = Math::Cos(angle);
        Vector3 v = (*this * cos) + (center.Cross(*this) * Math::Sin(angle)) + (center * Dot(center)) * (1 - cos);
        x = v.x; y = v.y; z = v.z;
    }
    NH_API NH_INLINE Vector3 Rotated(const Vector3& center, F32 angle) const
    {
        F32 cos = Math::Cos(angle);
        return (*this * cos) + (center.Cross(*this) * Math::Sin(angle)) + (center * Dot(center)) * (1 - cos);
    }
    NH_API NH_INLINE Vector3& Clamp(Vector2 xBound, Vector2 yBound, Vector2 zBound)
    {
        x = Math::Clamp(x, xBound.x, xBound.y);
        y = Math::Clamp(y, yBound.x, yBound.y);
        z = Math::Clamp(z, zBound.x, zBound.y);
        return *this;
    }

    static const Vector3 ONE;
    static const Vector3 ZERO;
    static const Vector3 RIGHT;
    static const Vector3 LEFT;
    static const Vector3 UP;
    static const Vector3 DOWN;
    static const Vector3 FORWARD;
    static const Vector3 BACK;
};

struct Vector4
{
    F32 x, y, z, w;

    NH_API Vector4() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 0.0f } {}
    NH_API Vector4(F32 f) : x{ f }, y{ f }, z{ f }, w{ f } {}
    NH_API Vector4(F32 x, F32 y, F32 z, F32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
    NH_API Vector4(const Vector4& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
    NH_API Vector4(Vector4&& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}

    NH_API Vector4& operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
    NH_API Vector4& operator=(Vector4&& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

    NH_API Vector4& operator+= (const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    NH_API Vector4& operator-= (const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    NH_API Vector4& operator*= (F32 f) { x *= f; y *= f; z *= f; w *= f;					return *this; }
    NH_API Vector4& operator/= (F32 f) { x /= f; y /= f; z /= f; w /= f;					return *this; }
    NH_API Vector4& operator%= (F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); w = Math::Mod(w, f);  return *this; }

    NH_API Vector4 operator+ (const Vector4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
    NH_API Vector4 operator- (const Vector4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
    NH_API Vector4 operator* (F32 f) const { return { x * f, y * f, z * f, w * f }; }
    NH_API Vector4 operator/ (F32 f) const { return { x / f, y / f, z / f, w / f }; }
    NH_API Vector4 operator% (F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f), Math::Mod(w, f) }; }

    NH_API bool operator== (const Vector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    NH_API bool operator!= (const Vector4& v) const { return x != v.x || y != v.y || z != v.z || w != v.w; }
    NH_API bool operator<  (const Vector4& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
    NH_API bool operator>  (const Vector4& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
    friend NH_API Vector4 operator- (Vector4& v) { return { -v.x, -v.y, -v.z, -v.w }; }

    NH_API NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }

    NH_API NH_INLINE F32 Dot(const Vector4& v) const { return v.x * x + v.y * y + v.z * z + v.w * w; }
    NH_API NH_INLINE F32 Magnitude() const { return Math::Sqrt(Dot(*this)); }
    NH_API NH_INLINE F32 SqrMagnitude() const { return Dot(*this); }
    NH_API NH_INLINE void Normalize() { Vector4 v = Normalized(); x = v.x; y = v.y; z = v.z; w = v.w; }
    NH_API NH_INLINE Vector4 Normalized() const { return *this / Magnitude(); }
    NH_API NH_INLINE F32 AngleBetween(const Vector4& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
    NH_API NH_INLINE Vector4 Projection(const Vector4& v) const { return v * (Dot(v) / v.Dot(v)); }
    NH_API NH_INLINE Vector4 OrthoProjection(const Vector4& v) const { return *this - Projection(v); }
    //TODO:
    //NH_API void Rotate(const Vector4& center, F32 angle)
    //{
    //  
    //}
    //NH_API Vector4 Rotated(const Vector4& center, F32 angle) const
    //{
    //  
    //}
    NH_API NH_INLINE Vector4& Clamp(Vector2 xBound, Vector2 yBound, Vector2 zBound, Vector2 wBound)
    {
        x = Math::Clamp(x, xBound.x, xBound.y);
        y = Math::Clamp(y, yBound.x, yBound.y);
        z = Math::Clamp(z, zBound.x, zBound.y);
        w = Math::Clamp(w, wBound.x, wBound.y);
        return *this;
    }

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

struct Vector2Int
{
    I32 x, y;

    NH_API Vector2Int() : x{ 0 }, y{ 0 } {}
    NH_API Vector2Int(I32 i) : x{ i }, y{ i } {}
    NH_API Vector2Int(I32 x, I32 y) : x{ x }, y{ y } {}
    NH_API Vector2Int(const Vector2Int& v) : x{ v.x }, y{ v.y } {}
    NH_API Vector2Int(Vector2Int&& v) : x{ v.x }, y{ v.y } {}

    NH_API Vector2Int& operator=  (const Vector2Int& v) { x = v.x; y = v.y;	return *this; }
    NH_API Vector2Int& operator=  (Vector2Int&& v) { x = v.x; y = v.y; return *this; }

    NH_API Vector2Int& operator+= (const Vector2Int& v) { x += v.x; y += v.y;	return *this; }
    NH_API Vector2Int& operator-= (const Vector2Int& v) { x -= v.x; y -= v.y;	return *this; }
    NH_API Vector2Int& operator*= (I32 i) { x *= i; y *= i;					return *this; }
    NH_API Vector2Int& operator/= (I32 i) { x /= i; y /= i;					return *this; }
    NH_API Vector2Int& operator%= (I32 i) { x %= i; y %= i;					return *this; }

    NH_API Vector2Int operator+ (const Vector2Int& v) const { return { x + v.x, y + v.y }; }
    NH_API Vector2Int operator- (const Vector2Int& v) const { return { x - v.x, y - v.y }; }
    NH_API Vector2Int operator* (I32 i) const { return { x * i, y * i }; }
    NH_API Vector2Int operator/ (I32 i) const { return { x / i, y / i }; }
    NH_API Vector2Int operator% (I32 i) const { return { x % i, y % i }; }

    NH_API bool operator== (const Vector2Int& v) const { return x == v.x && y == v.y; }
    NH_API bool operator!= (const Vector2Int& v) const { return x != v.x || y != v.y; }
    NH_API bool operator<  (const Vector2Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
    NH_API bool operator>  (const Vector2Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
    friend NH_API Vector2Int operator- (Vector2Int& v) { return { -v.x, -v.y }; }

    NH_API NH_INLINE const I32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE I32& operator[] (U8 i) { return ((&x)[i]); }

    NH_API NH_INLINE I32 Dot(const Vector2Int& v) const { return v.x * x + v.y * y; }
    NH_API NH_INLINE F32 Magnitude() const { return Math::Sqrt((F32)Dot(*this)); }
    NH_API NH_INLINE F32 SqrMagnitude() const { return (F32)Dot(*this); }
    NH_API NH_INLINE Vector2Int& Clamp(Vector2Int xBound, Vector2Int yBound)
    {
        x = Math::Clamp(x, xBound.x, xBound.y);
        y = Math::Clamp(y, yBound.x, yBound.y);
        return *this;
    }

    static const Vector2Int ONE;
    static const Vector2Int ZERO;
    static const Vector2Int RIGHT;
    static const Vector2Int LEFT;
    static const Vector2Int UP;
    static const Vector2Int DOWN;
};

struct Vector3Int
{
    I32 x, y, z;

    NH_API Vector3Int() : x{ 0 }, y{ 0 }, z{ 0 } {}
    NH_API Vector3Int(I32 i) : x{ i }, y{ i }, z{ i } {}
    NH_API Vector3Int(I32 x, I32 y, I32 z) : x{ x }, y{ y }, z{ z } {}
    NH_API Vector3Int(const Vector3Int& v) : x{ v.x }, y{ v.y }, z{ v.z } {}
    NH_API Vector3Int(Vector3Int&& v) : x{ v.x }, y{ v.y }, z{ v.z } {}

    NH_API Vector3Int& operator=  (const Vector3Int& v) { x = v.x; y = v.y; z = v.z;	return *this; }
    NH_API Vector3Int& operator=  (Vector3Int&& v) { x = v.x; y = v.y; z = v.z;         return *this; }

    NH_API Vector3Int& operator+= (const Vector3Int& v) { x += v.x; y += v.y; z += v.z;	return *this; }
    NH_API Vector3Int& operator-= (const Vector3Int& v) { x -= v.x; y -= v.y; z -= v.z;	return *this; }
    NH_API Vector3Int& operator*= (I32 i) { x *= i; y *= i; z *= i;					    return *this; }
    NH_API Vector3Int& operator/= (I32 i) { x /= i; y /= i; z /= i;					    return *this; }
    NH_API Vector3Int& operator%= (I32 i) { x %= i; y %= i;	z %= i;				        return *this; }

    NH_API Vector3Int operator+ (const Vector3Int& v) const { return { x + v.x, y + v.y, z + v.z }; }
    NH_API Vector3Int operator- (const Vector3Int& v) const { return { x - v.x, y - v.y, z - v.z }; }
    NH_API Vector3Int operator* (I32 i) const { return { x * i, y * i, z * i }; }
    NH_API Vector3Int operator/ (I32 i) const { return { x / i, y / i, z / i }; }
    NH_API Vector3Int operator% (I32 i) const { return { x % i, y % i, z % i }; }

    NH_API bool operator== (const Vector3Int& v) const { return x == v.x && y == v.y && z == v.z; }
    NH_API bool operator!= (const Vector3Int& v) const { return x != v.x || y != v.y || z != v.z; }
    NH_API bool operator<  (const Vector3Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
    NH_API bool operator>  (const Vector3Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
    friend NH_API Vector3Int operator- (Vector3Int& v) { return { -v.x, -v.y, -v.z }; }

    NH_API NH_INLINE const I32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE I32& operator[] (U8 i) { return ((&x)[i]); }

    NH_API NH_INLINE I32 Dot(const Vector3Int& v) const { return v.x * x + v.y * y + v.z * z; }
    NH_API NH_INLINE F32 Magnitude() const { return Math::Sqrt((F32)Dot(*this)); }
    NH_API NH_INLINE F32 SqrMagnitude() const { return (F32)Dot(*this); }
    NH_API NH_INLINE Vector3Int& Clamp(Vector2Int xBound, Vector2Int yBound, Vector2Int zBound)
    {
        x = Math::Clamp(x, xBound.x, xBound.y);
        y = Math::Clamp(y, yBound.x, yBound.y);
        z = Math::Clamp(z, zBound.x, zBound.y);
        return *this;
    }

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

    NH_API Vector4Int() : x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 } {}
    NH_API Vector4Int(I32 i) : x{ i }, y{ i }, z{ i }, w{ i } {}
    NH_API Vector4Int(I32 x, I32 y, I32 z, I32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
    NH_API Vector4Int(const Vector4Int& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
    NH_API Vector4Int(Vector4Int&& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}

    NH_API Vector4Int& operator=  (const Vector4Int& v) { x = v.x; y = v.y; z = v.z; w = v.w;	return *this; }
    NH_API Vector4Int& operator=  (Vector4Int&& v) { x = v.x; y = v.y; z = v.z; w = v.w;        return *this; }

    NH_API Vector4Int& operator+= (const Vector4Int& v) { x += v.x; y += v.y; z += v.z; w += v.w;	return *this; }
    NH_API Vector4Int& operator-= (const Vector4Int& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w;	return *this; }
    NH_API Vector4Int& operator*= (I32 i) { x *= i; y *= i; z *= i;	w *= i;				    return *this; }
    NH_API Vector4Int& operator/= (I32 i) { x /= i; y /= i; z /= i;	w /= i;				    return *this; }
    NH_API Vector4Int& operator%= (I32 i) { x %= i; y %= i;	z %= i;	w %= i;			        return *this; }

    NH_API Vector4Int operator+ (const Vector4Int& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
    NH_API Vector4Int operator- (const Vector4Int& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
    NH_API Vector4Int operator* (I32 i) const { return { x * i, y * i, z * i, w * i }; }
    NH_API Vector4Int operator/ (I32 i) const { return { x / i, y / i, z / i, w / i }; }
    NH_API Vector4Int operator% (I32 i) const { return { x % i, y % i, z % i, w % i }; }

    NH_API bool operator== (const Vector4Int& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    NH_API bool operator!= (const Vector4Int& v) const { return x != v.x || y != v.y || z != v.z || w != v.w; }
    NH_API bool operator<  (const Vector4Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
    NH_API bool operator>  (const Vector4Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
    friend NH_API Vector4Int operator- (Vector4Int& v) { return { -v.x, -v.y, -v.z, -v.w }; }

    NH_API NH_INLINE const I32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE I32& operator[] (U8 i) { return ((&x)[i]); }

    NH_API NH_INLINE I32 Dot(const Vector4Int& v) const { return v.x * x + v.y * y + v.z * z + v.w * w; }
    NH_API NH_INLINE F32 Magnitude() const { return Math::Sqrt((F32)Dot(*this)); }
    NH_API NH_INLINE F32 SqrMagnitude() const { return (F32)Dot(*this); }
    NH_API NH_INLINE Vector4Int& Clamp(Vector2Int xBound, Vector2Int yBound, Vector2Int zBound, Vector2Int wBound)
    {
        x = Math::Clamp(x, xBound.x, xBound.y);
        y = Math::Clamp(y, yBound.x, yBound.y);
        z = Math::Clamp(z, zBound.x, zBound.y);
        w = Math::Clamp(w, wBound.x, wBound.y);
        return *this;
    }

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

struct Matrix2
{
    Vector2 a, b;

    NH_API Matrix2() : a{}, b{} {}
    NH_API Matrix2(F32 n) : a{ n, 0.0f }, b{ 0.0f, n } {}
    NH_API Matrix2(F32 ax, F32 ay, F32 bx, F32 by) : a{ ax, ay }, b{ bx, by } {}
    NH_API Matrix2(const Vector2& v1, const Vector2& v2) : a{ v1 }, b{ v2 } {}
    NH_API Matrix2(Vector2&& v1, Vector2&& v2) : a{ v1 }, b{ v2 } {}
    NH_API Matrix2(const Matrix2& m) : a{ m.a }, b{ m.b } {}
    NH_API Matrix2(Matrix2&& m) : a{ m.a }, b{ m.b } {}

    NH_API Matrix2& operator= (const Matrix2& m) { a = m.a; b = m.b; return *this; }
    NH_API Matrix2& operator= (Matrix2&& m) { a = m.a; b = m.b; return *this; }

    NH_API Matrix2& operator+= (const Matrix2& m) { a += m.a; b += m.b; return *this; }
    NH_API Matrix2& operator-= (const Matrix2& m) { a -= m.a; b -= m.b; return *this; }
    NH_API Matrix2& operator*= (const Matrix2& m)
    {
        a[0] = a[0] * m[0][0] + b[0] * m[0][1];
        b[0] = a[0] * m[1][0] + b[0] * m[1][1];

        a[1] = a[1] * m[0][0] + b[1] * m[0][1];
        b[1] = a[1] * m[1][0] + b[1] * m[1][1];

        return *this;
    }

    NH_API Matrix2 operator+(const Matrix2& m) const { return { a + m.a, b + m.b }; }
    NH_API Matrix2 operator-(const Matrix2& m) const { return { a - m.a, b - m.b }; }
    NH_API Matrix2 operator*(const Matrix2& m) const
    {
        Matrix2 r;
        r[0][0] = a[0] * m[0][0] + b[0] * m[0][1];
        r[1][0] = a[0] * m[1][0] + b[0] * m[1][1];

        r[0][1] = a[1] * m[0][0] + b[1] * m[0][1];
        r[1][1] = a[1] * m[1][0] + b[1] * m[1][1];

        return r;
    }
    NH_API Vector2 operator*(const Vector2& v) const
    {
        Vector2 r;
        r[0] = a[0] * v[0] + b[0] * v[1];
        r[1] = a[1] * v[0] + b[1] * v[1];

        return r;
    }

    friend NH_API Matrix2 operator- (Matrix2& m) { return { -m.a, -m.b }; }
    NH_API bool operator==(const Matrix2& m) const { return a == m.a && b == m.b; }
    NH_API bool operator!=(const Matrix2& m) const { return a != m.a || b != m.b; }

    NH_API NH_INLINE const Vector2& operator[] (U8 i) const { return ((&a)[i]); }
    NH_API NH_INLINE Vector2& operator[] (U8 i) { return ((&a)[i]); }
};

struct Matrix3
{
    Vector3 a, b, c;

    NH_API NH_INLINE const Vector3& operator[] (U8 i) const { return ((&a)[i]); }
    NH_API NH_INLINE Vector3& operator[] (U8 i) { return ((&a)[i]); }
};

struct Matrix4
{
    Vector4 a, b, c, d;

    NH_API Matrix4() : a{ 0.f, 0.f, 0.f, 0.f }, b{ 0.f, 0.f, 0.f, 0.f }, c{ 0.f, 0.f, 0.f, 0.f }, d{ 0.f, 0.f, 0.f, 0.f } {}
    NH_API Matrix4(Vector4 a, Vector4 b, Vector4 c, Vector4 d) : a{ a }, b{ b }, c{ c }, d{ d } {}

    NH_API NH_INLINE const Vector4& operator[] (U8 i) const { return ((&a)[i]); }
    NH_API NH_INLINE Vector4& operator[] (U8 i) { return ((&a)[i]); }

    static const Matrix4 ONE;
};

struct Quaternion
{
    F32 x, y, z, w;

    NH_API Quaternion() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 1.0f } {}
    NH_API Quaternion(F32 x, F32 y, F32 z, F32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
    NH_API Quaternion(const Quaternion& q) : x{ q.x }, y{ q.y }, z{ q.z }, w{ q.w } {}
    NH_API Quaternion(Quaternion&& q) : x{ q.x }, y{ q.y }, z{ q.z }, w{ q.w } {}

    NH_API Quaternion& operator=(const Quaternion& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
    NH_API Quaternion& operator=(Quaternion&& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }

    static NH_API NH_INLINE Quaternion AxisAngle(Vector3 axis, F32 angle, bool normalize);

    NH_API Quaternion operator* (const Quaternion& q) const
    {
        return { x * q.w + y * q.z - z * q.y + w * q.x,
                -x * q.z + y * q.w + z * q.x + w * q.y,
                x * q.y - y * q.x + z * q.w + w * q.z,
                -x * q.x - y * q.y - z * q.z + w * q.w };
    }

    NH_API NH_INLINE Matrix4 ToMatrix4() const;
    NH_API NH_INLINE Matrix4 ToRotationMat(Vector3 center) const;
    NH_API NH_INLINE Quaternion Slerp(const Quaternion& q, F32 t) const;

    NH_API NH_INLINE F32 Dot(Quaternion q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
    NH_API NH_INLINE F32 Normal() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
    NH_API NH_INLINE Quaternion Normalized() const { F32 normal = 1.0f / Normal(); return { x * normal, y * normal, z * normal, w * normal }; }
    NH_API NH_INLINE void Normalize() { F32 normal = 1.0f / Normal(); x *= normal; y *= normal; z *= normal; w *= normal; }
    NH_API NH_INLINE Quaternion Conjugate() const { return { -x, -y, -z, w }; }
    NH_API NH_INLINE Quaternion Inverse() const { return Conjugate().Normalized(); }

    NH_API NH_INLINE const F32& operator[] (U8 i) const { return ((&x)[i]); }
    NH_API NH_INLINE F32& operator[] (U8 i) { return ((&x)[i]); }
};

struct Vertex3
{
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
    Vector4 color;
    Vector4 tangent;
};

struct Vertex2
{
    //TODO: add normal?
    Vector2 position;
    Vector2 uv;
    Vector4 color;
};

struct Transform
{
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    
    bool dirty;
    Matrix4 local;

    struct Transform* parent;
};

NH_INLINE Vector2 Math::Lerp(Vector2 a, Vector2 b, F32 t) { return a + (b - a) * t; }
NH_INLINE Vector3 Math::Lerp(Vector3 a, Vector3 b, F32 t) { return a + (b - a) * t; }
NH_INLINE Vector4 Math::Lerp(Vector4 a, Vector4 b, F32 t) { return a + (b - a) * t; }