module;

#include "Defines.hpp"

export module Math:Types;

import :Functions;
import :Constants;

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

export struct NH_API Vector2
{
	constexpr Vector2() : x(0.0f), y(0.0f) {}
	constexpr Vector2(F32 f) : x(f), y(f) {}
	constexpr Vector2(F32 x, F32 y) : x(x), y(y) {}
	constexpr Vector2(const Vector2& v) : x(v.x), y(v.y) {}
	constexpr Vector2(Vector2&& v) noexcept : x(v.x), y(v.y) {}

	constexpr Vector2& operator=(F32 f) { x = f; y = f; return *this; }
	constexpr Vector2& operator=(const Vector2& v) { x = v.x; y = v.y; return *this; }
	constexpr Vector2& operator=(Vector2&& v) noexcept { x = v.x; y = v.y; return *this; }

	constexpr Vector2& operator+=(F32 f) { x += f; y += f; return *this; }
	constexpr Vector2& operator-=(F32 f) { x -= f; y -= f; return *this; }
	constexpr Vector2& operator*=(F32 f) { x *= f; y *= f; return *this; }
	constexpr Vector2& operator/=(F32 f) { x /= f; y /= f; return *this; }
	constexpr Vector2& operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); return *this; }
	constexpr Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
	constexpr Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
	constexpr Vector2& operator*=(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
	constexpr Vector2& operator/=(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
	constexpr Vector2& operator%=(const Vector2& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); return *this; }
	constexpr Vector2& operator*=(const Quaternion2& q);

	constexpr Vector2 operator+(F32 f) const { return { x + f, y + f }; }
	constexpr Vector2 operator-(F32 f) const { return { x - f, y - f }; }
	constexpr Vector2 operator*(F32 f) const { return { x * f, y * f }; }
	constexpr Vector2 operator/(F32 f) const { return { x / f, y / f }; }
	constexpr Vector2 operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f) }; }
	constexpr Vector2 operator+(const Vector2& v) const { return { x + v.x, y + v.y }; }
	constexpr Vector2 operator-(const Vector2& v) const { return { x - v.x, y - v.y }; }
	constexpr Vector2 operator*(const Vector2& v) const { return { x * v.x, y * v.y }; }
	constexpr Vector2 operator/(const Vector2& v) const { return { x / v.x, y / v.y }; }
	constexpr Vector2 operator%(const Vector2& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y) }; }
	constexpr Vector2 operator*(const Quaternion2& q) const;
	constexpr Vector2 operator^(const Quaternion2& q) const;

	constexpr bool operator==(const Vector2& v) const { return Math::IsZero(x - v.x) && Math::IsZero(y - v.y); }
	constexpr bool operator!=(const Vector2& v) const { return !Math::IsZero(x - v.x) || !Math::IsZero(y - v.y); }
	constexpr bool operator>(const Vector2& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<(const Vector2& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool operator>=(const Vector2& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<=(const Vector2& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool IsZero() const { return Math::IsZero(x) && Math::IsZero(y); }

	constexpr Vector2 operator-() const { return { -x, -y }; }
	constexpr Vector2 operator~() const { return { -x, -y }; }
	constexpr Vector2 operator!() const { return { -x, -y }; }

	constexpr F32 SqrMagnitude() const { return x * x + y * y; }
	constexpr F32 Magnitude() const { return Math::Sqrt(x * x + y * y); }
	constexpr F32 Dot(const Vector2& v) const { return x * v.x + y * v.y; }
	constexpr F32 Dot(F32 vx, F32 vy) const { return x * vx + y * vy; }
	constexpr Vector2& Normalize() { Vector2 v = Normalized(); x = v.x; y = v.y; return *this; }
	constexpr Vector2 Normalized() const { return IsZero() ? Vector2{ 0.0f } : (*this) / Magnitude(); }
	constexpr Vector2& Normalize(F32& magnitude) { Vector2 v = Normalized(magnitude); x = v.x; y = v.y; return *this; }
	constexpr Vector2 Normalized(F32& magnitude) { magnitude = Magnitude(); return IsZero() ? Vector2{ 0.0f } : (*this) / magnitude; }
	F32 AngleBetween(const Vector2& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
	constexpr Vector2 Projection(const Vector2& v) const { return v * (Dot(v) / v.Dot(v)); }
	constexpr Vector2 OrthoProjection(const Vector2& v) const { return *this - Projection(v); }
	constexpr F32 Cross(const Vector2& v) const { return x * v.y - y * v.x; }
	constexpr Vector2 Cross(F32 f) const { return { y * f, x * -f }; }
	constexpr Vector2 CrossInv(F32 f) const { return { -f * y, f * x }; }
	constexpr Vector2 PerpendicularLeft() const { return { -y, x }; }
	constexpr Vector2 PerpendicularRight() const { return { y, -x }; }
	constexpr Vector2 Normal(const Vector2& v) const { return Vector2(-(v.y - y), v.x - x).Normalized(); }
	constexpr Vector2& Rotate(const Vector2& center, F32 angle)
	{
		F32 cos = Math::Cos(angle * DEG_TO_RAD_F);
		F32 sin = Math::Sin(angle * DEG_TO_RAD_F);
		F32 temp = cos * (x - center.x) - sin * (y - center.y) + center.x;
		y = sin * (x - center.x) + cos * (y - center.y) + center.y;
		x = temp;

		return *this;
	}
	constexpr Vector2 Rotated(const Vector2& center, F32 angle) const
	{
		F32 cos = Math::Cos(angle * DEG_TO_RAD_F);
		F32 sin = Math::Sin(angle * DEG_TO_RAD_F);
		return Vector2{ cos * (x - center.x) - sin * (y - center.y) + center.x,
		sin * (x - center.x) + cos * (y - center.y) + center.y };
	}
	constexpr Vector2& Rotate(F32 angle)
	{
		F32 cos = Math::Cos(angle * DEG_TO_RAD_F);
		F32 sin = Math::Sin(angle * DEG_TO_RAD_F);
		y = cos * x - sin * y;
		x = sin * x + cos * y;

		return *this;
	}
	constexpr Vector2 Rotated(F32 angle) const
	{
		F32 cos = Math::Cos(angle * DEG_TO_RAD_F);
		F32 sin = Math::Sin(angle * DEG_TO_RAD_F);
		return { cos * x - sin * y, sin * x + cos * y };
	}
	constexpr Vector2& Rotate(const Vector2& center, const Quaternion2& quat);
	constexpr Vector2 Rotated(const Vector2& center, const Quaternion2& quat) const;
	constexpr Vector2& Rotate(const Quaternion2& quat);
	constexpr Vector2 Rotated(const Quaternion2& quat) const;
	constexpr Vector2& Clamp(const Vector2& min, const Vector2& max) { x = Math::Clamp(x, min.x, max.x); y = Math::Clamp(y, min.y, max.y); return *this; }
	constexpr Vector2 Clamped(const Vector2& min, const Vector2& max) const { return { Math::Clamp(x, min.x, max.x), Math::Clamp(y, min.y, max.y) }; }
	constexpr Vector2& SetClosest(const Vector2& min, const Vector2& max) { x = Math::Closest(x, min.x, max.x); y = Math::Closest(y, min.y, max.y); return *this; }
	constexpr Vector2 Closest(const Vector2& min, const Vector2& max) const { return { Math::Closest(x, min.x, max.x), Math::Closest(y, min.y, max.y) }; }

	constexpr Vector2 Min(const Vector2& other) { return { Math::Min(x, other.x), Math::Min(y, other.y) }; }
	constexpr Vector2 Max(const Vector2& other) { return { Math::Max(x, other.x), Math::Max(y, other.y) }; }

	constexpr bool Valid() { return !(Math::IsValid(x) || Math::IsValid(y)); }

	F32& operator[] (U64 i) { return (&x)[i]; }
	const F32& operator[] (U64 i) const { return (&x)[i]; }

	F32* Data() { return &x; }
	const F32* Data() const { return &x; }

	constexpr Vector2 xx() const { return { x, x }; }
	constexpr Vector2 xy() const { return { x, y }; }
	constexpr Vector2 yx() const { return { y, x }; }
	constexpr Vector2 yy() const { return { y, y }; }

	constexpr explicit operator Vector3() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector3Int() const;
	constexpr explicit operator Vector4Int() const;

	//operator String8() const { return String8(x, ", ", y); }
	//operator String16() const { return String16(x, u", ", y); }
	//operator String32() const { return String32(x, U", ", y); }

public:
	F32 x, y;
};

export constexpr Vector2 operator*(F32 f, const Vector2& v) { return { f * v.x, f * v.y }; }
export constexpr Vector2 operator/(F32 f, const Vector2& v) { return { f / v.x, f / v.y }; }

export struct NH_API Vector3
{
	constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	constexpr Vector3(F32 f) : x(f), y(f), z(f) {}
	constexpr Vector3(F32 x, F32 y, F32 z) : x(x), y(y), z(z) {}
	constexpr Vector3(const Vector2& v, F32 z) : x(v.x), y(v.y), z(z) {}
	constexpr Vector3(F32 x, const Vector2& v) : x(x), y(v.x), z(v.y) {}
	constexpr Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}
	constexpr Vector3(Vector3&& v) noexcept : x(v.x), y(v.y), z(v.z) {}

	constexpr Vector3& operator=(F32 f) { x = f; y = f; z = f; return *this; }
	constexpr Vector3& operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z; return *this; }
	constexpr Vector3& operator=(Vector3&& v) noexcept { x = v.x; y = v.y; z = v.z; return *this; }

	constexpr Vector3& operator+=(F32 f) { x += f; y += f; z += f; return *this; }
	constexpr Vector3& operator-=(F32 f) { x -= f; y -= f; z -= f; return *this; }
	constexpr Vector3& operator*=(F32 f) { x *= f; y *= f; z *= f; return *this; }
	constexpr Vector3& operator/=(F32 f) { x /= f; y /= f; z /= f; return *this; }
	constexpr Vector3& operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); return *this; }
	constexpr Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	constexpr Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	constexpr Vector3& operator*=(const Vector3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	constexpr Vector3& operator/=(const Vector3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
	constexpr Vector3& operator%=(const Vector3& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); z = Math::Mod(z, v.z); return *this; }
	constexpr Vector3& operator*=(const Quaternion3& q);

	constexpr Vector3 operator+(F32 f) const { return { x + f, y + f, z + f }; }
	constexpr Vector3 operator-(F32 f) const { return { x - f, y - f, z - f }; }
	constexpr Vector3 operator*(F32 f) const { return { x * f, y * f, z * f }; }
	constexpr Vector3 operator/(F32 f) const { return { x / f, y / f, z / f }; }
	constexpr Vector3 operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f) }; }
	constexpr Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
	constexpr Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
	constexpr Vector3 operator*(const Vector3& v) const { return { x * v.x, y * v.y, z * v.z }; }
	constexpr Vector3 operator/(const Vector3& v) const { return { x / v.x, y / v.y, z / v.z }; }
	constexpr Vector3 operator%(const Vector3& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y), Math::Mod(z, v.z) }; }
	constexpr Vector3 operator*(const Quaternion3& q) const;

	constexpr bool operator==(const Vector3& v) const { return Math::IsZero(x - v.x) && Math::IsZero(y - v.y) && Math::IsZero(z - v.z); }
	constexpr bool operator!=(const Vector3& v) const { return !Math::IsZero(x - v.x) || !Math::IsZero(y - v.y) || !Math::IsZero(z - v.z); }
	constexpr bool operator>(const Vector3& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<(const Vector3& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool operator>=(const Vector3& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<=(const Vector3& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool IsZero() const { return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z); }

	constexpr Vector3 operator-() const { return { -x, -y, -z }; }
	constexpr Vector3 operator~() const { return { -x, -y, -z }; }
	constexpr Vector3 operator!() const { return { -x, -y, -z }; }

	constexpr F32 SqrMagnitude() const { return x * x + y * y + z * z; }
	constexpr F32 Magnitude() const { return Math::Sqrt(x * x + y * y + z * z); }
	constexpr F32 Dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
	constexpr F32 Dot(F32 vx, F32 vy, F32 vz) const { return x * vx + y * vy + z * vz; }
	constexpr Vector3& Normalize() { Vector3 v = Normalized(); x = v.x; y = v.y; z = v.z; return *this; }
	constexpr Vector3 Normalized() const { return IsZero() ? Vector3{ 0.0f } : (*this) / Magnitude(); }
	constexpr Vector3 Projection(const Vector3& v) const { return v * (Dot(v) / v.Dot(v)); }
	constexpr Vector3 OrthoProjection(const Vector3& v) const { return *this - Projection(v); }
	constexpr Vector3 Cross(const Vector3& v) const { return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x }; }
	constexpr Vector3 Normal(const Vector3& v) const;
	constexpr Vector3& Rotate(const Vector3& center, const Quaternion3& quat);
	constexpr Vector3 Rotated(const Vector3& center, const Quaternion3& quat) const;
	constexpr Vector3& Clamp(const Vector3& min, const Vector3& max)
	{
		x = Math::Clamp(x, min.x, max.x);
		y = Math::Clamp(y, min.y, max.y);
		z = Math::Clamp(z, min.z, max.z);
		return *this;
	}
	constexpr Vector3 Clamped(const Vector3& min, const Vector3& max) const
	{
		return {
			Math::Clamp(x, min.x, max.x),
			Math::Clamp(y, min.y, max.y),
			Math::Clamp(z, min.z, max.z)
		};
	}
	constexpr Vector3& SetClosest(const Vector3& min, const Vector3& max)
	{
		x = Math::Closest(x, min.x, max.x);
		y = Math::Closest(y, min.y, max.y);
		z = Math::Closest(z, min.z, max.z);
		return *this;
	}
	constexpr Vector3 Closest(const Vector3& min, const Vector3& max) const
	{
		return {
			Math::Closest(x, min.x, max.x),
			Math::Closest(y, min.y, max.y),
			Math::Closest(z, min.z, max.z)
		};
	}

	constexpr Vector3 Min(const Vector3& other) { return { Math::Min(x, other.x), Math::Min(y, other.y), Math::Min(z, other.z) }; }
	constexpr Vector3 Max(const Vector3& other) { return { Math::Max(x, other.x), Math::Max(y, other.y), Math::Max(z, other.z) }; }

	constexpr bool Valid() { return !(Math::IsValid(x) || Math::IsValid(y) || Math::IsValid(z)); }

	F32& operator[] (U64 i) { return (&x)[i]; }
	const F32& operator[] (U64 i) const { return (&x)[i]; }

	F32* Data() { return &x; }
	const F32* Data() const { return &x; }

#pragma region SwizzleFunctions
	constexpr Vector2 xx() const { return { x, x }; }
	constexpr Vector2 xy() const { return { x, y }; }
	constexpr Vector2 xz() const { return { x, z }; }
	constexpr Vector2 yx() const { return { y, x }; }
	constexpr Vector2 yy() const { return { y, y }; }
	constexpr Vector2 yz() const { return { y, z }; }
	constexpr Vector2 zx() const { return { z, x }; }
	constexpr Vector2 zy() const { return { z, y }; }
	constexpr Vector2 zz() const { return { z, z }; }
	constexpr Vector3 xxx() const { return { x, x, x }; }
	constexpr Vector3 xxy() const { return { x, x, y }; }
	constexpr Vector3 xxz() const { return { x, x, z }; }
	constexpr Vector3 xyx() const { return { x, y, x }; }
	constexpr Vector3 xyy() const { return { x, y, y }; }
	constexpr Vector3 xyz() const { return { x, y, z }; }
	constexpr Vector3 xzx() const { return { x, z, x }; }
	constexpr Vector3 xzy() const { return { x, z, y }; }
	constexpr Vector3 xzz() const { return { x, z, z }; }
	constexpr Vector3 yxx() const { return { y, x, x }; }
	constexpr Vector3 yxy() const { return { y, x, y }; }
	constexpr Vector3 yxz() const { return { y, x, z }; }
	constexpr Vector3 yyx() const { return { y, y, x }; }
	constexpr Vector3 yyy() const { return { y, y, y }; }
	constexpr Vector3 yyz() const { return { y, y, z }; }
	constexpr Vector3 yzx() const { return { y, z, x }; }
	constexpr Vector3 yzy() const { return { y, z, y }; }
	constexpr Vector3 yzz() const { return { y, z, z }; }
	constexpr Vector3 zxx() const { return { z, x, x }; }
	constexpr Vector3 zxy() const { return { z, x, y }; }
	constexpr Vector3 zxz() const { return { z, x, z }; }
	constexpr Vector3 zyx() const { return { z, y, x }; }
	constexpr Vector3 zyy() const { return { z, y, y }; }
	constexpr Vector3 zyz() const { return { z, y, z }; }
	constexpr Vector3 zzx() const { return { z, z, x }; }
	constexpr Vector3 zzy() const { return { z, z, y }; }
	constexpr Vector3 zzz() const { return { z, z, z }; }
#pragma endregion

	constexpr explicit operator Vector2() const;
	constexpr explicit operator Vector4() const;
	constexpr explicit operator Vector2Int() const;
	constexpr explicit operator Vector3Int() const;
	constexpr explicit operator Vector4Int() const;

	//operator String8() const { return String8(x, ", ", y, ", ", z); }
	//operator String16() const { return String16(x, u", ", y, u", ", z); }
	//operator String32() const { return String32(x, U", ", y, U", ", z); }

public:
	F32 x, y, z;
};

export constexpr Vector3 operator*(F32 f, const Vector3& v) { return { f * v.x, f * v.y, f * v.z }; }
export constexpr Vector3 operator/(F32 f, const Vector3& v) { return { f / v.x, f / v.y, f / v.z }; }

export struct NH_API Vector4
{
	constexpr Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	constexpr Vector4(F32 f) : x(f), y(f), z(f), w(f) {}
	constexpr Vector4(F32 x, F32 y, F32 z, F32 w) : x(x), y(y), z(z), w(w) {}
	constexpr Vector4(const Vector2& a, const Vector2& b) : x(a.x), y(a.y), z(b.x), w(b.y) {}
	constexpr Vector4(F32 x, const Vector2& a, F32 w) : x(x), y(a.x), z(a.y), w(w) {}
	constexpr Vector4(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	constexpr Vector4(Vector4&& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) {}

	constexpr Vector4& operator=(F32 f) { x = f; y = f; z = f; w = f; return *this; }
	constexpr Vector4& operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
	constexpr Vector4& operator=(Vector4&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

	constexpr Vector4& operator+=(F32 f) { x += f; y += f; z += f; w += f; return *this; }
	constexpr Vector4& operator-=(F32 f) { x -= f; y -= f; z -= f; w -= f; return *this; }
	constexpr Vector4& operator*=(F32 f) { x *= f; y *= f; z *= f; w *= f; return *this; }
	constexpr Vector4& operator/=(F32 f) { x /= f; y /= f; z /= f; w /= f; return *this; }
	constexpr Vector4& operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); w = Math::Mod(w, f); return *this; }
	constexpr Vector4& operator+=(const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	constexpr Vector4& operator-=(const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	constexpr Vector4& operator*=(const Vector4& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	constexpr Vector4& operator/=(const Vector4& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
	constexpr Vector4& operator%=(const Vector4& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); z = Math::Mod(z, v.z); w = Math::Mod(w, v.w); return *this; }

	constexpr Vector4 operator+(F32 f) const { return { x + f, y + f, z + f, w + f }; }
	constexpr Vector4 operator-(F32 f) const { return { x - f, y - f, z - f, w - f }; }
	constexpr Vector4 operator*(F32 f) const { return { x * f, y * f, z * f, w * f }; }
	constexpr Vector4 operator/(F32 f) const { return { x / f, y / f, z / f, w / f }; }
	constexpr Vector4 operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f), Math::Mod(w, f) }; }
	constexpr Vector4 operator+(const Vector4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
	constexpr Vector4 operator-(const Vector4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
	constexpr Vector4 operator*(const Vector4& v) const { return { x * v.x, y * v.y, z * v.z, w * v.w }; }
	constexpr Vector4 operator/(const Vector4& v) const { return { x / v.x, y / v.y, z / v.z, w / v.w }; }
	constexpr Vector4 operator%(const Vector4& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y), Math::Mod(z, v.z), Math::Mod(w, v.w) }; }

	constexpr bool operator==(const Vector4& v) const { return Math::IsZero(x - v.x) && Math::IsZero(y - v.y) && Math::IsZero(z - v.z) && Math::IsZero(w - v.w); }
	constexpr bool operator!=(const Vector4& v) const { return !Math::IsZero(x - v.x) || !Math::IsZero(y - v.y) || !Math::IsZero(z - v.z) || !Math::IsZero(w - v.w); }
	constexpr bool operator>(const Vector4& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<(const Vector4& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool operator>=(const Vector4& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<=(const Vector4& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool IsZero() const { return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z) && Math::IsZero(w); }

	constexpr Vector4 operator-() const { return { -x, -y, -z, -w }; }
	constexpr Vector4 operator~() const { return { -x, -y, -z, -w }; }
	constexpr Vector4 operator!() const { return { -x, -y, -z, -w }; }

	constexpr F32 SqrMagnitude() const { return x * x + y * y + z * z + w * w; }
	constexpr F32 Magnitude() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
	constexpr F32 Dot(const Vector4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
	constexpr F32 Dot(F32 vx, F32 vy, F32 vz, F32 vw) const { return x * vx + y * vy + z * vz + w * vw; }
	constexpr Vector4& Normalize() { Vector4 v = Normalized(); x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
	constexpr Vector4 Normalized() const { return IsZero() ? Vector4{ 0.0f } : (Vector3(this->x, this->y, this->z) / Magnitude(), this->z); }
	constexpr Vector4 Projection(const Vector4& v) const { return v * (Dot(v) / v.Dot(v)); }
	constexpr Vector4 OrthoProjection(const Vector4& v) const { return *this - Projection(v); }
	constexpr Vector4& Clamp(const Vector4& min, const Vector4& max)
	{
		x = Math::Clamp(x, min.x, max.x);
		y = Math::Clamp(y, min.y, max.y);
		z = Math::Clamp(z, min.z, max.z);
		w = Math::Clamp(w, min.w, max.w);
		return *this;
	}
	constexpr Vector4 Clamped(const Vector4& min, const Vector4& max) const
	{
		return {
			Math::Clamp(x, min.x, max.x),
			Math::Clamp(y, min.y, max.y),
			Math::Clamp(z, min.z, max.z),
			Math::Clamp(w, min.w, max.w)
		};
	}
	constexpr Vector4& SetClosest(const Vector4& min, const Vector4& max)
	{
		x = Math::Closest(x, min.x, max.x);
		y = Math::Closest(y, min.y, max.y);
		z = Math::Closest(z, min.z, max.z);
		w = Math::Closest(w, min.w, max.w);
		return *this;
	}
	constexpr Vector4 Closest(const Vector4& min, const Vector4& max) const
	{
		return {
			Math::Closest(x, min.x, max.x),
			Math::Closest(y, min.y, max.y),
			Math::Closest(z, min.z, max.z),
			Math::Closest(w, min.w, max.w)
		};
	}

	constexpr Vector4 Min(const Vector4& other) { return { Math::Min(x, other.x), Math::Min(y, other.y), Math::Min(z, other.z), Math::Min(w, other.w) }; }
	constexpr Vector4 Max(const Vector4& other) { return { Math::Max(x, other.x), Math::Max(y, other.y), Math::Max(z, other.z), Math::Max(w, other.w) }; }

	constexpr bool Valid() { return !(Math::IsValid(x) || Math::IsValid(y) || Math::IsValid(z) || Math::IsValid(w)); }

	F32& operator[] (U64 i) { return (&x)[i]; }
	const F32& operator[] (U64 i) const { return (&x)[i]; }

	F32* Data() { return &x; }
	const F32* Data() const { return &x; }

#pragma region SwizzleFunctions
	constexpr Vector2 xx() const { return { x, x }; }
	constexpr Vector2 xy() const { return { x, y }; }
	constexpr Vector2 xz() const { return { x, z }; }
	constexpr Vector2 xw() const { return { x, w }; }
	constexpr Vector2 yx() const { return { y, x }; }
	constexpr Vector2 yy() const { return { y, y }; }
	constexpr Vector2 yz() const { return { y, z }; }
	constexpr Vector2 yw() const { return { y, w }; }
	constexpr Vector2 zx() const { return { z, x }; }
	constexpr Vector2 zy() const { return { z, y }; }
	constexpr Vector2 zz() const { return { z, z }; }
	constexpr Vector2 zw() const { return { z, w }; }
	constexpr Vector2 wx() const { return { w, x }; }
	constexpr Vector2 wy() const { return { w, y }; }
	constexpr Vector2 wz() const { return { w, z }; }
	constexpr Vector2 ww() const { return { w, w }; }
	constexpr Vector3 xxx() const { return { x, x, x }; }
	constexpr Vector3 xxy() const { return { x, x, y }; }
	constexpr Vector3 xxz() const { return { x, x, z }; }
	constexpr Vector3 xxw() const { return { x, x, w }; }
	constexpr Vector3 xyx() const { return { x, y, x }; }
	constexpr Vector3 xyy() const { return { x, y, y }; }
	constexpr Vector3 xyz() const { return { x, y, z }; }
	constexpr Vector3 xyw() const { return { x, y, w }; }
	constexpr Vector3 xzx() const { return { x, z, x }; }
	constexpr Vector3 xzy() const { return { x, z, y }; }
	constexpr Vector3 xzz() const { return { x, z, z }; }
	constexpr Vector3 xzw() const { return { x, z, w }; }
	constexpr Vector3 xwx() const { return { x, w, x }; }
	constexpr Vector3 xwy() const { return { x, w, y }; }
	constexpr Vector3 xwz() const { return { x, w, z }; }
	constexpr Vector3 xww() const { return { x, w, w }; }
	constexpr Vector3 yxx() const { return { y, x, x }; }
	constexpr Vector3 yxy() const { return { y, x, y }; }
	constexpr Vector3 yxz() const { return { y, x, z }; }
	constexpr Vector3 yxw() const { return { y, x, w }; }
	constexpr Vector3 yyx() const { return { y, y, x }; }
	constexpr Vector3 yyy() const { return { y, y, y }; }
	constexpr Vector3 yyz() const { return { y, y, z }; }
	constexpr Vector3 yyw() const { return { y, y, w }; }
	constexpr Vector3 yzx() const { return { y, z, x }; }
	constexpr Vector3 yzy() const { return { y, z, y }; }
	constexpr Vector3 yzz() const { return { y, z, z }; }
	constexpr Vector3 yzw() const { return { y, z, w }; }
	constexpr Vector3 ywx() const { return { y, w, x }; }
	constexpr Vector3 ywy() const { return { y, w, y }; }
	constexpr Vector3 ywz() const { return { y, w, z }; }
	constexpr Vector3 yww() const { return { y, w, w }; }
	constexpr Vector3 zxx() const { return { z, x, x }; }
	constexpr Vector3 zxy() const { return { z, x, y }; }
	constexpr Vector3 zxz() const { return { z, x, z }; }
	constexpr Vector3 zxw() const { return { z, x, w }; }
	constexpr Vector3 zyx() const { return { z, y, x }; }
	constexpr Vector3 zyy() const { return { z, y, y }; }
	constexpr Vector3 zyz() const { return { z, y, z }; }
	constexpr Vector3 zyw() const { return { z, y, w }; }
	constexpr Vector3 zzx() const { return { z, z, x }; }
	constexpr Vector3 zzy() const { return { z, z, y }; }
	constexpr Vector3 zzz() const { return { z, z, z }; }
	constexpr Vector3 zzw() const { return { z, z, w }; }
	constexpr Vector3 zwx() const { return { z, w, x }; }
	constexpr Vector3 zwy() const { return { z, w, y }; }
	constexpr Vector3 zwz() const { return { z, w, z }; }
	constexpr Vector3 zww() const { return { z, w, w }; }
	constexpr Vector3 wxx() const { return { w, x, x }; }
	constexpr Vector3 wxy() const { return { w, x, y }; }
	constexpr Vector3 wxz() const { return { w, x, z }; }
	constexpr Vector3 wxw() const { return { w, x, w }; }
	constexpr Vector3 wyx() const { return { w, y, x }; }
	constexpr Vector3 wyy() const { return { w, y, y }; }
	constexpr Vector3 wyz() const { return { w, y, z }; }
	constexpr Vector3 wyw() const { return { w, y, w }; }
	constexpr Vector3 wzx() const { return { w, z, x }; }
	constexpr Vector3 wzy() const { return { w, z, y }; }
	constexpr Vector3 wzz() const { return { w, z, z }; }
	constexpr Vector3 wzw() const { return { w, z, w }; }
	constexpr Vector3 wwx() const { return { w, w, x }; }
	constexpr Vector3 wwy() const { return { w, w, y }; }
	constexpr Vector3 wwz() const { return { w, w, z }; }
	constexpr Vector3 www() const { return { w, w, w }; }
	constexpr Vector4 xxxx() const { return { x, x, x, x }; }
	constexpr Vector4 xxxy() const { return { x, x, x, y }; }
	constexpr Vector4 xxxz() const { return { x, x, x, z }; }
	constexpr Vector4 xxxw() const { return { x, x, x, w }; }
	constexpr Vector4 xxyx() const { return { x, x, y, x }; }
	constexpr Vector4 xxyy() const { return { x, x, y, y }; }
	constexpr Vector4 xxyz() const { return { x, x, y, z }; }
	constexpr Vector4 xxyw() const { return { x, x, y, w }; }
	constexpr Vector4 xxzx() const { return { x, x, z, x }; }
	constexpr Vector4 xxzy() const { return { x, x, z, y }; }
	constexpr Vector4 xxzz() const { return { x, x, z, z }; }
	constexpr Vector4 xxzw() const { return { x, x, z, w }; }
	constexpr Vector4 xxwx() const { return { x, x, w, x }; }
	constexpr Vector4 xxwy() const { return { x, x, w, y }; }
	constexpr Vector4 xxwz() const { return { x, x, w, z }; }
	constexpr Vector4 xxww() const { return { x, x, w, w }; }
	constexpr Vector4 xyxx() const { return { x, y, x, x }; }
	constexpr Vector4 xyxy() const { return { x, y, x, y }; }
	constexpr Vector4 xyxz() const { return { x, y, x, z }; }
	constexpr Vector4 xyxw() const { return { x, y, x, w }; }
	constexpr Vector4 xyyx() const { return { x, y, y, x }; }
	constexpr Vector4 xyyy() const { return { x, y, y, y }; }
	constexpr Vector4 xyyz() const { return { x, y, y, z }; }
	constexpr Vector4 xyyw() const { return { x, y, y, w }; }
	constexpr Vector4 xyzx() const { return { x, y, z, x }; }
	constexpr Vector4 xyzy() const { return { x, y, z, y }; }
	constexpr Vector4 xyzz() const { return { x, y, z, z }; }
	constexpr Vector4 xyzw() const { return { x, y, z, w }; }
	constexpr Vector4 xywx() const { return { x, y, w, x }; }
	constexpr Vector4 xywy() const { return { x, y, w, y }; }
	constexpr Vector4 xywz() const { return { x, y, w, z }; }
	constexpr Vector4 xyww() const { return { x, y, w, w }; }
	constexpr Vector4 xzxx() const { return { x, z, x, x }; }
	constexpr Vector4 xzxy() const { return { x, z, x, y }; }
	constexpr Vector4 xzxz() const { return { x, z, x, z }; }
	constexpr Vector4 xzxw() const { return { x, z, x, w }; }
	constexpr Vector4 xzyx() const { return { x, z, y, x }; }
	constexpr Vector4 xzyy() const { return { x, z, y, y }; }
	constexpr Vector4 xzyz() const { return { x, z, y, z }; }
	constexpr Vector4 xzyw() const { return { x, z, y, w }; }
	constexpr Vector4 xzzx() const { return { x, z, z, x }; }
	constexpr Vector4 xzzy() const { return { x, z, z, y }; }
	constexpr Vector4 xzzz() const { return { x, z, z, z }; }
	constexpr Vector4 xzzw() const { return { x, z, z, w }; }
	constexpr Vector4 xzwx() const { return { x, z, w, x }; }
	constexpr Vector4 xzwy() const { return { x, z, w, y }; }
	constexpr Vector4 xzwz() const { return { x, z, w, z }; }
	constexpr Vector4 xzww() const { return { x, z, w, w }; }
	constexpr Vector4 xwxx() const { return { x, w, x, x }; }
	constexpr Vector4 xwxy() const { return { x, w, x, y }; }
	constexpr Vector4 xwxz() const { return { x, w, x, z }; }
	constexpr Vector4 xwxw() const { return { x, w, x, w }; }
	constexpr Vector4 xwyx() const { return { x, w, y, x }; }
	constexpr Vector4 xwyy() const { return { x, w, y, y }; }
	constexpr Vector4 xwyz() const { return { x, w, y, z }; }
	constexpr Vector4 xwyw() const { return { x, w, y, w }; }
	constexpr Vector4 xwzx() const { return { x, w, z, x }; }
	constexpr Vector4 xwzy() const { return { x, w, z, y }; }
	constexpr Vector4 xwzz() const { return { x, w, z, z }; }
	constexpr Vector4 xwzw() const { return { x, w, z, w }; }
	constexpr Vector4 xwwx() const { return { x, w, w, x }; }
	constexpr Vector4 xwwy() const { return { x, w, w, y }; }
	constexpr Vector4 xwwz() const { return { x, w, w, z }; }
	constexpr Vector4 xwww() const { return { x, w, w, w }; }
	constexpr Vector4 yxxx() const { return { y, x, x, x }; }
	constexpr Vector4 yxxy() const { return { y, x, x, y }; }
	constexpr Vector4 yxxz() const { return { y, x, x, z }; }
	constexpr Vector4 yxxw() const { return { y, x, x, w }; }
	constexpr Vector4 yxyx() const { return { y, x, y, x }; }
	constexpr Vector4 yxyy() const { return { y, x, y, y }; }
	constexpr Vector4 yxyz() const { return { y, x, y, z }; }
	constexpr Vector4 yxyw() const { return { y, x, y, w }; }
	constexpr Vector4 yxzx() const { return { y, x, z, x }; }
	constexpr Vector4 yxzy() const { return { y, x, z, y }; }
	constexpr Vector4 yxzz() const { return { y, x, z, z }; }
	constexpr Vector4 yxzw() const { return { y, x, z, w }; }
	constexpr Vector4 yxwx() const { return { y, x, w, x }; }
	constexpr Vector4 yxwy() const { return { y, x, w, y }; }
	constexpr Vector4 yxwz() const { return { y, x, w, z }; }
	constexpr Vector4 yxww() const { return { y, x, w, w }; }
	constexpr Vector4 yyxx() const { return { y, y, x, x }; }
	constexpr Vector4 yyxy() const { return { y, y, x, y }; }
	constexpr Vector4 yyxz() const { return { y, y, x, z }; }
	constexpr Vector4 yyxw() const { return { y, y, x, w }; }
	constexpr Vector4 yyyx() const { return { y, y, y, x }; }
	constexpr Vector4 yyyy() const { return { y, y, y, y }; }
	constexpr Vector4 yyyz() const { return { y, y, y, z }; }
	constexpr Vector4 yyyw() const { return { y, y, y, w }; }
	constexpr Vector4 yyzx() const { return { y, y, z, x }; }
	constexpr Vector4 yyzy() const { return { y, y, z, y }; }
	constexpr Vector4 yyzz() const { return { y, y, z, z }; }
	constexpr Vector4 yyzw() const { return { y, y, z, w }; }
	constexpr Vector4 yywx() const { return { y, y, w, x }; }
	constexpr Vector4 yywy() const { return { y, y, w, y }; }
	constexpr Vector4 yywz() const { return { y, y, w, z }; }
	constexpr Vector4 yyww() const { return { y, y, w, w }; }
	constexpr Vector4 yzxx() const { return { y, z, x, x }; }
	constexpr Vector4 yzxy() const { return { y, z, x, y }; }
	constexpr Vector4 yzxz() const { return { y, z, x, z }; }
	constexpr Vector4 yzxw() const { return { y, z, x, w }; }
	constexpr Vector4 yzyx() const { return { y, z, y, x }; }
	constexpr Vector4 yzyy() const { return { y, z, y, y }; }
	constexpr Vector4 yzyz() const { return { y, z, y, z }; }
	constexpr Vector4 yzyw() const { return { y, z, y, w }; }
	constexpr Vector4 yzzx() const { return { y, z, z, x }; }
	constexpr Vector4 yzzy() const { return { y, z, z, y }; }
	constexpr Vector4 yzzz() const { return { y, z, z, z }; }
	constexpr Vector4 yzzw() const { return { y, z, z, w }; }
	constexpr Vector4 yzwx() const { return { y, z, w, x }; }
	constexpr Vector4 yzwy() const { return { y, z, w, y }; }
	constexpr Vector4 yzwz() const { return { y, z, w, z }; }
	constexpr Vector4 yzww() const { return { y, z, w, w }; }
	constexpr Vector4 ywxx() const { return { y, w, x, x }; }
	constexpr Vector4 ywxy() const { return { y, w, x, y }; }
	constexpr Vector4 ywxz() const { return { y, w, x, z }; }
	constexpr Vector4 ywxw() const { return { y, w, x, w }; }
	constexpr Vector4 ywyx() const { return { y, w, y, x }; }
	constexpr Vector4 ywyy() const { return { y, w, y, y }; }
	constexpr Vector4 ywyz() const { return { y, w, y, z }; }
	constexpr Vector4 ywyw() const { return { y, w, y, w }; }
	constexpr Vector4 ywzx() const { return { y, w, z, x }; }
	constexpr Vector4 ywzy() const { return { y, w, z, y }; }
	constexpr Vector4 ywzz() const { return { y, w, z, z }; }
	constexpr Vector4 ywzw() const { return { y, w, z, w }; }
	constexpr Vector4 ywwx() const { return { y, w, w, x }; }
	constexpr Vector4 ywwy() const { return { y, w, w, y }; }
	constexpr Vector4 ywwz() const { return { y, w, w, z }; }
	constexpr Vector4 ywww() const { return { y, w, w, w }; }
	constexpr Vector4 zxxx() const { return { z, x, x, x }; }
	constexpr Vector4 zxxy() const { return { z, x, x, y }; }
	constexpr Vector4 zxxz() const { return { z, x, x, z }; }
	constexpr Vector4 zxxw() const { return { z, x, x, w }; }
	constexpr Vector4 zxyx() const { return { z, x, y, x }; }
	constexpr Vector4 zxyy() const { return { z, x, y, y }; }
	constexpr Vector4 zxyz() const { return { z, x, y, z }; }
	constexpr Vector4 zxyw() const { return { z, x, y, w }; }
	constexpr Vector4 zxzx() const { return { z, x, z, x }; }
	constexpr Vector4 zxzy() const { return { z, x, z, y }; }
	constexpr Vector4 zxzz() const { return { z, x, z, z }; }
	constexpr Vector4 zxzw() const { return { z, x, z, w }; }
	constexpr Vector4 zxwx() const { return { z, x, w, x }; }
	constexpr Vector4 zxwy() const { return { z, x, w, y }; }
	constexpr Vector4 zxwz() const { return { z, x, w, z }; }
	constexpr Vector4 zxww() const { return { z, x, w, w }; }
	constexpr Vector4 zyxx() const { return { z, y, x, x }; }
	constexpr Vector4 zyxy() const { return { z, y, x, y }; }
	constexpr Vector4 zyxz() const { return { z, y, x, z }; }
	constexpr Vector4 zyxw() const { return { z, y, x, w }; }
	constexpr Vector4 zyyx() const { return { z, y, y, x }; }
	constexpr Vector4 zyyy() const { return { z, y, y, y }; }
	constexpr Vector4 zyyz() const { return { z, y, y, z }; }
	constexpr Vector4 zyyw() const { return { z, y, y, w }; }
	constexpr Vector4 zyzx() const { return { z, y, z, x }; }
	constexpr Vector4 zyzy() const { return { z, y, z, y }; }
	constexpr Vector4 zyzz() const { return { z, y, z, z }; }
	constexpr Vector4 zyzw() const { return { z, y, z, w }; }
	constexpr Vector4 zywx() const { return { z, y, w, x }; }
	constexpr Vector4 zywy() const { return { z, y, w, y }; }
	constexpr Vector4 zywz() const { return { z, y, w, z }; }
	constexpr Vector4 zyww() const { return { z, y, w, w }; }
	constexpr Vector4 zzxx() const { return { z, z, x, x }; }
	constexpr Vector4 zzxy() const { return { z, z, x, y }; }
	constexpr Vector4 zzxz() const { return { z, z, x, z }; }
	constexpr Vector4 zzxw() const { return { z, z, x, w }; }
	constexpr Vector4 zzyx() const { return { z, z, y, x }; }
	constexpr Vector4 zzyy() const { return { z, z, y, y }; }
	constexpr Vector4 zzyz() const { return { z, z, y, z }; }
	constexpr Vector4 zzyw() const { return { z, z, y, w }; }
	constexpr Vector4 zzzx() const { return { z, z, z, x }; }
	constexpr Vector4 zzzy() const { return { z, z, z, y }; }
	constexpr Vector4 zzzz() const { return { z, z, z, z }; }
	constexpr Vector4 zzzw() const { return { z, z, z, w }; }
	constexpr Vector4 zzwx() const { return { z, z, w, x }; }
	constexpr Vector4 zzwy() const { return { z, z, w, y }; }
	constexpr Vector4 zzwz() const { return { z, z, w, z }; }
	constexpr Vector4 zzww() const { return { z, z, w, w }; }
	constexpr Vector4 zwxx() const { return { z, w, x, x }; }
	constexpr Vector4 zwxy() const { return { z, w, x, y }; }
	constexpr Vector4 zwxz() const { return { z, w, x, z }; }
	constexpr Vector4 zwxw() const { return { z, w, x, w }; }
	constexpr Vector4 zwyx() const { return { z, w, y, x }; }
	constexpr Vector4 zwyy() const { return { z, w, y, y }; }
	constexpr Vector4 zwyz() const { return { z, w, y, z }; }
	constexpr Vector4 zwyw() const { return { z, w, y, w }; }
	constexpr Vector4 zwzx() const { return { z, w, z, x }; }
	constexpr Vector4 zwzy() const { return { z, w, z, y }; }
	constexpr Vector4 zwzz() const { return { z, w, z, z }; }
	constexpr Vector4 zwzw() const { return { z, w, z, w }; }
	constexpr Vector4 zwwx() const { return { z, w, w, x }; }
	constexpr Vector4 zwwy() const { return { z, w, w, y }; }
	constexpr Vector4 zwwz() const { return { z, w, w, z }; }
	constexpr Vector4 zwww() const { return { z, w, w, w }; }
	constexpr Vector4 wxxx() const { return { w, x, x, x }; }
	constexpr Vector4 wxxy() const { return { w, x, x, y }; }
	constexpr Vector4 wxxz() const { return { w, x, x, z }; }
	constexpr Vector4 wxxw() const { return { w, x, x, w }; }
	constexpr Vector4 wxyx() const { return { w, x, y, x }; }
	constexpr Vector4 wxyy() const { return { w, x, y, y }; }
	constexpr Vector4 wxyz() const { return { w, x, y, z }; }
	constexpr Vector4 wxyw() const { return { w, x, y, w }; }
	constexpr Vector4 wxzx() const { return { w, x, z, x }; }
	constexpr Vector4 wxzy() const { return { w, x, z, y }; }
	constexpr Vector4 wxzz() const { return { w, x, z, z }; }
	constexpr Vector4 wxzw() const { return { w, x, z, w }; }
	constexpr Vector4 wxwx() const { return { w, x, w, x }; }
	constexpr Vector4 wxwy() const { return { w, x, w, y }; }
	constexpr Vector4 wxwz() const { return { w, x, w, z }; }
	constexpr Vector4 wxww() const { return { w, x, w, w }; }
	constexpr Vector4 wyxx() const { return { w, y, x, x }; }
	constexpr Vector4 wyxy() const { return { w, y, x, y }; }
	constexpr Vector4 wyxz() const { return { w, y, x, z }; }
	constexpr Vector4 wyxw() const { return { w, y, x, w }; }
	constexpr Vector4 wyyx() const { return { w, y, y, x }; }
	constexpr Vector4 wyyy() const { return { w, y, y, y }; }
	constexpr Vector4 wyyz() const { return { w, y, y, z }; }
	constexpr Vector4 wyyw() const { return { w, y, y, w }; }
	constexpr Vector4 wyzx() const { return { w, y, z, x }; }
	constexpr Vector4 wyzy() const { return { w, y, z, y }; }
	constexpr Vector4 wyzz() const { return { w, y, z, z }; }
	constexpr Vector4 wyzw() const { return { w, y, z, w }; }
	constexpr Vector4 wywx() const { return { w, y, w, x }; }
	constexpr Vector4 wywy() const { return { w, y, w, y }; }
	constexpr Vector4 wywz() const { return { w, y, w, z }; }
	constexpr Vector4 wyww() const { return { w, y, w, w }; }
	constexpr Vector4 wzxx() const { return { w, z, x, x }; }
	constexpr Vector4 wzxy() const { return { w, z, x, y }; }
	constexpr Vector4 wzxz() const { return { w, z, x, z }; }
	constexpr Vector4 wzxw() const { return { w, z, x, w }; }
	constexpr Vector4 wzyx() const { return { w, z, y, x }; }
	constexpr Vector4 wzyy() const { return { w, z, y, y }; }
	constexpr Vector4 wzyz() const { return { w, z, y, z }; }
	constexpr Vector4 wzyw() const { return { w, z, y, w }; }
	constexpr Vector4 wzzx() const { return { w, z, z, x }; }
	constexpr Vector4 wzzy() const { return { w, z, z, y }; }
	constexpr Vector4 wzzz() const { return { w, z, z, z }; }
	constexpr Vector4 wzzw() const { return { w, z, z, w }; }
	constexpr Vector4 wzwx() const { return { w, z, w, x }; }
	constexpr Vector4 wzwy() const { return { w, z, w, y }; }
	constexpr Vector4 wzwz() const { return { w, z, w, z }; }
	constexpr Vector4 wzww() const { return { w, z, w, w }; }
	constexpr Vector4 wwxx() const { return { w, w, x, x }; }
	constexpr Vector4 wwxy() const { return { w, w, x, y }; }
	constexpr Vector4 wwxz() const { return { w, w, x, z }; }
	constexpr Vector4 wwxw() const { return { w, w, x, w }; }
	constexpr Vector4 wwyx() const { return { w, w, y, x }; }
	constexpr Vector4 wwyy() const { return { w, w, y, y }; }
	constexpr Vector4 wwyz() const { return { w, w, y, z }; }
	constexpr Vector4 wwyw() const { return { w, w, y, w }; }
	constexpr Vector4 wwzx() const { return { w, w, z, x }; }
	constexpr Vector4 wwzy() const { return { w, w, z, y }; }
	constexpr Vector4 wwzz() const { return { w, w, z, z }; }
	constexpr Vector4 wwzw() const { return { w, w, z, w }; }
	constexpr Vector4 wwwx() const { return { w, w, w, x }; }
	constexpr Vector4 wwwy() const { return { w, w, w, y }; }
	constexpr Vector4 wwwz() const { return { w, w, w, z }; }
	constexpr Vector4 wwww() const { return { w, w, w, w }; }
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
};

export constexpr Vector4 operator*(F32 f, const Vector4& v) { return { f * v.x, f * v.y, f * v.z, f * v.w }; }
export constexpr Vector4 operator/(F32 f, const Vector4& v) { return { f / v.x, f / v.y, f / v.z, f / v.w }; }

export struct NH_API Vector2Int
{
	constexpr Vector2Int() : x(0), y(0) {}
	constexpr Vector2Int(I32 i) : x(i), y(i) {}
	constexpr Vector2Int(I32 x, I32 y) : x(x), y(y) {}
	constexpr Vector2Int(const Vector2Int& v) : x(v.x), y(v.y) {}
	constexpr Vector2Int(Vector2Int&& v) noexcept : x(v.x), y(v.y) {}

	constexpr Vector2Int& operator=(I32 i) { x = i; y = i; return *this; }
	constexpr Vector2Int& operator=(const Vector2Int& v) { x = v.x; y = v.y; return *this; }
	constexpr Vector2Int& operator=(Vector2Int&& v) noexcept { x = v.x; y = v.y; return *this; }

	constexpr Vector2Int& operator+=(I32 i) { x += i; y += i; return *this; }
	constexpr Vector2Int& operator-=(I32 i) { x -= i; y -= i; return *this; }
	constexpr Vector2Int& operator*=(I32 i) { x *= i; y *= i; return *this; }
	constexpr Vector2Int& operator/=(I32 i) { x /= i; y /= i; return *this; }
	constexpr Vector2Int& operator%=(I32 i) { x %= i; y %= i; return *this; }
	constexpr Vector2Int& operator+=(const Vector2Int& v) { x += v.x; y += v.y; return *this; }
	constexpr Vector2Int& operator-=(const Vector2Int& v) { x -= v.x; y -= v.y; return *this; }
	constexpr Vector2Int& operator*=(const Vector2Int& v) { x *= v.x; y *= v.y; return *this; }
	constexpr Vector2Int& operator/=(const Vector2Int& v) { x /= v.x; y /= v.y; return *this; }
	constexpr Vector2Int& operator%=(const Vector2Int& v) { x &= v.x; y &= v.y; return *this; }

	constexpr Vector2Int operator+(I32 i) const { return { x + i, y + i }; }
	constexpr Vector2Int operator-(I32 i) const { return { x - i, y - i }; }
	constexpr Vector2Int operator*(I32 i) const { return { x * i, y * i }; }
	constexpr Vector2Int operator/(I32 i) const { return { x / i, y / i }; }
	constexpr Vector2Int operator%(I32 i) const { return { x % i, y % i }; }
	constexpr Vector2Int operator+(const Vector2Int& v) const { return { x + v.x, y + v.y }; }
	constexpr Vector2Int operator-(const Vector2Int& v) const { return { x - v.x, y - v.y }; }
	constexpr Vector2Int operator*(const Vector2Int& v) const { return { x * v.x, y * v.y }; }
	constexpr Vector2Int operator/(const Vector2Int& v) const { return { x / v.x, y / v.y }; }
	constexpr Vector2Int operator%(const Vector2Int& v) const { return { x % v.x, y % v.y }; }
	constexpr Vector2Int operator+(const Vector2& v) const { return { (I32)(x + v.x), (I32)(y + v.y) }; }
	constexpr Vector2Int operator-(const Vector2& v) const { return { (I32)(x - v.x), (I32)(y - v.y) }; }
	constexpr Vector2Int operator*(const Vector2& v) const { return { (I32)(x * v.x), (I32)(y * v.y) }; }
	constexpr Vector2Int operator/(const Vector2& v) const { return { (I32)(x / v.x), (I32)(y / v.y) }; }

	constexpr bool operator==(const Vector2Int& v) const { return !(x - v.x) && !(y - v.y); }
	constexpr bool operator!=(const Vector2Int& v) const { return (x - v.x) || (y - v.y); }
	constexpr bool operator>(const Vector2Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<(const Vector2Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool operator>=(const Vector2Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<=(const Vector2Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool IsZero() const { return !x && !y; }

	constexpr Vector2Int operator-() const { return { -x, -y }; }
	constexpr Vector2Int operator~() const { return { -x, -y }; }
	constexpr Vector2Int operator!() const { return { -x, -y }; }

	constexpr I32 SqrMagnitude() const { return x * x + y * y; }
	constexpr F32 Magnitude() const { return Math::Sqrt((F32)(x * x + y * y)); }
	constexpr I32 Dot(const Vector2Int& v) const { return x * v.x + y * v.y; }
	constexpr I32 Dot(I32 vx, I32 vy) const { return x * vx + y * vy; }

	constexpr Vector2Int& Clamp(const Vector2Int& min, const Vector2Int& max)
	{
		x = Math::Clamp(x, min.x, max.x);
		y = Math::Clamp(y, min.y, max.y);
		return *this;
	}
	constexpr Vector2Int Clamped(const Vector2Int& min, const Vector2Int& max) const
	{
		return {
			Math::Clamp(x, min.x, max.x),
			Math::Clamp(y, min.y, max.y)
		};
	}
	constexpr Vector2Int& SetClosest(const Vector2Int& min, const Vector2Int& max)
	{
		x = Math::Closest(x, min.x, max.x);
		y = Math::Closest(y, min.y, max.y);
		return *this;
	}
	constexpr Vector2Int Closest(const Vector2Int& min, const Vector2Int& max) const
	{
		return {
			Math::Closest(x, min.x, max.x),
			Math::Closest(y, min.y, max.y)
		};
	}

	I32& operator[] (U64 i) { return (&x)[i]; }
	const I32& operator[] (U64 i) const { return (&x)[i]; }

	I32* Data() { return &x; }
	const I32* Data() const { return &x; }

	constexpr Vector2Int xx() const { return { x, x }; }
	constexpr Vector2Int xy() const { return { x, y }; }
	constexpr Vector2Int yx() const { return { y, x }; }
	constexpr Vector2Int yy() const { return { y, y }; }

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
};

export constexpr Vector2Int operator*(F32 f, const Vector2Int& v) { return { (I32)(f * v.x), (I32)(f * v.y) }; }
export constexpr Vector2Int operator/(F32 f, const Vector2Int& v) { return { (I32)(f / v.x), (I32)(f / v.y) }; }
export constexpr Vector2Int operator*(I32 i, const Vector2Int& v) { return { i * v.x, i * v.y }; }
export constexpr Vector2Int operator/(I32 i, const Vector2Int& v) { return { i / v.x, i / v.y }; }

export struct NH_API Vector3Int
{
	constexpr Vector3Int() : x(0), y(0), z(0) {}
	constexpr Vector3Int(I32 i) : x(i), y(i), z(i) {}
	constexpr Vector3Int(I32 x, I32 y, I32 z) : x(x), y(y), z(z) {}
	constexpr Vector3Int(const Vector3Int& v) : x(v.x), y(v.y), z(v.z) {}
	constexpr Vector3Int(Vector3Int&& v) noexcept : x(v.x), y(v.y), z(v.z) {}

	constexpr Vector3Int& operator=(I32 i) { x = i; y = i; z = i; return *this; }
	constexpr Vector3Int& operator=(const Vector3Int& v) { x = v.x; y = v.y; z = v.z; return *this; }
	constexpr Vector3Int& operator=(Vector3Int&& v) noexcept { x = v.x; y = v.y; z = v.z; return *this; }

	constexpr Vector3Int& operator+=(I32 i) { x += i; y += i; z += i; return *this; }
	constexpr Vector3Int& operator-=(I32 i) { x -= i; y -= i; z -= i; return *this; }
	constexpr Vector3Int& operator*=(I32 i) { x *= i; y *= i; z *= i; return *this; }
	constexpr Vector3Int& operator/=(I32 i) { x /= i; y /= i; z /= i; return *this; }
	constexpr Vector3Int& operator%=(I32 i) { x %= i; y %= i; z %= i; return *this; }
	constexpr Vector3Int& operator+=(const Vector3Int& v) { x += v.x; y += v.y; z += v.z; return *this; }
	constexpr Vector3Int& operator-=(const Vector3Int& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	constexpr Vector3Int& operator*=(const Vector3Int& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	constexpr Vector3Int& operator/=(const Vector3Int& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
	constexpr Vector3Int& operator%=(const Vector3Int& v) { x %= v.x; y %= v.y; z %= v.z; return *this; }

	constexpr Vector3Int operator+(I32 i) const { return { x + i, y + i, z + i }; }
	constexpr Vector3Int operator-(I32 i) const { return { x - i, y - i, z - i }; }
	constexpr Vector3Int operator*(I32 i) const { return { x * i, y * i, z * i }; }
	constexpr Vector3Int operator/(I32 i) const { return { x / i, y / i, z / i }; }
	constexpr Vector3Int operator%(I32 i) const { return { x % i, y % i, z % i }; }
	constexpr Vector3Int operator+(const Vector3Int& v) const { return { x + v.x, y + v.y, z + v.z }; }
	constexpr Vector3Int operator-(const Vector3Int& v) const { return { x - v.x, y - v.y, z - v.z }; }
	constexpr Vector3Int operator*(const Vector3Int& v) const { return { x * v.x, y * v.y, z * v.z }; }
	constexpr Vector3Int operator/(const Vector3Int& v) const { return { x / v.x, y / v.y, z / v.z }; }
	constexpr Vector3Int operator%(const Vector3Int& v) const { return { x % v.x, y % v.y, z % v.z }; }

	constexpr bool operator==(const Vector3Int& v) const { return !(x - v.x) && !(y - v.y) && !(z - v.z); }
	constexpr bool operator!=(const Vector3Int& v) const { return (x - v.x) || (y - v.y) || (z - v.z); }
	constexpr bool operator>(const Vector3Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<(const Vector3Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool operator>=(const Vector3Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<=(const Vector3Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool IsZero() const { return !x && !y && !z; }

	constexpr Vector3Int operator-() const { return { -x, -y, -z }; }
	constexpr Vector3Int operator~() const { return { -x, -y, -z }; }
	constexpr Vector3Int operator!() const { return { -x, -y, -z }; }

	constexpr I32 SqrMagnitude() const { return x * x + y * y + z * z; }
	constexpr F32 Magnitude() const { return Math::Sqrt((F32)(x * x + y * y + z * z)); }
	constexpr I32 Dot(const Vector3Int& v) const { return x * v.x + y * v.y + z * v.z; }
	constexpr I32 Dot(I32 vx, I32 vy, I32 vz) const { return x * vx + y * vy + z * vz; }

	constexpr Vector3Int& Clamp(const Vector3Int& min, const Vector3Int& max)
	{
		x = Math::Clamp(x, min.x, max.x);
		y = Math::Clamp(y, min.y, max.y);
		z = Math::Clamp(z, min.z, max.z);
		return *this;
	}
	constexpr Vector3Int Clamped(const Vector3Int& min, const Vector3Int& max) const
	{
		return {
			Math::Clamp(x, min.x, max.x),
			Math::Clamp(y, min.y, max.y),
			Math::Clamp(z, min.z, max.z)
		};
	}
	constexpr Vector3Int& SetClosest(const Vector3Int& min, const Vector3Int& max)
	{
		x = Math::Closest(x, min.x, max.x);
		y = Math::Closest(y, min.y, max.y);
		z = Math::Closest(z, min.z, max.z);
		return *this;
	}
	constexpr Vector3Int Closest(const Vector3Int& min, const Vector3Int& max) const
	{
		return {
			Math::Closest(x, min.x, max.x),
			Math::Closest(y, min.y, max.y),
			Math::Closest(z, min.z, max.z)
		};
	}

	I32& operator[] (U64 i) { return (&x)[i]; }
	const I32& operator[] (U64 i) const { return (&x)[i]; }

	I32* Data() { return &x; }
	const I32* Data() const { return &x; }

#pragma region SwizzleFunctions
	constexpr Vector2Int xx() const { return { x, x }; }
	constexpr Vector2Int xy() const { return { x, y }; }
	constexpr Vector2Int xz() const { return { x, z }; }
	constexpr Vector2Int yx() const { return { y, x }; }
	constexpr Vector2Int yy() const { return { y, y }; }
	constexpr Vector2Int yz() const { return { y, z }; }
	constexpr Vector2Int zx() const { return { z, x }; }
	constexpr Vector2Int zy() const { return { z, y }; }
	constexpr Vector2Int zz() const { return { z, z }; }
	constexpr Vector3Int xxx() const { return { x, x, x }; }
	constexpr Vector3Int xxy() const { return { x, x, y }; }
	constexpr Vector3Int xxz() const { return { x, x, z }; }
	constexpr Vector3Int xyx() const { return { x, y, x }; }
	constexpr Vector3Int xyy() const { return { x, y, y }; }
	constexpr Vector3Int xyz() const { return { x, y, z }; }
	constexpr Vector3Int xzx() const { return { x, z, x }; }
	constexpr Vector3Int xzy() const { return { x, z, y }; }
	constexpr Vector3Int xzz() const { return { x, z, z }; }
	constexpr Vector3Int yxx() const { return { y, x, x }; }
	constexpr Vector3Int yxy() const { return { y, x, y }; }
	constexpr Vector3Int yxz() const { return { y, x, z }; }
	constexpr Vector3Int yyx() const { return { y, y, x }; }
	constexpr Vector3Int yyy() const { return { y, y, y }; }
	constexpr Vector3Int yyz() const { return { y, y, z }; }
	constexpr Vector3Int yzx() const { return { y, z, x }; }
	constexpr Vector3Int yzy() const { return { y, z, y }; }
	constexpr Vector3Int yzz() const { return { y, z, z }; }
	constexpr Vector3Int zxx() const { return { z, x, x }; }
	constexpr Vector3Int zxy() const { return { z, x, y }; }
	constexpr Vector3Int zxz() const { return { z, x, z }; }
	constexpr Vector3Int zyx() const { return { z, y, x }; }
	constexpr Vector3Int zyy() const { return { z, y, y }; }
	constexpr Vector3Int zyz() const { return { z, y, z }; }
	constexpr Vector3Int zzx() const { return { z, z, x }; }
	constexpr Vector3Int zzy() const { return { z, z, y }; }
	constexpr Vector3Int zzz() const { return { z, z, z }; }
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
};

export constexpr Vector3Int operator*(F32 f, const Vector3Int& v) { return { (I32)(f * v.x), (I32)(f * v.y), (I32)(f * v.z) }; }
export constexpr Vector3Int operator/(F32 f, const Vector3Int& v) { return { (I32)(f / v.x), (I32)(f / v.y), (I32)(f / v.z) }; }
export constexpr Vector3Int operator*(I32 i, const Vector3Int& v) { return { i * v.x, i * v.y, i * v.z }; }
export constexpr Vector3Int operator/(I32 i, const Vector3Int& v) { return { i / v.x, i / v.y, i / v.z }; }

export struct NH_API Vector4Int
{
	constexpr Vector4Int() : x(0), y(0), z(0), w(0) {}
	constexpr Vector4Int(I32 i) : x(i), y(i), z(i), w(i) {}
	constexpr Vector4Int(I32 x, I32 y, I32 z, I32 w) : x(x), y(y), z(z), w(w) {}
	constexpr Vector4Int(const Vector4Int& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	constexpr Vector4Int(Vector4Int&& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) {}

	constexpr Vector4Int& operator=(I32 i) { x = i; y = i; z = i; w = i; return *this; }
	constexpr Vector4Int& operator=(const Vector4Int& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
	constexpr Vector4Int& operator=(Vector4Int&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

	constexpr Vector4Int& operator+=(I32 i) { x += i; y += i; z += i; w += i; return *this; }
	constexpr Vector4Int& operator-=(I32 i) { x -= i; y -= i; z -= i; w -= i; return *this; }
	constexpr Vector4Int& operator*=(I32 i) { x *= i; y *= i; z *= i; w *= i; return *this; }
	constexpr Vector4Int& operator/=(I32 i) { x /= i; y /= i; z /= i; w /= i; return *this; }
	constexpr Vector4Int& operator%=(I32 i) { x &= i; y %= i; z %= i; w %= i; return *this; }
	constexpr Vector4Int& operator+=(F32 f) { x += (I32)f; y += (I32)f; z += (I32)f; w += (I32)f; return *this; }
	constexpr Vector4Int& operator-=(F32 f) { x -= (I32)f; y -= (I32)f; z -= (I32)f; w -= (I32)f; return *this; }
	constexpr Vector4Int& operator*=(F32 f) { x *= (I32)f; y *= (I32)f; z *= (I32)f; w *= (I32)f; return *this; }
	constexpr Vector4Int& operator/=(F32 f) { x /= (I32)f; y /= (I32)f; z /= (I32)f; w /= (I32)f; return *this; }
	constexpr Vector4Int& operator+=(const Vector4Int& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	constexpr Vector4Int& operator-=(const Vector4Int& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	constexpr Vector4Int& operator*=(const Vector4Int& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	constexpr Vector4Int& operator/=(const Vector4Int& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
	constexpr Vector4Int& operator%=(const Vector4Int& v) { x %= v.x; y %= v.y; z %= v.z; w %= v.w; return *this; }

	constexpr Vector4Int operator+(I32 i) const { return { x + i, y + i, z + i, w + i }; }
	constexpr Vector4Int operator-(I32 i) const { return { x - i, y - i, z - i, w - i }; }
	constexpr Vector4Int operator*(I32 i) const { return { x * i, y * i, z * i, w * i }; }
	constexpr Vector4Int operator/(I32 i) const { return { x / i, y / i, z / i, w / i }; }
	constexpr Vector4Int operator%(I32 i) const { return { x % i, y % i, z % i, w % i }; }
	constexpr Vector4Int operator+(F32 f) const { return { (I32)(x + f), (I32)(y + f), (I32)(z + f), (I32)(w + f) }; }
	constexpr Vector4Int operator-(F32 f) const { return { (I32)(x - f), (I32)(y - f), (I32)(z - f), (I32)(w - f) }; }
	constexpr Vector4Int operator*(F32 f) const { return { (I32)(x * f), (I32)(y * f), (I32)(z * f), (I32)(w * f) }; }
	constexpr Vector4Int operator/(F32 f) const { return { (I32)(x / f), (I32)(y / f), (I32)(z / f), (I32)(w / f) }; }
	constexpr Vector4Int operator+(const Vector4Int& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
	constexpr Vector4Int operator-(const Vector4Int& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
	constexpr Vector4Int operator*(const Vector4Int& v) const { return { x * v.x, y * v.y, z * v.z, w * v.w }; }
	constexpr Vector4Int operator/(const Vector4Int& v) const { return { x / v.x, y / v.y, z / v.z, w / v.w }; }
	constexpr Vector4Int operator%(const Vector4Int& v) const { return { x % v.x, y % v.y, z % v.z, w % v.w }; }

	constexpr bool operator==(const Vector4Int& v) const { return !(x - v.x) && !(y - v.y) && (z - v.z) && !(w - v.w); }
	constexpr bool operator!=(const Vector4Int& v) const { return (x - v.x) || (y - v.y) || (z - v.z) || (w - v.w); }
	constexpr bool operator>(const Vector4Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<(const Vector4Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool operator>=(const Vector4Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
	constexpr bool operator<=(const Vector4Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
	constexpr bool IsZero() const { return !x && !y && !z && !w; }

	constexpr Vector4Int operator-() const { return { -x, -y, -z, -w }; }
	constexpr Vector4Int operator~() const { return { -x, -y, -z, -w }; }
	constexpr Vector4Int operator!() const { return { -x, -y, -z, -w }; }

	constexpr I32 SqrMagnitude() const { return x * x + y * y + z * z + w * w; }
	constexpr F32 Magnitude() const { return Math::Sqrt((F32)(x * x + y * y + z * z + w * w)); }
	constexpr I32 Dot(const Vector4Int& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
	constexpr I32 Dot(I32 vx, I32 vy, I32 vz, I32 vw) const { return x * vx + y * vy + z * vz + w * vw; }
	constexpr Vector4Int& Clamp(const Vector4Int& min, const Vector4Int& max)
	{
		x = Math::Clamp(x, min.x, max.x);
		y = Math::Clamp(y, min.y, max.y);
		z = Math::Clamp(z, min.z, max.z);
		w = Math::Clamp(w, min.w, max.w);
		return *this;
	}
	constexpr Vector4Int Clamped(const Vector4Int& min, const Vector4Int& max) const
	{
		return {
			Math::Clamp(x, min.x, max.x),
			Math::Clamp(y, min.y, max.y),
			Math::Clamp(z, min.z, max.z),
			Math::Clamp(w, min.w, max.w)
		};
	}
	constexpr Vector4Int& SetClosest(const Vector4Int& min, const Vector4Int& max)
	{
		x = Math::Closest(x, min.x, max.x);
		y = Math::Closest(y, min.y, max.y);
		z = Math::Closest(z, min.z, max.z);
		w = Math::Closest(w, min.w, max.w);
		return *this;
	}
	constexpr Vector4Int Closest(const Vector4Int& min, const Vector4Int& max) const
	{
		return {
			Math::Closest(x, min.x, max.x),
			Math::Closest(y, min.y, max.y),
			Math::Closest(z, min.z, max.z),
			Math::Closest(w, min.w, max.w)
		};
	}

	I32& operator[] (U64 i) { return (&x)[i]; }
	const I32& operator[] (U64 i) const { return (&x)[i]; }

	I32* Data() { return &x; }
	const I32* Data() const { return &x; }

#pragma region SwizzleFunctions
	constexpr Vector2Int xx() const { return { x, x }; }
	constexpr Vector2Int xy() const { return { x, y }; }
	constexpr Vector2Int xz() const { return { x, z }; }
	constexpr Vector2Int xw() const { return { x, w }; }
	constexpr Vector2Int yx() const { return { y, x }; }
	constexpr Vector2Int yy() const { return { y, y }; }
	constexpr Vector2Int yz() const { return { y, z }; }
	constexpr Vector2Int yw() const { return { y, w }; }
	constexpr Vector2Int zx() const { return { z, x }; }
	constexpr Vector2Int zy() const { return { z, y }; }
	constexpr Vector2Int zz() const { return { z, z }; }
	constexpr Vector2Int zw() const { return { z, w }; }
	constexpr Vector2Int wx() const { return { w, x }; }
	constexpr Vector2Int wy() const { return { w, y }; }
	constexpr Vector2Int wz() const { return { w, z }; }
	constexpr Vector2Int ww() const { return { w, w }; }
	constexpr Vector3Int xxx() const { return { x, x, x }; }
	constexpr Vector3Int xxy() const { return { x, x, y }; }
	constexpr Vector3Int xxz() const { return { x, x, z }; }
	constexpr Vector3Int xxw() const { return { x, x, w }; }
	constexpr Vector3Int xyx() const { return { x, y, x }; }
	constexpr Vector3Int xyy() const { return { x, y, y }; }
	constexpr Vector3Int xyz() const { return { x, y, z }; }
	constexpr Vector3Int xyw() const { return { x, y, w }; }
	constexpr Vector3Int xzx() const { return { x, z, x }; }
	constexpr Vector3Int xzy() const { return { x, z, y }; }
	constexpr Vector3Int xzz() const { return { x, z, z }; }
	constexpr Vector3Int xzw() const { return { x, z, w }; }
	constexpr Vector3Int xwx() const { return { x, w, x }; }
	constexpr Vector3Int xwy() const { return { x, w, y }; }
	constexpr Vector3Int xwz() const { return { x, w, z }; }
	constexpr Vector3Int xww() const { return { x, w, w }; }
	constexpr Vector3Int yxx() const { return { y, x, x }; }
	constexpr Vector3Int yxy() const { return { y, x, y }; }
	constexpr Vector3Int yxz() const { return { y, x, z }; }
	constexpr Vector3Int yxw() const { return { y, x, w }; }
	constexpr Vector3Int yyx() const { return { y, y, x }; }
	constexpr Vector3Int yyy() const { return { y, y, y }; }
	constexpr Vector3Int yyz() const { return { y, y, z }; }
	constexpr Vector3Int yyw() const { return { y, y, w }; }
	constexpr Vector3Int yzx() const { return { y, z, x }; }
	constexpr Vector3Int yzy() const { return { y, z, y }; }
	constexpr Vector3Int yzz() const { return { y, z, z }; }
	constexpr Vector3Int yzw() const { return { y, z, w }; }
	constexpr Vector3Int ywx() const { return { y, w, x }; }
	constexpr Vector3Int ywy() const { return { y, w, y }; }
	constexpr Vector3Int ywz() const { return { y, w, z }; }
	constexpr Vector3Int yww() const { return { y, w, w }; }
	constexpr Vector3Int zxx() const { return { z, x, x }; }
	constexpr Vector3Int zxy() const { return { z, x, y }; }
	constexpr Vector3Int zxz() const { return { z, x, z }; }
	constexpr Vector3Int zxw() const { return { z, x, w }; }
	constexpr Vector3Int zyx() const { return { z, y, x }; }
	constexpr Vector3Int zyy() const { return { z, y, y }; }
	constexpr Vector3Int zyz() const { return { z, y, z }; }
	constexpr Vector3Int zyw() const { return { z, y, w }; }
	constexpr Vector3Int zzx() const { return { z, z, x }; }
	constexpr Vector3Int zzy() const { return { z, z, y }; }
	constexpr Vector3Int zzz() const { return { z, z, z }; }
	constexpr Vector3Int zzw() const { return { z, z, w }; }
	constexpr Vector3Int zwx() const { return { z, w, x }; }
	constexpr Vector3Int zwy() const { return { z, w, y }; }
	constexpr Vector3Int zwz() const { return { z, w, z }; }
	constexpr Vector3Int zww() const { return { z, w, w }; }
	constexpr Vector3Int wxx() const { return { w, x, x }; }
	constexpr Vector3Int wxy() const { return { w, x, y }; }
	constexpr Vector3Int wxz() const { return { w, x, z }; }
	constexpr Vector3Int wxw() const { return { w, x, w }; }
	constexpr Vector3Int wyx() const { return { w, y, x }; }
	constexpr Vector3Int wyy() const { return { w, y, y }; }
	constexpr Vector3Int wyz() const { return { w, y, z }; }
	constexpr Vector3Int wyw() const { return { w, y, w }; }
	constexpr Vector3Int wzx() const { return { w, z, x }; }
	constexpr Vector3Int wzy() const { return { w, z, y }; }
	constexpr Vector3Int wzz() const { return { w, z, z }; }
	constexpr Vector3Int wzw() const { return { w, z, w }; }
	constexpr Vector3Int wwx() const { return { w, w, x }; }
	constexpr Vector3Int wwy() const { return { w, w, y }; }
	constexpr Vector3Int wwz() const { return { w, w, z }; }
	constexpr Vector3Int www() const { return { w, w, w }; }
	constexpr Vector4Int xxxx() const { return { x, x, x, x }; }
	constexpr Vector4Int xxxy() const { return { x, x, x, y }; }
	constexpr Vector4Int xxxz() const { return { x, x, x, z }; }
	constexpr Vector4Int xxxw() const { return { x, x, x, w }; }
	constexpr Vector4Int xxyx() const { return { x, x, y, x }; }
	constexpr Vector4Int xxyy() const { return { x, x, y, y }; }
	constexpr Vector4Int xxyz() const { return { x, x, y, z }; }
	constexpr Vector4Int xxyw() const { return { x, x, y, w }; }
	constexpr Vector4Int xxzx() const { return { x, x, z, x }; }
	constexpr Vector4Int xxzy() const { return { x, x, z, y }; }
	constexpr Vector4Int xxzz() const { return { x, x, z, z }; }
	constexpr Vector4Int xxzw() const { return { x, x, z, w }; }
	constexpr Vector4Int xxwx() const { return { x, x, w, x }; }
	constexpr Vector4Int xxwy() const { return { x, x, w, y }; }
	constexpr Vector4Int xxwz() const { return { x, x, w, z }; }
	constexpr Vector4Int xxww() const { return { x, x, w, w }; }
	constexpr Vector4Int xyxx() const { return { x, y, x, x }; }
	constexpr Vector4Int xyxy() const { return { x, y, x, y }; }
	constexpr Vector4Int xyxz() const { return { x, y, x, z }; }
	constexpr Vector4Int xyxw() const { return { x, y, x, w }; }
	constexpr Vector4Int xyyx() const { return { x, y, y, x }; }
	constexpr Vector4Int xyyy() const { return { x, y, y, y }; }
	constexpr Vector4Int xyyz() const { return { x, y, y, z }; }
	constexpr Vector4Int xyyw() const { return { x, y, y, w }; }
	constexpr Vector4Int xyzx() const { return { x, y, z, x }; }
	constexpr Vector4Int xyzy() const { return { x, y, z, y }; }
	constexpr Vector4Int xyzz() const { return { x, y, z, z }; }
	constexpr Vector4Int xyzw() const { return { x, y, z, w }; }
	constexpr Vector4Int xywx() const { return { x, y, w, x }; }
	constexpr Vector4Int xywy() const { return { x, y, w, y }; }
	constexpr Vector4Int xywz() const { return { x, y, w, z }; }
	constexpr Vector4Int xyww() const { return { x, y, w, w }; }
	constexpr Vector4Int xzxx() const { return { x, z, x, x }; }
	constexpr Vector4Int xzxy() const { return { x, z, x, y }; }
	constexpr Vector4Int xzxz() const { return { x, z, x, z }; }
	constexpr Vector4Int xzxw() const { return { x, z, x, w }; }
	constexpr Vector4Int xzyx() const { return { x, z, y, x }; }
	constexpr Vector4Int xzyy() const { return { x, z, y, y }; }
	constexpr Vector4Int xzyz() const { return { x, z, y, z }; }
	constexpr Vector4Int xzyw() const { return { x, z, y, w }; }
	constexpr Vector4Int xzzx() const { return { x, z, z, x }; }
	constexpr Vector4Int xzzy() const { return { x, z, z, y }; }
	constexpr Vector4Int xzzz() const { return { x, z, z, z }; }
	constexpr Vector4Int xzzw() const { return { x, z, z, w }; }
	constexpr Vector4Int xzwx() const { return { x, z, w, x }; }
	constexpr Vector4Int xzwy() const { return { x, z, w, y }; }
	constexpr Vector4Int xzwz() const { return { x, z, w, z }; }
	constexpr Vector4Int xzww() const { return { x, z, w, w }; }
	constexpr Vector4Int xwxx() const { return { x, w, x, x }; }
	constexpr Vector4Int xwxy() const { return { x, w, x, y }; }
	constexpr Vector4Int xwxz() const { return { x, w, x, z }; }
	constexpr Vector4Int xwxw() const { return { x, w, x, w }; }
	constexpr Vector4Int xwyx() const { return { x, w, y, x }; }
	constexpr Vector4Int xwyy() const { return { x, w, y, y }; }
	constexpr Vector4Int xwyz() const { return { x, w, y, z }; }
	constexpr Vector4Int xwyw() const { return { x, w, y, w }; }
	constexpr Vector4Int xwzx() const { return { x, w, z, x }; }
	constexpr Vector4Int xwzy() const { return { x, w, z, y }; }
	constexpr Vector4Int xwzz() const { return { x, w, z, z }; }
	constexpr Vector4Int xwzw() const { return { x, w, z, w }; }
	constexpr Vector4Int xwwx() const { return { x, w, w, x }; }
	constexpr Vector4Int xwwy() const { return { x, w, w, y }; }
	constexpr Vector4Int xwwz() const { return { x, w, w, z }; }
	constexpr Vector4Int xwww() const { return { x, w, w, w }; }
	constexpr Vector4Int yxxx() const { return { y, x, x, x }; }
	constexpr Vector4Int yxxy() const { return { y, x, x, y }; }
	constexpr Vector4Int yxxz() const { return { y, x, x, z }; }
	constexpr Vector4Int yxxw() const { return { y, x, x, w }; }
	constexpr Vector4Int yxyx() const { return { y, x, y, x }; }
	constexpr Vector4Int yxyy() const { return { y, x, y, y }; }
	constexpr Vector4Int yxyz() const { return { y, x, y, z }; }
	constexpr Vector4Int yxyw() const { return { y, x, y, w }; }
	constexpr Vector4Int yxzx() const { return { y, x, z, x }; }
	constexpr Vector4Int yxzy() const { return { y, x, z, y }; }
	constexpr Vector4Int yxzz() const { return { y, x, z, z }; }
	constexpr Vector4Int yxzw() const { return { y, x, z, w }; }
	constexpr Vector4Int yxwx() const { return { y, x, w, x }; }
	constexpr Vector4Int yxwy() const { return { y, x, w, y }; }
	constexpr Vector4Int yxwz() const { return { y, x, w, z }; }
	constexpr Vector4Int yxww() const { return { y, x, w, w }; }
	constexpr Vector4Int yyxx() const { return { y, y, x, x }; }
	constexpr Vector4Int yyxy() const { return { y, y, x, y }; }
	constexpr Vector4Int yyxz() const { return { y, y, x, z }; }
	constexpr Vector4Int yyxw() const { return { y, y, x, w }; }
	constexpr Vector4Int yyyx() const { return { y, y, y, x }; }
	constexpr Vector4Int yyyy() const { return { y, y, y, y }; }
	constexpr Vector4Int yyyz() const { return { y, y, y, z }; }
	constexpr Vector4Int yyyw() const { return { y, y, y, w }; }
	constexpr Vector4Int yyzx() const { return { y, y, z, x }; }
	constexpr Vector4Int yyzy() const { return { y, y, z, y }; }
	constexpr Vector4Int yyzz() const { return { y, y, z, z }; }
	constexpr Vector4Int yyzw() const { return { y, y, z, w }; }
	constexpr Vector4Int yywx() const { return { y, y, w, x }; }
	constexpr Vector4Int yywy() const { return { y, y, w, y }; }
	constexpr Vector4Int yywz() const { return { y, y, w, z }; }
	constexpr Vector4Int yyww() const { return { y, y, w, w }; }
	constexpr Vector4Int yzxx() const { return { y, z, x, x }; }
	constexpr Vector4Int yzxy() const { return { y, z, x, y }; }
	constexpr Vector4Int yzxz() const { return { y, z, x, z }; }
	constexpr Vector4Int yzxw() const { return { y, z, x, w }; }
	constexpr Vector4Int yzyx() const { return { y, z, y, x }; }
	constexpr Vector4Int yzyy() const { return { y, z, y, y }; }
	constexpr Vector4Int yzyz() const { return { y, z, y, z }; }
	constexpr Vector4Int yzyw() const { return { y, z, y, w }; }
	constexpr Vector4Int yzzx() const { return { y, z, z, x }; }
	constexpr Vector4Int yzzy() const { return { y, z, z, y }; }
	constexpr Vector4Int yzzz() const { return { y, z, z, z }; }
	constexpr Vector4Int yzzw() const { return { y, z, z, w }; }
	constexpr Vector4Int yzwx() const { return { y, z, w, x }; }
	constexpr Vector4Int yzwy() const { return { y, z, w, y }; }
	constexpr Vector4Int yzwz() const { return { y, z, w, z }; }
	constexpr Vector4Int yzww() const { return { y, z, w, w }; }
	constexpr Vector4Int ywxx() const { return { y, w, x, x }; }
	constexpr Vector4Int ywxy() const { return { y, w, x, y }; }
	constexpr Vector4Int ywxz() const { return { y, w, x, z }; }
	constexpr Vector4Int ywxw() const { return { y, w, x, w }; }
	constexpr Vector4Int ywyx() const { return { y, w, y, x }; }
	constexpr Vector4Int ywyy() const { return { y, w, y, y }; }
	constexpr Vector4Int ywyz() const { return { y, w, y, z }; }
	constexpr Vector4Int ywyw() const { return { y, w, y, w }; }
	constexpr Vector4Int ywzx() const { return { y, w, z, x }; }
	constexpr Vector4Int ywzy() const { return { y, w, z, y }; }
	constexpr Vector4Int ywzz() const { return { y, w, z, z }; }
	constexpr Vector4Int ywzw() const { return { y, w, z, w }; }
	constexpr Vector4Int ywwx() const { return { y, w, w, x }; }
	constexpr Vector4Int ywwy() const { return { y, w, w, y }; }
	constexpr Vector4Int ywwz() const { return { y, w, w, z }; }
	constexpr Vector4Int ywww() const { return { y, w, w, w }; }
	constexpr Vector4Int zxxx() const { return { z, x, x, x }; }
	constexpr Vector4Int zxxy() const { return { z, x, x, y }; }
	constexpr Vector4Int zxxz() const { return { z, x, x, z }; }
	constexpr Vector4Int zxxw() const { return { z, x, x, w }; }
	constexpr Vector4Int zxyx() const { return { z, x, y, x }; }
	constexpr Vector4Int zxyy() const { return { z, x, y, y }; }
	constexpr Vector4Int zxyz() const { return { z, x, y, z }; }
	constexpr Vector4Int zxyw() const { return { z, x, y, w }; }
	constexpr Vector4Int zxzx() const { return { z, x, z, x }; }
	constexpr Vector4Int zxzy() const { return { z, x, z, y }; }
	constexpr Vector4Int zxzz() const { return { z, x, z, z }; }
	constexpr Vector4Int zxzw() const { return { z, x, z, w }; }
	constexpr Vector4Int zxwx() const { return { z, x, w, x }; }
	constexpr Vector4Int zxwy() const { return { z, x, w, y }; }
	constexpr Vector4Int zxwz() const { return { z, x, w, z }; }
	constexpr Vector4Int zxww() const { return { z, x, w, w }; }
	constexpr Vector4Int zyxx() const { return { z, y, x, x }; }
	constexpr Vector4Int zyxy() const { return { z, y, x, y }; }
	constexpr Vector4Int zyxz() const { return { z, y, x, z }; }
	constexpr Vector4Int zyxw() const { return { z, y, x, w }; }
	constexpr Vector4Int zyyx() const { return { z, y, y, x }; }
	constexpr Vector4Int zyyy() const { return { z, y, y, y }; }
	constexpr Vector4Int zyyz() const { return { z, y, y, z }; }
	constexpr Vector4Int zyyw() const { return { z, y, y, w }; }
	constexpr Vector4Int zyzx() const { return { z, y, z, x }; }
	constexpr Vector4Int zyzy() const { return { z, y, z, y }; }
	constexpr Vector4Int zyzz() const { return { z, y, z, z }; }
	constexpr Vector4Int zyzw() const { return { z, y, z, w }; }
	constexpr Vector4Int zywx() const { return { z, y, w, x }; }
	constexpr Vector4Int zywy() const { return { z, y, w, y }; }
	constexpr Vector4Int zywz() const { return { z, y, w, z }; }
	constexpr Vector4Int zyww() const { return { z, y, w, w }; }
	constexpr Vector4Int zzxx() const { return { z, z, x, x }; }
	constexpr Vector4Int zzxy() const { return { z, z, x, y }; }
	constexpr Vector4Int zzxz() const { return { z, z, x, z }; }
	constexpr Vector4Int zzxw() const { return { z, z, x, w }; }
	constexpr Vector4Int zzyx() const { return { z, z, y, x }; }
	constexpr Vector4Int zzyy() const { return { z, z, y, y }; }
	constexpr Vector4Int zzyz() const { return { z, z, y, z }; }
	constexpr Vector4Int zzyw() const { return { z, z, y, w }; }
	constexpr Vector4Int zzzx() const { return { z, z, z, x }; }
	constexpr Vector4Int zzzy() const { return { z, z, z, y }; }
	constexpr Vector4Int zzzz() const { return { z, z, z, z }; }
	constexpr Vector4Int zzzw() const { return { z, z, z, w }; }
	constexpr Vector4Int zzwx() const { return { z, z, w, x }; }
	constexpr Vector4Int zzwy() const { return { z, z, w, y }; }
	constexpr Vector4Int zzwz() const { return { z, z, w, z }; }
	constexpr Vector4Int zzww() const { return { z, z, w, w }; }
	constexpr Vector4Int zwxx() const { return { z, w, x, x }; }
	constexpr Vector4Int zwxy() const { return { z, w, x, y }; }
	constexpr Vector4Int zwxz() const { return { z, w, x, z }; }
	constexpr Vector4Int zwxw() const { return { z, w, x, w }; }
	constexpr Vector4Int zwyx() const { return { z, w, y, x }; }
	constexpr Vector4Int zwyy() const { return { z, w, y, y }; }
	constexpr Vector4Int zwyz() const { return { z, w, y, z }; }
	constexpr Vector4Int zwyw() const { return { z, w, y, w }; }
	constexpr Vector4Int zwzx() const { return { z, w, z, x }; }
	constexpr Vector4Int zwzy() const { return { z, w, z, y }; }
	constexpr Vector4Int zwzz() const { return { z, w, z, z }; }
	constexpr Vector4Int zwzw() const { return { z, w, z, w }; }
	constexpr Vector4Int zwwx() const { return { z, w, w, x }; }
	constexpr Vector4Int zwwy() const { return { z, w, w, y }; }
	constexpr Vector4Int zwwz() const { return { z, w, w, z }; }
	constexpr Vector4Int zwww() const { return { z, w, w, w }; }
	constexpr Vector4Int wxxx() const { return { w, x, x, x }; }
	constexpr Vector4Int wxxy() const { return { w, x, x, y }; }
	constexpr Vector4Int wxxz() const { return { w, x, x, z }; }
	constexpr Vector4Int wxxw() const { return { w, x, x, w }; }
	constexpr Vector4Int wxyx() const { return { w, x, y, x }; }
	constexpr Vector4Int wxyy() const { return { w, x, y, y }; }
	constexpr Vector4Int wxyz() const { return { w, x, y, z }; }
	constexpr Vector4Int wxyw() const { return { w, x, y, w }; }
	constexpr Vector4Int wxzx() const { return { w, x, z, x }; }
	constexpr Vector4Int wxzy() const { return { w, x, z, y }; }
	constexpr Vector4Int wxzz() const { return { w, x, z, z }; }
	constexpr Vector4Int wxzw() const { return { w, x, z, w }; }
	constexpr Vector4Int wxwx() const { return { w, x, w, x }; }
	constexpr Vector4Int wxwy() const { return { w, x, w, y }; }
	constexpr Vector4Int wxwz() const { return { w, x, w, z }; }
	constexpr Vector4Int wxww() const { return { w, x, w, w }; }
	constexpr Vector4Int wyxx() const { return { w, y, x, x }; }
	constexpr Vector4Int wyxy() const { return { w, y, x, y }; }
	constexpr Vector4Int wyxz() const { return { w, y, x, z }; }
	constexpr Vector4Int wyxw() const { return { w, y, x, w }; }
	constexpr Vector4Int wyyx() const { return { w, y, y, x }; }
	constexpr Vector4Int wyyy() const { return { w, y, y, y }; }
	constexpr Vector4Int wyyz() const { return { w, y, y, z }; }
	constexpr Vector4Int wyyw() const { return { w, y, y, w }; }
	constexpr Vector4Int wyzx() const { return { w, y, z, x }; }
	constexpr Vector4Int wyzy() const { return { w, y, z, y }; }
	constexpr Vector4Int wyzz() const { return { w, y, z, z }; }
	constexpr Vector4Int wyzw() const { return { w, y, z, w }; }
	constexpr Vector4Int wywx() const { return { w, y, w, x }; }
	constexpr Vector4Int wywy() const { return { w, y, w, y }; }
	constexpr Vector4Int wywz() const { return { w, y, w, z }; }
	constexpr Vector4Int wyww() const { return { w, y, w, w }; }
	constexpr Vector4Int wzxx() const { return { w, z, x, x }; }
	constexpr Vector4Int wzxy() const { return { w, z, x, y }; }
	constexpr Vector4Int wzxz() const { return { w, z, x, z }; }
	constexpr Vector4Int wzxw() const { return { w, z, x, w }; }
	constexpr Vector4Int wzyx() const { return { w, z, y, x }; }
	constexpr Vector4Int wzyy() const { return { w, z, y, y }; }
	constexpr Vector4Int wzyz() const { return { w, z, y, z }; }
	constexpr Vector4Int wzyw() const { return { w, z, y, w }; }
	constexpr Vector4Int wzzx() const { return { w, z, z, x }; }
	constexpr Vector4Int wzzy() const { return { w, z, z, y }; }
	constexpr Vector4Int wzzz() const { return { w, z, z, z }; }
	constexpr Vector4Int wzzw() const { return { w, z, z, w }; }
	constexpr Vector4Int wzwx() const { return { w, z, w, x }; }
	constexpr Vector4Int wzwy() const { return { w, z, w, y }; }
	constexpr Vector4Int wzwz() const { return { w, z, w, z }; }
	constexpr Vector4Int wzww() const { return { w, z, w, w }; }
	constexpr Vector4Int wwxx() const { return { w, w, x, x }; }
	constexpr Vector4Int wwxy() const { return { w, w, x, y }; }
	constexpr Vector4Int wwxz() const { return { w, w, x, z }; }
	constexpr Vector4Int wwxw() const { return { w, w, x, w }; }
	constexpr Vector4Int wwyx() const { return { w, w, y, x }; }
	constexpr Vector4Int wwyy() const { return { w, w, y, y }; }
	constexpr Vector4Int wwyz() const { return { w, w, y, z }; }
	constexpr Vector4Int wwyw() const { return { w, w, y, w }; }
	constexpr Vector4Int wwzx() const { return { w, w, z, x }; }
	constexpr Vector4Int wwzy() const { return { w, w, z, y }; }
	constexpr Vector4Int wwzz() const { return { w, w, z, z }; }
	constexpr Vector4Int wwzw() const { return { w, w, z, w }; }
	constexpr Vector4Int wwwx() const { return { w, w, w, x }; }
	constexpr Vector4Int wwwy() const { return { w, w, w, y }; }
	constexpr Vector4Int wwwz() const { return { w, w, w, z }; }
	constexpr Vector4Int wwww() const { return { w, w, w, w }; }
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
};

export constexpr Vector4Int operator*(F32 f, const Vector4Int& v) { return { (I32)(f * v.x), (I32)(f * v.y), (I32)(f * v.z), (I32)(f * v.w) }; }
export constexpr Vector4Int operator/(F32 f, const Vector4Int& v) { return { (I32)(f / v.x), (I32)(f / v.y), (I32)(f / v.z), (I32)(f / v.w) }; }
export constexpr Vector4Int operator*(I32 i, const Vector4Int& v) { return { i * v.x, i * v.y, i * v.z, i * v.w }; }
export constexpr Vector4Int operator/(I32 i, const Vector4Int& v) { return { i / v.x, i / v.y, i / v.z, i / v.w }; }

inline constexpr Vector2::operator Vector3() const { return Vector3{ x, y, 0.0f }; }
inline constexpr Vector2::operator Vector4() const { return Vector4{ x, y, 0.0f, 0.0f }; }
inline constexpr Vector2::operator Vector2Int() const { return Vector2Int{ (I32)x, (I32)y }; }
inline constexpr Vector2::operator Vector3Int() const { return Vector3Int{ (I32)x, (I32)y, 0 }; }
inline constexpr Vector2::operator Vector4Int() const { return Vector4Int{ (I32)x, (I32)y, 0, 0 }; }

inline constexpr Vector3::operator Vector2() const { return Vector2{ x, y }; }
inline constexpr Vector3::operator Vector4() const { return Vector4{ x, y, z, 0.0f }; }
inline constexpr Vector3::operator Vector2Int() const { return Vector2Int{ (I32)x, (I32)y }; }
inline constexpr Vector3::operator Vector3Int() const { return Vector3Int{ (I32)x, (I32)y, (I32)z }; }
inline constexpr Vector3::operator Vector4Int() const { return Vector4Int{ (I32)x, (I32)y, (I32)z, 0 }; }

inline constexpr Vector4::operator Vector2() const { return Vector2{ x, y }; }
inline constexpr Vector4::operator Vector3() const { return Vector3{ x, y, z }; }
inline constexpr Vector4::operator Vector2Int() const { return Vector2Int{ (I32)x, (I32)y }; }
inline constexpr Vector4::operator Vector3Int() const { return Vector3Int{ (I32)x, (I32)y, (I32)z }; }
inline constexpr Vector4::operator Vector4Int() const { return Vector4Int{ (I32)x, (I32)y, (I32)z, (I32)w }; }

inline constexpr Vector2Int::operator Vector2() const { return Vector2{ (F32)x, (F32)y }; }
inline constexpr Vector2Int::operator Vector3() const { return Vector3{ (F32)x, (F32)y, 0.0f }; }
inline constexpr Vector2Int::operator Vector4() const { return Vector4{ (F32)x, (F32)y, 0.0f, 0.0f }; }
inline constexpr Vector2Int::operator Vector3Int() const { return Vector3Int{ x, y, 0 }; }
inline constexpr Vector2Int::operator Vector4Int() const { return Vector4Int{ x, y, 0, 0 }; }

inline constexpr Vector3Int::operator Vector2() const { return Vector2{ (F32)x, (F32)y }; }
inline constexpr Vector3Int::operator Vector3() const { return Vector3{ (F32)x, (F32)y, (F32)z }; }
inline constexpr Vector3Int::operator Vector4() const { return Vector4{ (F32)x, (F32)y, (F32)z, 0 }; }
inline constexpr Vector3Int::operator Vector2Int() const { return Vector2Int{ x, y }; }
inline constexpr Vector3Int::operator Vector4Int() const { return Vector4Int{ x, y, z, 0 }; }

inline constexpr Vector4Int::operator Vector2() const { return Vector2{ (F32)x, (F32)y }; }
inline constexpr Vector4Int::operator Vector3() const { return Vector3{ (F32)x, (F32)y, (F32)z }; }
inline constexpr Vector4Int::operator Vector4() const { return Vector4{ (F32)x, (F32)y, (F32)z, (F32)w }; }
inline constexpr Vector4Int::operator Vector2Int() const { return Vector2Int{ x, y }; }
inline constexpr Vector4Int::operator Vector3Int() const { return Vector3Int{ x, y, z }; }

export struct NH_API Matrix2
{
	constexpr Matrix2() : a(1.0f, 0.0f), b(0.0f, 1.0f) {}
	constexpr Matrix2(F32 ax, F32 ay, F32 bx, F32 by) : a(ax, ay), b(bx, by) {}
	constexpr Matrix2(const Vector2& v) : a(v.x, 0.0f), b(0.0f, v.y) {}
	constexpr Matrix2(const Vector2& a, const Vector2& b) : a(a), b(b) {}
	constexpr Matrix2(Vector2&& a, Vector2&& b) noexcept : a(a), b(b) {}
	constexpr Matrix2(const Matrix2& m) : a(m.a), b(m.b) {}
	constexpr Matrix2(Matrix2&& m) noexcept : a(m.a), b(m.b) {}

	constexpr Matrix2& operator= (const Matrix2& m) { a = m.a; b = m.b; return *this; }
	constexpr Matrix2& operator= (Matrix2&& m) noexcept { a = m.a; b = m.b; return *this; }

	constexpr Matrix2& operator+= (const Matrix2& m) { a += m.a; b += m.b; return *this; }
	constexpr Matrix2& operator-= (const Matrix2& m) { a -= m.a; b -= m.b; return *this; }
	constexpr Matrix2& operator*= (const Matrix2& m)
	{
		a.x = a.x * m.a.x + b.x * m.a.y;
		a.y = a.y * m.a.x + b.y * m.a.y;
		b.x = a.x * m.b.x + b.x * m.b.y;
		b.y = a.y * m.b.x + b.y * m.b.y;

		return *this;
	}

	constexpr Matrix2 operator+(const Matrix2& m) const { return { a + m.a, b + m.b }; }
	constexpr Matrix2 operator-(const Matrix2& m) const { return { a - m.a, b - m.b }; }
	constexpr Matrix2 operator*(const Matrix2& m) const
	{
		return {
			a.x * m.a.x + b.x * m.a.y,
			a.y * m.a.x + b.y * m.a.y,
			a.x * m.b.x + b.x * m.b.y,
			a.y * m.b.x + b.y * m.b.y,
		};
	}
	constexpr Vector2 operator*(const Vector2& v) const
	{
		return {
			a.x * v.x + b.x * v.y,
			a.y * v.x + b.y * v.y,
		};
	}

	constexpr Vector2 Solve(const Vector2& v) const
	{
		F32 det = a.x * b.y - b.x * a.y;
		if (!Math::IsZero(det)) { det = 1.0f / det; }
		return { det * (b.y * b.x - b.x * b.y), det * (a.x * b.y - a.y * b.x) };
	}

	constexpr Matrix2 Inverse() const
	{
		F32 determinant = a.x * b.y - b.x * a.y;

		if (Math::IsZero(determinant)) { return Matrix2{ }; }
		F32 f = 1.0f / determinant;

		return {
			 b.y * f, -a.y * f,
			-b.x * f,  a.x * f
		};
	}
	constexpr Matrix2& Inversed()
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
	constexpr Matrix2 Transpose() const
	{
		return {
			a.x, b.x,
			a.y, b.y
		};
	}
	constexpr Matrix2& Transposed()
	{
		F32 bx = a.y;

		a.y = b.x;
		b.x = bx;

		return *this;
	}

	constexpr Matrix2 operator-() const { return { -a, -b }; }
	constexpr Matrix2 operator~() const { return { -a, -b }; }
	constexpr Matrix2 operator!() const { return { -a, -b }; }

	constexpr bool operator==(const Matrix2& m) const { return a == m.a && b == m.b; }
	constexpr bool operator!=(const Matrix2& m) const { return a != m.a || b != m.b; }

	const Vector2& operator[] (U8 i) const { return (&a)[i]; }
	Vector2& operator[] (U8 i) { return (&a)[i]; }

	F32* Data() { return a.Data(); }
	const F32* Data() const { return a.Data(); }

public:
	Vector2 a, b; //Columns
};

export struct NH_API Matrix3
{
	constexpr Matrix3() : a(1.0f, 0.0f, 0.0f), b(0.0f, 1.0f, 0.0f), c(0.0f, 0.0f, 1.0f) {}
	constexpr Matrix3(F32 ax, F32 ay, F32 az, F32 bx, F32 by, F32 bz, F32 cx, F32 cy, F32 cz) : a(ax, ay, az), b(bx, by, bz), c(cx, cy, cz) {}
	constexpr Matrix3(const Vector3& v) : a(v.x, 0.0f, 0.0f), b(0.0f, v.y, 0.0f), c(0.0f, 0.0f, v.z) {}
	constexpr Matrix3(const Vector3& a, const Vector3& b, const Vector3& c) : a(a), b(b), c(c) {}
	constexpr Matrix3(Vector3&& v1, Vector3&& v2, Vector3&& v3) noexcept : a(v1), b(v2), c(v3) {}
	constexpr Matrix3(const Matrix3& m) : a(m.a), b(m.b), c(m.c) {}
	constexpr Matrix3(Matrix3&& m) noexcept : a(m.a), b(m.b), c(m.c) {}
	constexpr Matrix3(const Vector2& position, const F32& rotation, const Vector2& scale)
	{
		F32 cos = Math::Cos(rotation * DEG_TO_RAD_F);
		F32 sin = Math::Sin(rotation * DEG_TO_RAD_F);
		a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
		a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
		a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
	}
	constexpr Matrix3(const Vector2& position, const Quaternion2& rotation, const Vector2& scale);

	constexpr Matrix3& operator= (const Matrix3& m) { a = m.a; b = m.b; c = m.c; return *this; }
	constexpr Matrix3& operator= (Matrix3&& m) noexcept { a = m.a; b = m.b; c = m.c; return *this; }
	constexpr void Set(const Vector2& position, const Quaternion2& rotation, const Vector2& scale);
	constexpr void Set(const Vector2& position, const F32& rotation, const Vector2& scale)
	{
		F32 cos = Math::Cos(rotation * DEG_TO_RAD_F);
		F32 sin = Math::Sin(rotation * DEG_TO_RAD_F);
		a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
		a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
		a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
	}

	constexpr Matrix3& operator+= (const Matrix3& m) { a += m.a; b += m.b; c += m.c; return *this; }
	constexpr Matrix3& operator-= (const Matrix3& m) { a -= m.a; b -= m.b; c -= m.c; return *this; }
	constexpr Matrix3& operator*= (const Matrix3& m)
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

	constexpr Matrix3 operator+(const Matrix3& m) const { return { a + m.a, b + m.b, c + m.c }; }
	constexpr Matrix3 operator-(const Matrix3& m) const { return { a - m.a, b - m.b, c - m.c }; }
	constexpr Matrix3 operator*(const Matrix3& m) const
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
	constexpr Vector3 operator*(const Vector3& v) const
	{
		return {
			a.x * v.x + b.x * v.y + c.x * v.z,
			a.y * v.x + b.y * v.y + c.y * v.z,
			a.z * v.x + b.z * v.y + c.z * v.z
		};
	}

	constexpr Matrix3 Inverse() const
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
	constexpr Matrix3& Inversed()
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
	constexpr Matrix3 Transpose() const
	{
		return {
			a.x, b.x, c.x,
			a.y, b.y, c.y,
			a.z, b.z, c.z
		};
	}
	constexpr Matrix3& Transposed()
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

	constexpr Matrix3 operator-() const { return { -a, -b, -c }; }
	constexpr Matrix3 operator~() const { return { -a, -b, -c }; }
	constexpr Matrix3 operator!() const { return { -a, -b, -c }; }

	constexpr bool operator==(const Matrix3& m) const { return a == m.a && b == m.b && c == m.c; }
	constexpr bool operator!=(const Matrix3& m) const { return a != m.a || b != m.b || c != m.c; }

	const Vector3& operator[] (U8 i) const { return (&a)[i]; }
	Vector3& operator[] (U8 i) { return (&a)[i]; }

	F32* Data() { return a.Data(); }
	const F32* Data() const { return a.Data(); }

public:
	Vector3 a, b, c; //Columns
};

export struct NH_API Matrix4
{
	constexpr Matrix4() : a(1.0f, 0.0f, 0.0f, 0.0f), b(0.0f, 1.0f, 0.0f, 0.0f), c(0.0f, 0.0f, 1.0f, 0.0f), d(0.0f, 0.0f, 0.0f, 1.0f) {}
	constexpr Matrix4(F32 ax, F32 ay, F32 az, F32 aw, F32 bx, F32 by, F32 bz, F32 bw, F32 cx, F32 cy, F32 cz, F32 cw, F32 dx, F32 dy, F32 dz, F32 dw) :
		a(ax, ay, az, aw), b(bx, by, bz, bw), c(cx, cy, cz, cw), d(dx, dy, dz, dw)
	{
	}
	constexpr Matrix4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d) : a(a), b(b), c(c), d(d) {}
	constexpr Matrix4(Vector4&& a, Vector4&& b, Vector4&& c, Vector4&& d) noexcept : a(a), b(b), c(c), d(d) {}
	constexpr Matrix4(const Matrix4& m) : a(m.a), b(m.b), c(m.c), d(m.d) {}
	constexpr Matrix4(const Matrix3& m) : a(m.a.x, m.a.y, 0.0f, m.a.z), b(m.b.x, m.b.y, 0.0f, m.b.z), c(0.0f, 0.0f, 1.0f, m.b.z), d(m.c.x, m.c.y, 0.0f, 1.0f) {}
	constexpr Matrix4(Matrix4&& m) noexcept : a(m.a), b(m.b), c(m.c), d(m.d) {}
	constexpr Matrix4(const Vector3& position, const Vector3& rotation = (0.0f), const Vector3& scale = (1.0f));
	constexpr Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale = (1.0f));

	constexpr Matrix4& operator= (const Matrix4& m) { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
	constexpr Matrix4& operator= (Matrix4&& m) noexcept { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
	constexpr Matrix4& operator+= (const Matrix4& m) { a += m.a; b += m.b; c += m.c; d += m.d; return *this; }
	constexpr Matrix4& operator-= (const Matrix4& m) { a -= m.a; b -= m.b; c -= m.c; d -= m.d; return *this; }
	constexpr Matrix4& operator*= (const Matrix4& m)
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

	constexpr Matrix4 operator+(const Matrix4& m) const { return { a + m.a, b + m.b, c + m.c, d + m.d }; }
	constexpr Matrix4 operator-(const Matrix4& m) const { return { a - m.a, b - m.b, c - m.c, d + m.d }; }
	constexpr Matrix4 operator*(const Matrix4& m) const
	{
		//#if defined NH_AVX
		//
		//#elif defined NH_SSE || defined NH_SSE2 //TODO: Failure
		//	M128 l, r0, r1, r2, r3, v0, v1, v2, v3;
		//	Vector4 f0, f1, f2, f3;
		//
		//	l = _mm_load_ps(a.Data());
		//	r0 = _mm_load_ps(m.a.Data());
		//	r1 = _mm_load_ps(m.b.Data());
		//	r2 = _mm_load_ps(m.c.Data());
		//	r3 = _mm_load_ps(m.d.Data());
		//
		//	v0 = _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 0, 0)), l);
		//	v1 = _mm_mul_ps(_mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 0, 0)), l);
		//	v2 = _mm_mul_ps(_mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 0, 0)), l);
		//	v3 = _mm_mul_ps(_mm_shuffle_ps(r0, r3, _MM_SHUFFLE(0, 0, 0, 0)), l);
		//
		//	l = _mm_load_ps(m.b.Data());
		//	v0 = _mm_add_ps(v0, _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(1, 1, 1, 1)), l));
		//	v1 = _mm_add_ps(v1, _mm_mul_ps(_mm_shuffle_ps(r1, r1, _MM_SHUFFLE(1, 1, 1, 1)), l));
		//	v2 = _mm_add_ps(v2, _mm_mul_ps(_mm_shuffle_ps(r2, r2, _MM_SHUFFLE(1, 1, 1, 1)), l));
		//	v3 = _mm_add_ps(v3, _mm_mul_ps(_mm_shuffle_ps(r3, r3, _MM_SHUFFLE(1, 1, 1, 1)), l));
		//
		//	l = _mm_load_ps(m.c.Data());
		//	v0 = _mm_add_ps(v0, _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2, 2, 2, 2)), l));
		//	v1 = _mm_add_ps(v1, _mm_mul_ps(_mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2)), l));
		//	v2 = _mm_add_ps(v2, _mm_mul_ps(_mm_shuffle_ps(r2, r2, _MM_SHUFFLE(2, 2, 2, 2)), l));
		//	v3 = _mm_add_ps(v3, _mm_mul_ps(_mm_shuffle_ps(r3, r3, _MM_SHUFFLE(2, 2, 2, 2)), l));
		//
		//	l = _mm_load_ps(m.d.Data());
		//	v0 = _mm_add_ps(v0, _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(3, 3, 3, 3)), l));
		//	v1 = _mm_add_ps(v1, _mm_mul_ps(_mm_shuffle_ps(r1, r1, _MM_SHUFFLE(3, 3, 3, 3)), l));
		//	v2 = _mm_add_ps(v2, _mm_mul_ps(_mm_shuffle_ps(r2, r2, _MM_SHUFFLE(3, 3, 3, 3)), l));
		//	v3 = _mm_add_ps(v3, _mm_mul_ps(_mm_shuffle_ps(r3, r3, _MM_SHUFFLE(3, 3, 3, 3)), l));
		//
		//	_mm_store_ps(f0.Data(), v0);
		//	_mm_store_ps(f1.Data(), v1);
		//	_mm_store_ps(f2.Data(), v2);
		//	_mm_store_ps(f3.Data(), v3);
		//
		//	return { f0, f1, f2, f3 };
		//#elif defined NH_ARM_NEON
		//
		//#else
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
		//#endif
	}
	constexpr Vector2 operator*(const Vector2& v) const
	{
		return {
			a.x * v.x + b.x * v.y,
			a.y * v.x + b.y * v.y
		};
	}
	constexpr Vector3 operator*(const Vector3& v) const
	{
		return {
			a.x * v.x + b.x * v.y + c.x * v.z,
			a.y * v.x + b.y * v.y + c.y * v.z,
			a.z * v.x + b.z * v.y + c.z * v.z
		};
	}
	constexpr Vector4 operator*(const Vector4& v) const
	{
		return {
			a.x * v.x + b.x * v.y + c.x * v.z + c.x * v.w,
			a.y * v.x + b.y * v.y + c.y * v.z + c.y * v.w,
			a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.w,
			a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.w
		};
	}

	constexpr Matrix4 Inverse() const
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
	constexpr Matrix4& Inversed()
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
	constexpr Matrix4 Invert() const
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
	constexpr Matrix4& Inverted()
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
	constexpr Matrix4 Transpose() const
	{
		return {
			a.x, b.x, c.x, d.x,
			a.y, b.y, c.y, d.y,
			a.z, b.z, c.z, d.z,
			a.w, b.w, c.w, d.w
		};
	}
	constexpr Matrix4& Transposed()
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

	constexpr void Set(const Vector3& position, const Vector3& rotation, const Vector3& scale);
	constexpr void Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale);
	constexpr void SetPerspective(F32 fov, F32 aspect, F32 near, F32 far)
	{
		F32 yScale = 1.0f / Math::Tan(fov * DEG_TO_RAD_F * 0.5f);
		F32 xScale = yScale / aspect;
		F32 nearFar = 1.0f / (near - far);

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
		c.z = -far * nearFar;
		c.w = 1.0f;

		d.x = 0.0f;
		d.y = 0.0f;
		d.z = far * near * nearFar;
		d.w = 0.0f;
	}
	constexpr void SetOrthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far)
	{
		F32 rightLeft = 1.0f / (right - left);
		F32 bottomTop = 1.0f / (bottom - top);
		F32 farNear = 1.0f / (far - near);

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
		d.z = -(far + near) * farNear;
		d.w = 1.0f;
	}
	constexpr void SetPosition(const Vector3& position)
	{
		d.x = position.x;
		d.y = position.y;
		d.z = position.z;
	}

	constexpr void LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
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

	constexpr Vector3 Forward() { return Vector3(-a.z, -b.z, -c.z).Normalize(); }
	constexpr Vector3 Back() { return Vector3(a.z, b.z, c.z).Normalize(); }
	constexpr Vector3 Right() { return Vector3(a.x, b.x, c.x).Normalize(); }
	constexpr Vector3 Left() { return Vector3(-a.x, -b.x, -c.x).Normalize(); }
	constexpr Vector3 Up() { return Vector3(a.y, b.y, c.y).Normalize(); }
	constexpr Vector3 Down() { return Vector3(-a.y, -b.y, -c.y).Normalize(); }

	constexpr Matrix4 operator-() { return { -a, -b, -c, -d }; }
	constexpr Matrix4 operator~() { return { -a, -b, -c, -d }; }
	constexpr Matrix4 operator!() { return { -a, -b, -c, -d }; }

	constexpr bool operator==(const Matrix4& m) const { return a == m.a && b == m.b && c == m.c && d == m.d; }
	constexpr bool operator!=(const Matrix4& m) const { return a != m.a || b != m.b || c != m.c || d != m.d; }

	Vector4& operator[] (U8 i) { return (&a)[i]; }
	const Vector4& operator[] (U8 i) const { return (&a)[i]; }

	F32* Data() { return a.Data(); }
	const F32* Data() const { return a.Data(); }

public:
	Vector4 a, b, c, d; //Columns
};

export struct NH_API Quaternion2
{
	constexpr Quaternion2() : x(0.0f), y(1.0f) {}
	constexpr Quaternion2(F32 x, F32 y) : x(x), y(y) {}
	constexpr Quaternion2(F32 angle) { F32 a = angle * DEG_TO_RAD_F * 0.5f; x = Math::Sin(a); y = Math::Cos(a); }
	constexpr Quaternion2(const Matrix2& mat) : x(Math::Sqrt((mat.a.x - 1.0f) * -0.5f)), y(Math::Sqrt(mat.a.y * 0.5f)) {}
	constexpr Quaternion2(const Quaternion2& q) : x(q.x), y(q.y) {}
	constexpr Quaternion2(Quaternion2&& q) noexcept : x(q.x), y(q.y) {}

	constexpr Quaternion2& operator=(F32 angle) { F32 a = angle * DEG_TO_RAD_F * 0.5f; x = Math::Sin(a); y = Math::Cos(a); return *this; }
	constexpr Quaternion2& operator=(const Quaternion2& q) { x = q.x; y = q.y; return *this; }
	constexpr Quaternion2& operator=(Quaternion2&& q) noexcept { x = q.x; y = q.y; return *this; }

	constexpr Quaternion2& operator+=(const Quaternion2& q) { x += q.x; y += q.y; return *this; }
	constexpr Quaternion2& operator-=(const Quaternion2& q) { x -= q.x; y -= q.y; return *this; }
	constexpr Quaternion2& operator*=(const Quaternion2& q) { x = y * q.x + x * q.y; y = y * q.y - x * q.x; return *this; }
	constexpr Quaternion2& operator/=(const Quaternion2& q)
	{
		F32 n2 = 1.0f / q.SqrNormal();

		x = (-y * q.x + x * q.y) * n2;
		y = (y * q.y + x * q.x) * n2;

		return *this;
	}


	constexpr Quaternion2 operator*(F32 f) const { return { x * f, y * f }; }
	constexpr Quaternion2 operator/(F32 f) const { return { x / f, y / f }; }
	constexpr Quaternion2 operator+(const Quaternion2& q) const { return { x + q.x, y + q.y }; }
	constexpr Quaternion2 operator-(const Quaternion2& q) const { return { x - q.x, y - q.y }; }
	constexpr Quaternion2 operator*(const Quaternion2& q) const { return { x * q.y + y * q.x, y * q.y - x * q.x, }; }
	constexpr Quaternion2 operator^(const Quaternion2& q) const { return { y * q.x - x * q.y, y * q.y + x * q.x, }; }
	constexpr Quaternion2 operator/(const Quaternion2& q) const
	{
		F32 n2 = 1.0f / q.SqrNormal();

		return {
			(x * q.y - y * q.x) * n2,
			(y * q.y + x * q.x) * n2
		};
	}

	constexpr void Set(F32 angle) { F32 a = angle * DEG_TO_RAD_F * 0.5f; x = Math::Sin(a); y = Math::Cos(a); }
	constexpr void Rotate(F32 angle) { F32 a = angle * DEG_TO_RAD_F * 0.5f; x += Math::Sin(a); y += Math::Cos(a); }
	F32 Angle() const { return Math::Asin(x) * RAD_TO_DEG_F * 2.0f; } //TODO: constexpr

	constexpr Matrix2 ToMatrix2() const
	{
		Quaternion2 q = Normalize();

		F32 xx = 2.0f * q.x * q.x;
		F32 xy = 2.0f * q.y * q.y;

		return {
			1.0f - xx, xy,
			-xy, 1.0f - xx
		};
	}
	constexpr Matrix3 ToMatrix3() const
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
	constexpr Matrix4 RotationMatrix(Vector2 center) const
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

	constexpr Quaternion2 Slerp(const Quaternion2& q, F32 t) const
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

		F32 theta0 = Math::Acos(dot);
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
	constexpr Quaternion2& Slerped(const Quaternion2& q, F32 t)
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

		F32 theta0 = Math::Acos(dot);
		F32 theta = theta0 * t;
		F32 sinTheta = Math::Sin(theta);
		F32 sinTheta0 = Math::Sin(theta0);

		F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
		F32 s1 = sinTheta / sinTheta0;

		x = v0.x * s0 + v1.x * s1;
		y = v0.y * s0 + v1.y * s1;

		return *this;
	}
	constexpr Quaternion2 NLerp(const Quaternion2& q, F32 t) const
	{
		F32 omt = 1.0f - t;
		Quaternion2 quat = {
			omt * x + t * q.x,
			omt * y + t * q.y
		};

		return quat.Normalize();
	}
	constexpr Quaternion2& NLerped(const Quaternion2& q, F32 t)
	{
		F32 omt = 1.0f - t;
		x = omt * x + t * q.x;
		y = omt * y + t * q.y;

		return Normalized();
	}

	constexpr F32 RelativeAngle(const Quaternion2& q) const { return 0.0f; }
	constexpr F32 Dot(const Quaternion2& q) const { return x * q.x + y * q.y; }
	constexpr F32 SqrNormal() const { return x * x + y * y; }
	constexpr F32 Normal() const { return Math::Sqrt(x * x + y * y); }
	constexpr Quaternion2 Normalize() const { F32 n = 1.0f / Normal(); return { x * n, y * n }; }
	constexpr Quaternion2& Normalized() { F32 n = 1.0f / Normal(); x *= n; y *= n; return *this; }
	constexpr Quaternion2 Conjugate() const { return { -x, y }; }
	constexpr Quaternion2& Conjugated() { x = -x; return *this; }
	constexpr Quaternion2 Inverse() const { F32 n = 1.0f / Math::Sqrt(x * x + y * y); return { -x * n, y * n }; }
	constexpr Quaternion2& Inversed() { return Conjugated().Normalized(); }
	constexpr Quaternion2 Integrate(F32 deltaAngle)
	{
		Quaternion2 q2 = { x + deltaAngle * y, y - deltaAngle * x };
		F32 mag = q2.Normal();
		F32 invMag = mag > 0.0 ? 1.0f / mag : 0.0f;
		return { q2.x * invMag, q2.y * invMag };
	}
	constexpr Quaternion2& Integrated(F32 deltaAngle)
	{
		Quaternion2 q2 = { x + deltaAngle * y, y - deltaAngle * x };
		F32 mag = q2.Normal();
		F32 invMag = mag > 0.0 ? 1.0f / mag : 0.0f;
		Quaternion2 qn = { q2.x * invMag, q2.y * invMag };
		x = q2.x * invMag;
		y = q2.y * invMag;

		return *this;
	}

	F32& operator[] (U8 i) { return (&x)[i]; }
	const F32& operator[] (U8 i) const { return (&x)[i]; }

	F32* Data() { return &x; }
	const F32* Data() const { return &x; }

	constexpr explicit operator Quaternion3() const;

public:
	F32 x, y; //sin, cos
};

//TODO: Cache euler angles
//W is real part
export struct NH_API Quaternion3
{
	constexpr Quaternion3() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	constexpr Quaternion3(F32 x, F32 y, F32 z, F32 w) : x(x), y(y), z(z), w(w) {}
	constexpr Quaternion3(const Vector3& euler)
	{
		F32 hx = euler.x * DEG_TO_RAD_F * 0.5f;
		F32 hy = euler.y * DEG_TO_RAD_F * 0.5f;
		F32 hz = euler.z * DEG_TO_RAD_F * 0.5f;

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
	constexpr Quaternion3(const Vector3& axis, F32 angle)
	{
		const F32 halfAngle = angle * DEG_TO_RAD_F * 0.5f;
		F32 s = Math::Sin(halfAngle);
		F32 c = Math::Cos(halfAngle);

		//TODO: Normalize axis?

		x = s * axis.x;
		y = s * axis.y;
		z = s * axis.z;
		w = c;
	}
	constexpr Quaternion3(const Quaternion3& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
	constexpr Quaternion3(Quaternion3&& q) noexcept : x(q.x), y(q.y), z(q.z), w(q.w) {}

	constexpr Quaternion3& operator=(const Vector3& euler)
	{
		F32 hx = euler.x * DEG_TO_RAD_F * 0.5f;
		F32 hy = euler.y * DEG_TO_RAD_F * 0.5f;
		F32 hz = euler.z * DEG_TO_RAD_F * 0.5f;

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

	constexpr Quaternion3& operator=(const Quaternion3& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
	constexpr Quaternion3& operator=(Quaternion3&& q) noexcept { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }

	constexpr Quaternion3& operator+=(const Quaternion3& q)
	{
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;

		return *this;
	}
	constexpr Quaternion3& operator-=(const Quaternion3& q)
	{
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;

		return *this;
	}
	constexpr Quaternion3& operator*=(const Quaternion3& q)
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
	constexpr Quaternion3& operator/=(const Quaternion3& q)
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

	constexpr Quaternion3 operator+(const Quaternion3& q) const
	{
		return {
			x + q.x,
			y + q.y,
			z + q.z,
			w + q.w
		};
	}
	constexpr Quaternion3 operator-(const Quaternion3& q) const
	{
		return {
			x - q.x,
			y - q.y,
			z - q.z,
			w - q.w
		};
	}
	constexpr Quaternion3 operator*(const Quaternion3& q) const
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
	constexpr Quaternion3 operator/(const Quaternion3& q) const
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

	constexpr Matrix3 ToMatrix3() const
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
			xy - zw,  1.0f - xx - zz,  yz + xw,
			xz + yw, yz - xw, 1.0f - xx - yy
		};
	}
	constexpr Matrix4 ToMatrix4() const
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
			xy - zw,  1.0f - xx - zz,  yz + xw, 0.0f,
			xz + yw, yz - xw, 1.0f - xx - yy, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	constexpr Matrix4 RotationMatrix(Vector3 center) const
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

	constexpr Vector3 Euler() const
	{
		F32 v = x * y + z * w;

		if (Math::Abs(v - 0.5f) < Traits<F32>::Epsilon) { return { 2.0f * Math::Atan2(x, w) * RAD_TO_DEG_F, HALF_PI_F * RAD_TO_DEG_F, 0.0f }; }
		if (Math::Abs(v + 0.5f) < Traits<F32>::Epsilon) { return { -2.0f * Math::Atan2(x, w) * RAD_TO_DEG_F, -HALF_PI_F * RAD_TO_DEG_F, 0.0f }; }

		return {
			Math::Asin(2.0f * v) * RAD_TO_DEG_F,
			Math::Atan2(2.0f * (w * y - x * z), 1.0f - 2.0f * (y * y + z * z)) * RAD_TO_DEG_F,
			Math::Atan2(2.0f * (w * x - y * z), 1.0f - 2.0f * (x * x + z * z)) * RAD_TO_DEG_F
		};
	}

	constexpr Quaternion3 Slerp(const Quaternion3& q, F32 t) const
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

		F32 theta0 = Math::Acos(dot);
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
	constexpr Quaternion3& Slerped(const Quaternion3& q, F32 t)
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

		F32 theta0 = Math::Acos(dot);
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

	constexpr F32 Dot(const Quaternion3& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
	constexpr F32 SqrNormal() const { return x * x + y * y + z * z + w * w; }
	constexpr F32 Normal() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
	constexpr Quaternion3 Cross(const Quaternion3& q) const { Vector3 v = Vector3{ x, y, z }.Cross({ q.x, q.y, q.z }); return { v.x, v.y, v.z, -w * q.w }; }
	constexpr Quaternion3 Normalize() const { F32 n = 1.0f / Normal(); return { x * n, y * n, z * n, w * n }; }
	constexpr Quaternion3& Normalized() { F32 n = 1.0f / Normal(); x *= n; y *= n; z *= n; w *= n; return *this; }
	constexpr Quaternion3 Conjugate() const { return { -x, -y, -z, w }; }
	constexpr Quaternion3& Conjugated() { x = -x; y = -y; z = -z; return *this; }
	constexpr Quaternion3 Inverse() const
	{
		F32 n = 1.0f / Math::Sqrt(x * x + y * y + z * z + w * w);
		return { -x * n, -y * n, -z * n, w * n };
	}
	constexpr Quaternion3& Inversed() { return Conjugated().Normalized(); }

	F32& operator[] (U8 i) { return (&x)[i]; }
	const F32& operator[] (U8 i) const { return (&x)[i]; }

	F32* Data() { return &x; }
	const F32* Data() const { return &x; }

	constexpr explicit operator Quaternion2() const;

public:
	F32 x, y, z, w;
};

inline constexpr Quaternion2::operator Quaternion3() const { return Quaternion3{ 0.0f, 0.0f, x, y }; }
inline constexpr Quaternion3::operator Quaternion2() const { return Quaternion2{ x, w }; }

constexpr Vector2& Vector2::operator*=(const Quaternion2& q) { F32 temp = q.y * x - q.x * y; y = q.x * x + q.y * y; x = temp; return *this; }
constexpr Vector2 Vector2::operator*(const Quaternion2& q) const { return Vector2{ q.y * x - q.x * y, q.x * x + q.y * y }; }
constexpr Vector2 Vector2::operator^(const Quaternion2& q) const { return Vector2{ q.y * x + q.x * y, q.y * y - q.x * x }; }
constexpr Vector2& Vector2::Rotate(const Vector2& center, const Quaternion2& quat)
{
	F32 temp = quat.y * (x - center.x) - quat.x * (y - center.y) + center.x;
	y = quat.x * (x - center.x) + quat.y * (y - center.y) + center.y;
	x = temp;
	return *this;
}
constexpr Vector2 Vector2::Rotated(const Vector2& center, const Quaternion2& quat) const
{
	return Vector2{ quat.y * (x - center.x) - quat.x * (y - center.y) + center.x, quat.x * (x - center.x) + quat.y * (y - center.y) + center.y };
}
constexpr Vector2& Vector2::Rotate(const Quaternion2& quat)
{
	y = quat.y * x - quat.x * y;
	x = quat.x * x + quat.y * y;
	return *this;
}
constexpr Vector2 Vector2::Rotated(const Quaternion2& quat) const
{
	return { quat.y * x - quat.x * y, quat.x * x + quat.y * y };
}

constexpr Vector3& Vector3::operator*=(const Quaternion3& q)
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
constexpr Vector3 Vector3::operator*(const Quaternion3& q) const
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

constexpr Matrix3::Matrix3(const Vector2& position, const Quaternion2& rotation, const Vector2& scale)
{
	F32 cos = rotation.y;
	F32 sin = rotation.x;
	a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
	a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
	a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
}

constexpr void Matrix3::Set(const Vector2& position, const Quaternion2& rotation, const Vector2& scale)
{
	F32 cos = rotation.y;
	F32 sin = rotation.x;
	a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
	a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
	a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
}

constexpr Matrix4::Matrix4(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	Quaternion3 q{ rotation };
	q.Normalized();

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
constexpr Matrix4::Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
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

constexpr void Matrix4::Set(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	Quaternion3 q{ rotation };
	q.Normalized();

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
constexpr void Matrix4::Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
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

export struct NH_API Transform
{
public:
	const Matrix4& LocalMatrix()
	{
		if (dirty) { dirty = false; localMatrix.Set(position, rotation, scale); }

		return localMatrix;
	}

	const Matrix4& WorldMatrix()
	{
		if (dirty) { dirty = false; localMatrix.Set(position, rotation, scale); }

		if (parent)
		{
			worldMatrix = localMatrix * parent->WorldMatrix();
			return worldMatrix;
		}

		return localMatrix;
	}

	const Vector3& Position() const { return position; }
	const Vector3& Scale() const { return scale; }
	const Quaternion3& Rotation() const { return rotation; }

	void Translate(const Vector3& translation)
	{
		position += translation;
		localMatrix.SetPosition(position);
	}

	void SetPosition(const Vector3& position)
	{
		this->position = position;
		localMatrix.SetPosition(position);
	}

	void SetRotation(const Quaternion3& rotation)
	{
		dirty = true;
		this->rotation = rotation;
	}

	void SetScale(const Vector3& scale)
	{
		dirty = true;
		this->scale = scale;
	}

private:
	bool dirty{ false };
	Transform* parent{ nullptr };
	Vector3 position{};
	Vector3 scale{ 1.0f };
	Quaternion3 rotation{};
	Matrix4 localMatrix{};
	Matrix4 worldMatrix{};
};

export template <class Type> inline constexpr bool IsFloatVectorType = AnyOf<RemovedQuals<Type>, Vector2, Vector3, Vector4>;
export template <class Type> inline constexpr bool IsIntVectorType = AnyOf<RemovedQuals<Type>, Vector2Int, Vector3Int, Vector4Int>;
export template <class Type> inline constexpr bool IsVectorType = AnyOf<RemovedQuals<Type>, Vector2, Vector3, Vector4, Vector2Int, Vector3Int, Vector4Int>;
export template <class Type> inline constexpr bool IsMatrixType = AnyOf<RemovedQuals<Type>, Matrix2, Matrix3, Matrix4>;
export template <class Type> inline constexpr bool IsQuaternionType = AnyOf<RemovedQuals<Type>, Quaternion2, Quaternion3>;

export template <class Type> concept FloatVectorType = IsFloatVectorType<Type>;
export template <class Type> concept FloatOrVector = IsFloatVectorType<Type> || IsFloatingPoint<Type>;
export template <class Type> concept IntVectorType = IsIntVectorType<Type>;
export template <class Type> concept VectorType = IsVectorType<Type>;
export template <class Type> concept MatrixType = IsMatrixType<Type>;
export template <class Type> concept QuaternionType = IsQuaternionType<Type>;

constexpr U64 MAX_SPLINE_POINTS = 128;

//TODO: Edit at runtime
export template<FloatOrVector Type>
struct LinearSpline
{
	static constexpr U64 MinPoints = 2;

	template<class... Points>
	constexpr LinearSpline(Points&&... pointsArgs) noexcept : pointCount(sizeof...(pointsArgs)), points(pointsArgs...)
	{
		static_assert(sizeof...(pointsArgs) >= MinPoints && sizeof...(pointsArgs) < MAX_SPLINE_POINTS);
	}

	template<FloatingPoint Float>
	constexpr Type operator[](Float t) const noexcept
	{
		I32 index = Math::Clamp((I32)t, 0, (I32)pointCount - 1);
		t -= (Float)index;

		return Math::Lerp(points[index], points[index + 1], t);
	}

	constexpr U32 Count() const { return pointCount; }

private:
	Type points[MAX_SPLINE_POINTS];
	U32 pointCount;
};

//TODO: Edit at runtime
export template<FloatOrVector Type>
struct BezierSpline
{
	static constexpr U64 MinPoints = 4;

	template<class... Points>
	constexpr BezierSpline(Points&&... pointsArgs) noexcept : pointCount(sizeof...(pointsArgs)), points(pointsArgs...)
	{
		static_assert(sizeof...(pointsArgs) >= MinPoints && sizeof...(pointsArgs) < MAX_SPLINE_POINTS);
	}

	template<FloatingPoint Float>
	constexpr Type operator[](Float t) const noexcept
	{
		I32 index = Math::Clamp((I32)t, 0, (I32)pointCount - 3);

		t -= (Float)index;
		Float t2 = t * t;
		Float t3 = t2 * t;

		Float f1 = -t3 + (Float)3.0 * t2 - (Float)3.0 * t + (Float)1.0;
		Float f2 = (Float)3.0 * t3 - (Float)6.0 * t2 + (Float)3.0 * t;
		Float f3 = -(Float)3.0 * t3 + (Float)3.0 * t2;
		Float f4 = t3;

		return (points[index] * f1 + points[index + 1] * f2 + points[index + 2] * f3 + points[index + 3] * f4);
	}

	constexpr U32 Count() const { return pointCount; }

private:
	Type points[MAX_SPLINE_POINTS];
	U32 pointCount;
};

//TODO: Edit at runtime
export template<FloatOrVector Type>
struct CatmullRomSpline
{
	static constexpr U64 MinPoints = 4;

	template<class... Points>
	constexpr CatmullRomSpline(Points&&... pointsArgs) noexcept : pointCount(sizeof...(pointsArgs)), points(pointsArgs...)
	{
		static_assert(sizeof...(pointsArgs) >= MinPoints && sizeof...(pointsArgs) < MAX_SPLINE_POINTS);
	}

	template<FloatingPoint Float>
	constexpr Type operator[](Float t) const noexcept
	{
		I32 index = Math::Clamp((I32)t, 0, (I32)pointCount - 3);

		t -= (Float)index;
		Float t2 = t * t;
		Float t3 = t2 * t;

		Float f1 = -t3 + (Float)2.0 * t2 - t;
		Float f2 = (Float)3.0 * t3 - (Float)5.0 * t2 + (Float)2.0;
		Float f3 = -(Float)3.0 * t3 + (Float)4.0 * t2 + t;
		Float f4 = t3 - t2;

		return (points[index] * f1 + points[index + 1] * f2 + points[index + 2] * f3 + points[index + 3] * f4) * (Float)0.5;
	}

	constexpr U32 Count() const { return pointCount; }

private:
	Type points[MAX_SPLINE_POINTS];
	U32 pointCount;
};

//TODO: Edit at runtime
export template<FloatOrVector Type>
struct CardinalSpline
{
	static constexpr U64 MinPoints = 4;

	template<class... Points>
	constexpr CardinalSpline(F64 scale, Points&&... pointsArgs) noexcept : pointCount(sizeof...(pointsArgs)), points(pointsArgs...), scale(scale)
	{
		static_assert(sizeof...(pointsArgs) >= MinPoints && sizeof...(pointsArgs) < MAX_SPLINE_POINTS);
	}

	template<FloatingPoint Float>
	constexpr Type operator[](Float t) const noexcept
	{
		I32 index = Math::Clamp((I32)t, 0, (I32)pointCount - 3);

		t -= (Float)index;
		Float t2 = t * t;
		Float t3 = t2 * t;

		Float f1 = -scale * t3 + (Float)2.0 * scale * t2 - scale * t;
		Float f2 = ((Float)2.0 - scale) * t3 + (scale - (Float)3.0) * t2 + (Float)1.0;
		Float f3 = (scale - (Float)2.0) * t3 + ((Float)3.0 - (Float)2.0 * scale) * t2 + scale * t;
		Float f4 = scale * t3 - scale * t2;

		return (points[index] * f1 + points[index + 1] * f2 + points[index + 2] * f3 + points[index + 3] * f4);
	}

	constexpr U32 Count() const { return pointCount; }

private:
	Type points[MAX_SPLINE_POINTS];
	U32 pointCount;
	F64 scale;
};

//TODO: Edit at runtime
export template<FloatOrVector Type>
struct BSpline
{
	static constexpr U64 MinPoints = 4;

	template<class... Points>
	constexpr BSpline(Points&&... pointsArgs) noexcept : pointCount(sizeof...(pointsArgs)), points(pointsArgs...)
	{
		static_assert(sizeof...(pointsArgs) >= MinPoints && sizeof...(pointsArgs) < MAX_SPLINE_POINTS);
	}

	template<FloatingPoint Float>
	constexpr Type operator[](Float t) const noexcept
	{
		I32 index = Math::Clamp((I32)t, 0, (I32)pointCount - 3);

		t -= (Float)index;
		Float t2 = t * t;
		Float t3 = t2 * t;

		Float f1 = -t3 + (Float)3.0 * t2 - (Float)3.0 * t + (Float)1.0;
		Float f2 = (Float)3.0 * t3 - (Float)6.0 * t2 + (Float)4.0;
		Float f3 = -(Float)3.0 * t3 + (Float)3.0 * t2 + (Float)3.0 * t + (Float)1.0;
		Float f4 = t3;

		return (points[index] * f1 + points[index + 1] * f2 + points[index + 2] * f3 + points[index + 3] * f4) * (Float)0.166666667;
	}

	constexpr U32 Count() const { return pointCount; }

private:
	Type points[MAX_SPLINE_POINTS];
	U32 pointCount;
};

//TODO: Edit at runtime
export template<FloatOrVector Type>
struct HermiteSpline
{
	static constexpr U64 MinPoints = 2;

	template<typename... Points>
	constexpr HermiteSpline(Points&&... pointsArgs) noexcept : pointCount(sizeof...(pointsArgs)), points(pointsArgs...)
	{
		static_assert(sizeof...(pointsArgs) >= MinPoints && sizeof...(pointsArgs) < MAX_SPLINE_POINTS);
	}

	template<FloatingPoint Float>
	constexpr Vector2 operator[](Float t) const noexcept
	{
		I32 index = Math::Clamp((I32)t, 0, (I32)pointCount - 1);

		t -= (Float)index;
		Float t2 = t * t;
		Float t3 = t2 * t;

		Float f1 = (Float)2.0 * t3 - (Float)3.0 * t2 + (Float)1.0;
		Float f2 = t3 - (Float)2.0 * t2 + t;
		Float f3 = -(Float)2.0 * t3 + (Float)3.0 * t2;
		Float f4 = t3 - t2;

		return (points[index].xy() * f1 + points[index].zw() * f2 + points[index + 1].xy() * f3 + points[index + 1].zw() * f4);
	}

	constexpr U32 Count() const { return pointCount; }

private:
	Vector4 points[MAX_SPLINE_POINTS];
	U32 pointCount;
};

export template <class Type> inline constexpr bool IsSplineType = AnyOf<RemovedQuals<Type>, BezierSpline, CatmullRomSpline, CardinalSpline, BSpline, HermiteSpline>;
export template <class Type> concept SplineType = IsSplineType<Type>;