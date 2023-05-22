#include "Math.hpp"

#include "Containers\String.hpp"

#include "SIMD.hpp"

//Math

Quaternion2 Math::Slerp(const Quaternion2& a, const Quaternion2& b, F32 t)
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion2 v0 = a.Normalize();
	Quaternion2 v1 = b.Normalize();

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

Quaternion3 Math::Slerp(const Quaternion3& a, const Quaternion3& b, F32 t)
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion3 v0 = a.Normalize();
	Quaternion3 v1 = b.Normalize();

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

//Vector2

Vector2::Vector2() : x{ 0.0f }, y{ 0.0f } {}
Vector2::Vector2(F32 f) : x{ f }, y{ f } {}
Vector2::Vector2(F32 x, F32 y) : x{ x }, y{ y } {}
Vector2::Vector2(const Vector2& v) : x{ v.x }, y{ v.y } {}
Vector2::Vector2(Vector2&& v) noexcept : x{ v.x }, y{ v.y } {}

Vector2& Vector2::operator=(F32 f) { x = f; y = f; return *this; }
Vector2& Vector2::operator=(const Vector2& v) { x = v.x; y = v.y; return *this; }
Vector2& Vector2::operator=(Vector2&& v) noexcept { x = v.x; y = v.y; return *this; }

Vector2& Vector2::operator+=(F32 f) { x += f; y += f; return *this; }
Vector2& Vector2::operator-=(F32 f) { x -= f; y -= f; return *this; }
Vector2& Vector2::operator*=(F32 f) { x *= f; y *= f; return *this; }
Vector2& Vector2::operator/=(F32 f) { x /= f; y /= f; return *this; }
Vector2& Vector2::operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); return *this; }
Vector2& Vector2::operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
Vector2& Vector2::operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
Vector2& Vector2::operator*=(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
Vector2& Vector2::operator/=(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
Vector2& Vector2::operator%=(const Vector2& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); return *this; }
Vector2& Vector2::operator*=(const Quaternion2& q)
{
	//TODO:
	return *this;
}

Vector2 Vector2::operator+(F32 f) const { return { x + f, y + f }; }
Vector2 Vector2::operator-(F32 f) const { return { x - f, y - f }; }
Vector2 Vector2::operator*(F32 f) const { return { x * f, y * f }; }
Vector2 Vector2::operator/(F32 f) const { return { x / f, y / f }; }
Vector2 Vector2::operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f) }; }
Vector2 Vector2::operator+(const Vector2& v) const { return { x + v.x, y + v.y }; }
Vector2 Vector2::operator-(const Vector2& v) const { return { x - v.x, y - v.y }; }
Vector2 Vector2::operator*(const Vector2& v) const { return { x * v.x, y * v.y }; }
Vector2 Vector2::operator/(const Vector2& v) const { return { x / v.x, y / v.y }; }
Vector2 Vector2::operator%(const Vector2& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y) }; }
Vector2 Vector2::operator*(const Quaternion2& q) const
{
	//TODO:
	return {};
}

bool Vector2::operator==(const Vector2& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y); }
bool Vector2::operator!=(const Vector2& v) const { return !Math::Zero(x - v.x) || !Math::Zero(y - v.y); }
bool Vector2::operator>(const Vector2& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
bool Vector2::operator<(const Vector2& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
bool Vector2::operator>=(const Vector2& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
bool Vector2::operator<=(const Vector2& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
bool Vector2::IsZero() const { return Math::Zero(x) && Math::Zero(y); }

Vector2 Vector2::operator-() const { return { -x, -y }; }
Vector2 Vector2::operator~() const { return { -x, -y }; }
Vector2 Vector2::operator!() const { return { -x, -y }; }

F32 Vector2::SqrMagnitude() const { return x * x + y * y; }
F32 Vector2::Magnitude() const { return Math::Sqrt(x * x + y * y); }
F32 Vector2::Dot(const Vector2& v) const { return x * v.x + y * v.y; }
Vector2& Vector2::Normalize() { Vector2 v = Normalized(); x = v.x; y = v.y; return *this; }
Vector2 Vector2::Normalized() const { return IsZero() ? Vector2::Zero : (*this) / Magnitude(); }
F32 Vector2::AngleBetween(const Vector2& v) const { return Math::Acos(Dot(v) * Math::InvSqrt(Dot(*this) * v.Dot(v))); }
Vector2 Vector2::Projection(const Vector2& v) const { return v * (Dot(v) / v.Dot(v)); }
Vector2 Vector2::OrthoProjection(const Vector2& v) const { return *this - Projection(v); }
F32 Vector2::Cross(const Vector2& v) const { return x * v.y - y * v.x; }
Vector2 Vector2::Cross(const F32 f) const { return { y * f, x * -f }; }
Vector2 Vector2::Normal(const Vector2& v) const { return Vector2(-(v.y - y), v.x - x).Normalized(); }

Vector2& Vector2::Rotate(const Vector2& center, F32 angle)
{
	F32 cos = Math::Cos(angle * DEG_TO_RAD_F);
	F32 sin = Math::Sin(angle * DEG_TO_RAD_F);
	F32 temp = cos * (x - center.x) - sin * (y - center.y) + center.x;
	y = sin * (x - center.x) + cos * (y - center.y) + center.y;
	x = temp;

	return *this;
}

Vector2 Vector2::Rotated(const Vector2& center, F32 angle) const
{
	F32 cos = Math::Cos(angle * DEG_TO_RAD_F);
	F32 sin = Math::Sin(angle * DEG_TO_RAD_F);
	return Vector2{ cos * (x - center.x) - sin * (y - center.y) + center.x,
	sin * (x - center.x) + cos * (y - center.y) + center.y };
}

Vector2& Vector2::Rotate(const Vector2& center, const Quaternion2& quat)
{
	F32 temp = quat.y * (x - center.x) - quat.x * (y - center.y) + center.x;
	y = quat.x * (x - center.x) + quat.y * (y - center.y) + center.y;
	x = temp;

	return *this;
}

Vector2 Vector2::Rotated(const Vector2& center, const Quaternion2& quat) const
{
	return Vector2{ quat.y * (x - center.x) - quat.x * (y - center.y) + center.x,
	quat.x * (x - center.x) + quat.y * (y - center.y) + center.y };
}

Vector2& Vector2::Clamp(const Vector2& min, const Vector2& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	return *this;
}

Vector2 Vector2::Clamped(const Vector2& min, const Vector2& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y)
	};
}

Vector2& Vector2::SetClosest(const Vector2& min, const Vector2& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	return *this;
}

Vector2 Vector2::Closest(const Vector2& min, const Vector2& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y)
	};
}

F32& Vector2::operator[] (U64 i) { return (&x)[i]; }
const F32& Vector2::operator[] (U64 i) const { return (&x)[i]; }

F32* Vector2::Data() { return &x; }
const F32* Vector2::Data() const { return &x; }

Vector2::operator String() const { return String(x, ", ", y); }
Vector2::operator String16() const { return String16(x, u", ", y); }
Vector2::operator String32() const { return String32(x, U", ", y); }

//Vector3

Vector3::Vector3() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f } {}
Vector3::Vector3(F32 f) : x{ f }, y{ f }, z{ f } {}
Vector3::Vector3(F32 x, F32 y, F32 z) : x{ x }, y{ y }, z{ z } {}
Vector3::Vector3(const Vector3& v) : x{ v.x }, y{ v.y }, z{ v.z } {}
Vector3::Vector3(Vector3&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z } {}

Vector3& Vector3::operator=(F32 f) { x = f; y = f; z = f; return *this; }
Vector3& Vector3::operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z; return *this; }
Vector3& Vector3::operator=(Vector3&& v) noexcept { x = v.x; y = v.y; z = v.z; return *this; }

Vector3& Vector3::operator+=(F32 f) { x += f; y += f; z += f; return *this; }
Vector3& Vector3::operator-=(F32 f) { x -= f; y -= f; z -= f; return *this; }
Vector3& Vector3::operator*=(F32 f) { x *= f; y *= f; z *= f; return *this; }
Vector3& Vector3::operator/=(F32 f) { x /= f; y /= f; z /= f; return *this; }
Vector3& Vector3::operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); return *this; }
Vector3& Vector3::operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
Vector3& Vector3::operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
Vector3& Vector3::operator*=(const Vector3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
Vector3& Vector3::operator/=(const Vector3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
Vector3& Vector3::operator%=(const Vector3& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); z = Math::Mod(z, v.z); return *this; }
Vector3& Vector3::operator*=(const Quaternion3& q)
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
	F32 xt = ((x * ((1.0f - yyy) - zzz)) + (y * (yyx - zzw))) + (z * (zzx + yyw));
	F32 yt = ((x * (yyx + zzw)) + (y * ((1.0f - xxx) - zzz))) + (z * (zzy - xxw));
	F32 zt = ((x * (zzx - yyw)) + (y * (zzy + xxw))) + (z * ((1.0f - xxx) - yyy));

	x = xt;
	y = yt;
	z = zt;

	return *this;
}

Vector3 Vector3::operator+(F32 f) const { return { x + f, y + f, z + f }; }
Vector3 Vector3::operator-(F32 f) const { return { x - f, y - f, z - f }; }
Vector3 Vector3::operator*(F32 f) const { return { x * f, y * f, z * f }; }
Vector3 Vector3::operator/(F32 f) const { return { x / f, y / f, z / f }; }
Vector3 Vector3::operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f) }; }
Vector3 Vector3::operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
Vector3 Vector3::operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
Vector3 Vector3::operator*(const Vector3& v) const { return { x * v.x, y * v.y, z * v.z }; }
Vector3 Vector3::operator/(const Vector3& v) const { return { x / v.x, y / v.y, z / v.z }; }
Vector3 Vector3::operator%(const Vector3& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y), Math::Mod(z, v.z) }; }
Vector3 Vector3::operator*(const Quaternion3& q) const
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

bool Vector3::operator==(const Vector3& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y) && Math::Zero(z - v.z); }
bool Vector3::operator!=(const Vector3& v) const { return !Math::Zero(x - v.x) || !Math::Zero(y - v.y) || !Math::Zero(z - v.z); }
bool Vector3::operator>(const Vector3& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
bool Vector3::operator<(const Vector3& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
bool Vector3::operator>=(const Vector3& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
bool Vector3::operator<=(const Vector3& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
bool Vector3::IsZero() const { return Math::Zero(x) && Math::Zero(y) && Math::Zero(z); }

Vector3 Vector3::operator-() const { return { -x, -y, -z }; }
Vector3 Vector3::operator~() const { return { -x, -y, -z }; }
Vector3 Vector3::operator!() const { return { -x, -y, -z }; }

F32 Vector3::SqrMagnitude() const { return x * x + y * y + z * z; }
F32 Vector3::Magnitude() const { return Math::Sqrt(x * x + y * y + z * z); }
F32 Vector3::Dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
Vector3& Vector3::Normalize() { Vector3 v = Normalized(); x = v.x; y = v.y; z = v.z; return *this; }
Vector3 Vector3::Normalized() const { return IsZero() ? Vector3::Zero : (*this) / Magnitude(); }
Vector3 Vector3::Projection(const Vector3& v) const { return v * (Dot(v) / v.Dot(v)); }
Vector3 Vector3::OrthoProjection(const Vector3& v) const { return *this - Projection(v); }
Vector3 Vector3::Cross(const Vector3& v) const { return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x }; }

Vector3& Vector3::Clamp(const Vector3& min, const Vector3& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	return *this;
}

Vector3 Vector3::Clamped(const Vector3& min, const Vector3& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z)
	};
}

Vector3& Vector3::SetClosest(const Vector3& min, const Vector3& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	return *this;
}

Vector3 Vector3::Closest(const Vector3& min, const Vector3& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z)
	};
}

F32& Vector3::operator[] (U64 i) { return (&x)[i]; }
const F32& Vector3::operator[] (U64 i) const { return (&x)[i]; }

F32* Vector3::Data() { return &x; }
const F32* Vector3::Data() const { return &x; }

Vector3::operator String() const { return String(x, ", ", y, ", ", z); }
Vector3::operator String16() const { return String16(x, u", ", y, u", ", z); }
Vector3::operator String32() const { return String32(x, U", ", y, U", ", z); }

//Vector4

Vector4::Vector4() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 0.0f } {}
Vector4::Vector4(F32 f) : x{ f }, y{ f }, z{ f }, w{ f } {}
Vector4::Vector4(F32 x, F32 y, F32 z, F32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
Vector4::Vector4(const Vector4& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
Vector4::Vector4(Vector4&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}

Vector4& Vector4::operator=(F32 f) { x = f; y = f; z = f; w = f; return *this; }
Vector4& Vector4::operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
Vector4& Vector4::operator=(Vector4&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

Vector4& Vector4::operator+=(F32 f) { x += f; y += f; z += f; w += f; return *this; }
Vector4& Vector4::operator-=(F32 f) { x -= f; y -= f; z -= f; w -= f; return *this; }
Vector4& Vector4::operator*=(F32 f) { x *= f; y *= f; z *= f; w *= f; return *this; }
Vector4& Vector4::operator/=(F32 f) { x /= f; y /= f; z /= f; w /= f; return *this; }
Vector4& Vector4::operator%=(F32 f) { x = Math::Mod(x, f); y = Math::Mod(y, f); z = Math::Mod(z, f); w = Math::Mod(w, f); return *this; }
Vector4& Vector4::operator+=(const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
Vector4& Vector4::operator-=(const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
Vector4& Vector4::operator*=(const Vector4& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
Vector4& Vector4::operator/=(const Vector4& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
Vector4& Vector4::operator%=(const Vector4& v) { x = Math::Mod(x, v.x); y = Math::Mod(y, v.y); z = Math::Mod(z, v.z); w = Math::Mod(w, v.w); return *this; }

Vector4 Vector4::operator+(F32 f) const { return { x + f, y + f, z + f, w + f }; }
Vector4 Vector4::operator-(F32 f) const { return { x - f, y - f, z - f, w - f }; }
Vector4 Vector4::operator*(F32 f) const { return { x * f, y * f, z * f, w * f }; }
Vector4 Vector4::operator/(F32 f) const { return { x / f, y / f, z / f, w / f }; }
Vector4 Vector4::operator%(F32 f) const { return { Math::Mod(x, f), Math::Mod(y, f), Math::Mod(z, f), Math::Mod(w, f) }; }
Vector4 Vector4::operator+(const Vector4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
Vector4 Vector4::operator-(const Vector4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
Vector4 Vector4::operator*(const Vector4& v) const { return { x * v.x, y * v.y, z * v.z, w * v.w }; }
Vector4 Vector4::operator/(const Vector4& v) const { return { x / v.x, y / v.y, z / v.z, w / v.w }; }
Vector4 Vector4::operator%(const Vector4& v) const { return { Math::Mod(x, v.x), Math::Mod(y, v.y), Math::Mod(z, v.z), Math::Mod(w, v.w) }; }

bool Vector4::operator==(const Vector4& v) const { return Math::Zero(x - v.x) && Math::Zero(y - v.y) && Math::Zero(z - v.z) && Math::Zero(w - v.w); }
bool Vector4::operator!=(const Vector4& v) const { return !Math::Zero(x - v.x) || !Math::Zero(y - v.y) || !Math::Zero(z - v.z) || !Math::Zero(w - v.w); }
bool Vector4::operator>(const Vector4& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
bool Vector4::operator<(const Vector4& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
bool Vector4::operator>=(const Vector4& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
bool Vector4::operator<=(const Vector4& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
bool Vector4::IsZero() const { return Math::Zero(x) && Math::Zero(y) && Math::Zero(z) && Math::Zero(w); }

Vector4 Vector4::operator-() const { return { -x, -y, -z, -w }; }
Vector4 Vector4::operator~() const { return { -x, -y, -z, -w }; }
Vector4 Vector4::operator!() const { return { -x, -y, -z, -w }; }

F32 Vector4::SqrMagnitude() const { return x * x + y * y + z * z + w * w; }
F32 Vector4::Magnitude() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
F32 Vector4::Dot(const Vector4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
Vector4& Vector4::Normalize() { Vector4 v = Normalized(); x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
Vector4 Vector4::Normalized() const { return IsZero() ? Vector4::Zero : (*this) / Magnitude(); }
Vector4 Vector4::Projection(const Vector4& v) const { return v * (Dot(v) / v.Dot(v)); }
Vector4 Vector4::OrthoProjection(const Vector4& v) const { return *this - Projection(v); }

Vector4& Vector4::Clamp(const Vector4& min, const Vector4& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	w = Math::Clamp(w, min.w, max.w);
	return *this;
}

Vector4 Vector4::Clamped(const Vector4& min, const Vector4& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z),
		Math::Clamp(w, min.w, max.w)
	};
}

Vector4& Vector4::SetClosest(const Vector4& min, const Vector4& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	w = Math::Closest(w, min.w, max.w);
	return *this;
}

Vector4 Vector4::Closest(const Vector4& min, const Vector4& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z),
		Math::Closest(w, min.w, max.w)
	};
}

F32& Vector4::operator[] (U64 i) { return (&x)[i]; }
const F32& Vector4::operator[] (U64 i) const { return (&x)[i]; }

F32* Vector4::Data() { return &x; }
const F32* Vector4::Data() const { return &x; }

Vector4::operator String() const { return String(x, ", ", y, ", ", z, ", ", w); }
Vector4::operator String16() const { return String16(x, u", ", y, u", ", z, u", ", w); }
Vector4::operator String32() const { return String32(x, U", ", y, U", ", z, U", ", w); }

//Vector2Int

Vector2Int::Vector2Int() : x{ 0 }, y{ 0 } {}
Vector2Int::Vector2Int(I32 i) : x{ i }, y{ i } {}
Vector2Int::Vector2Int(I32 x, I32 y) : x{ x }, y{ y } {}
Vector2Int::Vector2Int(const Vector2Int& v) : x{ v.x }, y{ v.y } {}
Vector2Int::Vector2Int(Vector2Int&& v) noexcept : x{ v.x }, y{ v.y } {}

Vector2Int& Vector2Int::operator=(I32 i) { x = i; y = i; return *this; }
Vector2Int& Vector2Int::operator=(const Vector2Int& v) { x = v.x; y = v.y; return *this; }
Vector2Int& Vector2Int::operator=(Vector2Int&& v) noexcept { x = v.x; y = v.y; return *this; }

Vector2Int& Vector2Int::operator+=(I32 i) { x += i; y += i; return *this; }
Vector2Int& Vector2Int::operator-=(I32 i) { x -= i; y -= i; return *this; }
Vector2Int& Vector2Int::operator*=(I32 i) { x *= i; y *= i; return *this; }
Vector2Int& Vector2Int::operator/=(I32 i) { x /= i; y /= i; return *this; }
Vector2Int& Vector2Int::operator%=(I32 i) { x %= i; y %= i; return *this; }
Vector2Int& Vector2Int::operator+=(const Vector2Int& v) { x += v.x; y += v.y; return *this; }
Vector2Int& Vector2Int::operator-=(const Vector2Int& v) { x -= v.x; y -= v.y; return *this; }
Vector2Int& Vector2Int::operator*=(const Vector2Int& v) { x *= v.x; y *= v.y; return *this; }
Vector2Int& Vector2Int::operator/=(const Vector2Int& v) { x /= v.x; y /= v.y; return *this; }
Vector2Int& Vector2Int::operator%=(const Vector2Int& v) { x &= v.x; y &= v.y; return *this; }

Vector2Int Vector2Int::operator+(I32 i) const { return { x + i, y + i }; }
Vector2Int Vector2Int::operator-(I32 i) const { return { x - i, y - i }; }
Vector2Int Vector2Int::operator*(I32 i) const { return { x * i, y * i }; }
Vector2Int Vector2Int::operator/(I32 i) const { return { x / i, y / i }; }
Vector2Int Vector2Int::operator%(I32 i) const { return { x % i, y % i }; }
Vector2Int Vector2Int::operator+(const Vector2Int& v) const { return { x + v.x, y + v.y }; }
Vector2Int Vector2Int::operator-(const Vector2Int& v) const { return { x - v.x, y - v.y }; }
Vector2Int Vector2Int::operator*(const Vector2Int& v) const { return { x * v.x, y * v.y }; }
Vector2Int Vector2Int::operator/(const Vector2Int& v) const { return { x / v.x, y / v.y }; }
Vector2Int Vector2Int::operator%(const Vector2Int& v) const { return { x % v.x, y % v.y }; }

bool Vector2Int::operator==(const Vector2Int& v) const { return !(x - v.x) && !(y - v.y); }
bool Vector2Int::operator!=(const Vector2Int& v) const { return (x - v.x) || (y - v.y); }
bool Vector2Int::operator>(const Vector2Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
bool Vector2Int::operator<(const Vector2Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
bool Vector2Int::operator>=(const Vector2Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
bool Vector2Int::operator<=(const Vector2Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
bool Vector2Int::IsZero() const { return !x && !y; }

Vector2Int Vector2Int::operator-() const { return { -x, -y }; }
Vector2Int Vector2Int::operator~() const { return { -x, -y }; }
Vector2Int Vector2Int::operator!() const { return { -x, -y }; }

I32 Vector2Int::SqrMagnitude() const { return x * x + y * y; }
F32 Vector2Int::Magnitude() const { return Math::Sqrt((F32)(x * x + y * y)); }
I32 Vector2Int::Dot(const Vector2Int& v) const { return x * v.x + y * v.y; }

Vector2Int& Vector2Int::Clamp(const Vector2Int& min, const Vector2Int& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	return *this;
}

Vector2Int Vector2Int::Clamped(const Vector2Int& min, const Vector2Int& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y)
	};
}

Vector2Int& Vector2Int::SetClosest(const Vector2Int& min, const Vector2Int& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	return *this;
}

Vector2Int Vector2Int::Closest(const Vector2Int& min, const Vector2Int& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y)
	};
}

I32& Vector2Int::operator[] (U64 i) { return (&x)[i]; }
const I32& Vector2Int::operator[] (U64 i) const { return (&x)[i]; }

I32* Vector2Int::Data() { return &x; }
const I32* Vector2Int::Data() const { return &x; }

Vector2Int::operator String() const { return String(x, ", ", y); }
Vector2Int::operator String16() const { return String16(x, u", ", y); }
Vector2Int::operator String32() const { return String32(x, U", ", y); }

//Vector3Int

Vector3Int::Vector3Int() : x{ 0 }, y{ 0 }, z{ 0 } {}
Vector3Int::Vector3Int(I32 i) : x{ i }, y{ i }, z{ i } {}
Vector3Int::Vector3Int(I32 x, I32 y, I32 z) : x{ x }, y{ y }, z{ z } {}
Vector3Int::Vector3Int(const Vector3Int& v) : x{ v.x }, y{ v.y }, z{ v.z } {}
Vector3Int::Vector3Int(Vector3Int&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z } {}

Vector3Int& Vector3Int::operator=(I32 i) { x = i; y = i; z = i; return *this; }
Vector3Int& Vector3Int::operator=(const Vector3Int& v) { x = v.x; y = v.y; z = v.z; return *this; }
Vector3Int& Vector3Int::operator=(Vector3Int&& v) noexcept { x = v.x; y = v.y; z = v.z; return *this; }

Vector3Int& Vector3Int::operator+=(I32 i) { x += i; y += i; z += i; return *this; }
Vector3Int& Vector3Int::operator-=(I32 i) { x -= i; y -= i; z -= i; return *this; }
Vector3Int& Vector3Int::operator*=(I32 i) { x *= i; y *= i; z *= i; return *this; }
Vector3Int& Vector3Int::operator/=(I32 i) { x /= i; y /= i; z /= i; return *this; }
Vector3Int& Vector3Int::operator%=(I32 i) { x %= i; y %= i; z %= i; return *this; }
Vector3Int& Vector3Int::operator+=(const Vector3Int& v) { x += v.x; y += v.y; z += v.z; return *this; }
Vector3Int& Vector3Int::operator-=(const Vector3Int& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
Vector3Int& Vector3Int::operator*=(const Vector3Int& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
Vector3Int& Vector3Int::operator/=(const Vector3Int& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
Vector3Int& Vector3Int::operator%=(const Vector3Int& v) { x %= v.x; y %= v.y; z %= v.z; return *this; }

Vector3Int Vector3Int::operator+(I32 i) const { return { x + i, y + i, z + i }; }
Vector3Int Vector3Int::operator-(I32 i) const { return { x - i, y - i, z - i }; }
Vector3Int Vector3Int::operator*(I32 i) const { return { x * i, y * i, z * i }; }
Vector3Int Vector3Int::operator/(I32 i) const { return { x / i, y / i, z / i }; }
Vector3Int Vector3Int::operator%(I32 i) const { return { x % i, y % i, z % i }; }
Vector3Int Vector3Int::operator+(const Vector3Int& v) const { return { x + v.x, y + v.y, z + v.z }; }
Vector3Int Vector3Int::operator-(const Vector3Int& v) const { return { x - v.x, y - v.y, z - v.z }; }
Vector3Int Vector3Int::operator*(const Vector3Int& v) const { return { x * v.x, y * v.y, z * v.z }; }
Vector3Int Vector3Int::operator/(const Vector3Int& v) const { return { x / v.x, y / v.y, z / v.z }; }
Vector3Int Vector3Int::operator%(const Vector3Int& v) const { return { x % v.x, y % v.y, z % v.z }; }

bool Vector3Int::operator==(const Vector3Int& v) const { return !(x - v.x) && !(y - v.y) && !(z - v.z); }
bool Vector3Int::operator!=(const Vector3Int& v) const { return (x - v.x) || (y - v.y) || (z - v.z); }
bool Vector3Int::operator>(const Vector3Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
bool Vector3Int::operator<(const Vector3Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
bool Vector3Int::operator>=(const Vector3Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
bool Vector3Int::operator<=(const Vector3Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
bool Vector3Int::IsZero() const { return !x && !y && !z; }

Vector3Int Vector3Int::operator-() const { return { -x, -y, -z }; }
Vector3Int Vector3Int::operator~() const { return { -x, -y, -z }; }
Vector3Int Vector3Int::operator!() const { return { -x, -y, -z }; }

I32 Vector3Int::SqrMagnitude() const { return x * x + y * y + z * z; }
F32 Vector3Int::Magnitude() const { return Math::Sqrt((F32)(x * x + y * y + z * z)); }
I32 Vector3Int::Dot(const Vector3Int& v) const { return x * v.x + y * v.y + z * v.z; }

Vector3Int& Vector3Int::Clamp(const Vector3Int& min, const Vector3Int& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	return *this;
}

Vector3Int Vector3Int::Clamped(const Vector3Int& min, const Vector3Int& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z)
	};
}

Vector3Int& Vector3Int::SetClosest(const Vector3Int& min, const Vector3Int& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	return *this;
}

Vector3Int Vector3Int::Closest(const Vector3Int& min, const Vector3Int& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z)
	};
}

I32& Vector3Int::operator[] (U64 i) { return (&x)[i]; }
const I32& Vector3Int::operator[] (U64 i) const { return (&x)[i]; }

I32* Vector3Int::Data() { return &x; }
const I32* Vector3Int::Data() const { return &x; }

Vector3Int::operator String() const { return String(x, ", ", y, ", ", z); }
Vector3Int::operator String16() const { return String16(x, u", ", y, u", ", z); }
Vector3Int::operator String32() const { return String32(x, U", ", y, U", ", z); }

//Vector4Int

Vector4Int::Vector4Int() : x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 } {}
Vector4Int::Vector4Int(I32 i) : x{ i }, y{ i }, z{ i }, w{ i } {}
Vector4Int::Vector4Int(I32 x, I32 y, I32 z, I32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}
Vector4Int::Vector4Int(const Vector4Int& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}
Vector4Int::Vector4Int(Vector4Int&& v) noexcept : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {}

Vector4Int& Vector4Int::operator=(I32 i) { x = i; y = i; z = i; w = i; return *this; }
Vector4Int& Vector4Int::operator=(const Vector4Int& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
Vector4Int& Vector4Int::operator=(Vector4Int&& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

Vector4Int& Vector4Int::operator+=(I32 i) { x += i; y += i; z += i; w += i; return *this; }
Vector4Int& Vector4Int::operator-=(I32 i) { x -= i; y -= i; z -= i; w -= i; return *this; }
Vector4Int& Vector4Int::operator*=(I32 i) { x *= i; y *= i; z *= i; w *= i; return *this; }
Vector4Int& Vector4Int::operator/=(I32 i) { x /= i; y /= i; z /= i; w /= i; return *this; }
Vector4Int& Vector4Int::operator%=(I32 i) { x &= i; y %= i; z %= i; w %= i; return *this; }
Vector4Int& Vector4Int::operator+=(const Vector4Int& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
Vector4Int& Vector4Int::operator-=(const Vector4Int& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
Vector4Int& Vector4Int::operator*=(const Vector4Int& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
Vector4Int& Vector4Int::operator/=(const Vector4Int& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
Vector4Int& Vector4Int::operator%=(const Vector4Int& v) { x %= v.x; y %= v.y; z %= v.z; w %= v.w; return *this; }

Vector4Int Vector4Int::operator+(I32 i) const { return { x + i, y + i, z + i, w + i }; }
Vector4Int Vector4Int::operator-(I32 i) const { return { x - i, y - i, z - i, w - i }; }
Vector4Int Vector4Int::operator*(I32 i) const { return { x * i, y * i, z * i, w * i }; }
Vector4Int Vector4Int::operator/(I32 i) const { return { x / i, y / i, z / i, w / i }; }
Vector4Int Vector4Int::operator%(I32 i) const { return { x % i, y % i, z % i, w % i }; }
Vector4Int Vector4Int::operator+(const Vector4Int& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
Vector4Int Vector4Int::operator-(const Vector4Int& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
Vector4Int Vector4Int::operator*(const Vector4Int& v) const { return { x * v.x, y * v.y, z * v.z, w * v.w }; }
Vector4Int Vector4Int::operator/(const Vector4Int& v) const { return { x / v.x, y / v.y, z / v.z, w / v.w }; }
Vector4Int Vector4Int::operator%(const Vector4Int& v) const { return { x % v.x, y % v.y, z % v.z, w % v.w }; }

bool Vector4Int::operator==(const Vector4Int& v) const { return !(x - v.x) && !(y - v.y) && (z - v.z) && !(w - v.w); }
bool Vector4Int::operator!=(const Vector4Int& v) const { return (x - v.x) || (y - v.y) || (z - v.z) || (w - v.w); }
bool Vector4Int::operator>(const Vector4Int& v) const { return SqrMagnitude() > v.SqrMagnitude(); }
bool Vector4Int::operator<(const Vector4Int& v) const { return SqrMagnitude() < v.SqrMagnitude(); }
bool Vector4Int::operator>=(const Vector4Int& v) const { return *this == v || SqrMagnitude() > v.SqrMagnitude(); }
bool Vector4Int::operator<=(const Vector4Int& v) const { return *this == v || SqrMagnitude() < v.SqrMagnitude(); }
bool Vector4Int::IsZero() const { return !x && !y && !z && !w; }

Vector4Int Vector4Int::operator-() const { return { -x, -y, -z, -w }; }
Vector4Int Vector4Int::operator~() const { return { -x, -y, -z, -w }; }
Vector4Int Vector4Int::operator!() const { return { -x, -y, -z, -w }; }

I32 Vector4Int::SqrMagnitude() const { return x * x + y * y + z * z + w * w; }
F32 Vector4Int::Magnitude() const { return Math::Sqrt((F32)(x * x + y * y + z * z + w * w)); }
I32 Vector4Int::Dot(const Vector4Int& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

Vector4Int& Vector4Int::Clamp(const Vector4Int& min, const Vector4Int& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	z = Math::Clamp(z, min.z, max.z);
	w = Math::Clamp(w, min.w, max.w);
	return *this;
}

Vector4Int Vector4Int::Clamped(const Vector4Int& min, const Vector4Int& max) const
{
	return {
		Math::Clamp(x, min.x, max.x),
		Math::Clamp(y, min.y, max.y),
		Math::Clamp(z, min.z, max.z),
		Math::Clamp(w, min.w, max.w)
	};
}

Vector4Int& Vector4Int::SetClosest(const Vector4Int& min, const Vector4Int& max)
{
	x = Math::Closest(x, min.x, max.x);
	y = Math::Closest(y, min.y, max.y);
	z = Math::Closest(z, min.z, max.z);
	w = Math::Closest(w, min.w, max.w);
	return *this;
}

Vector4Int Vector4Int::Closest(const Vector4Int& min, const Vector4Int& max) const
{
	return {
		Math::Closest(x, min.x, max.x),
		Math::Closest(y, min.y, max.y),
		Math::Closest(z, min.z, max.z),
		Math::Closest(w, min.w, max.w)
	};
}

I32& Vector4Int::operator[] (U64 i) { return (&x)[i]; }
const I32& Vector4Int::operator[] (U64 i) const { return (&x)[i]; }

I32* Vector4Int::Data() { return &x; }
const I32* Vector4Int::Data() const { return &x; }

Vector4Int::operator String() const { return String(x, ", ", y, ", ", z, ", ", w); }
Vector4Int::operator String16() const { return String16(x, u", ", y, u", ", z, u", ", w); }
Vector4Int::operator String32() const { return String32(x, U", ", y, U", ", z, U", ", w); }

//Matrix3

Matrix3::Matrix3() : a{ 1.0f, 0.0f, 0.0f }, b{ 0.0f, 1.0f, 0.0f }, c{ 0.0f, 0.0f, 1.0f } {}
Matrix3::Matrix3(F32 ax, F32 ay, F32 az, F32 bx, F32 by, F32 bz, F32 cx, F32 cy, F32 cz) : a{ ax, ay, az }, b{ bx, by, bz }, c{ cx, cy, cz } {}
Matrix3::Matrix3(const Vector3& a, const Vector3& b, const Vector3& c) : a{ a }, b{ b }, c{ c } {}
Matrix3::Matrix3(Vector3&& v1, Vector3&& v2, Vector3&& v3) noexcept : a{ v1 }, b{ v2 }, c{ v3 } {}
Matrix3::Matrix3(const Matrix3& m) : a{ m.a }, b{ m.b }, c{ m.c } {}
Matrix3::Matrix3(Matrix3&& m) noexcept : a{ m.a }, b{ m.b }, c{ m.c } {}

Matrix3::Matrix3(const Vector2& position, const F32& rotation, const Vector2& scale)
{
	F32 cos = Math::Cos(rotation * DEG_TO_RAD_F);
	F32 sin = Math::Sin(rotation * DEG_TO_RAD_F);
	a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
	a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
	a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
}

Matrix3::Matrix3(const Vector2& position, const Quaternion2& rotation, const Vector2& scale)
{

}

void Matrix3::Set(const Vector2& position, const Quaternion2& rotation, const Vector2& scale)
{

}

void Matrix3::Set(const Vector2& position, const F32& rotation, const Vector2& scale)
{
	F32 cos = Math::Cos(rotation * DEG_TO_RAD_F);
	F32 sin = Math::Sin(rotation * DEG_TO_RAD_F);
	a.x = cos * scale.x;	b.x = -sin;				c.x = position.x;
	a.y = sin;				b.y = cos * scale.y;	c.y = position.y;
	a.z = 0.0f;				b.z = 0.0f;				c.z = 1.0f;
}

Matrix3& Matrix3::operator= (const Matrix3& m) { a = m.a; b = m.b; c = m.c; return *this; }
Matrix3& Matrix3::operator= (Matrix3&& m) noexcept { a = m.a; b = m.b; c = m.c; return *this; }

Matrix3& Matrix3::operator+= (const Matrix3& m) { a += m.a; b += m.b; c += m.c; return *this; }
Matrix3& Matrix3::operator-= (const Matrix3& m) { a -= m.a; b -= m.b; c -= m.c; return *this; }
Matrix3& Matrix3::operator*= (const Matrix3& m)
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

Matrix3 Matrix3::operator+(const Matrix3& m) const { return { a + m.a, b + m.b, c + m.c }; }
Matrix3 Matrix3::operator-(const Matrix3& m) const { return { a - m.a, b - m.b, c - m.c }; }
Matrix3 Matrix3::operator*(const Matrix3& m) const
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
Vector3 Matrix3::operator*(const Vector3& v) const
{
	return {
		a.x * v.x + b.x * v.y + c.x * v.z,
		a.y * v.x + b.y * v.y + c.y * v.z,
		a.z * v.x + b.z * v.y + c.z * v.z
	};
}

Matrix3 Matrix3::operator-() const { return { -a, -b, -c }; }
Matrix3 Matrix3::operator~() const { return { -a, -b, -c }; }
Matrix3 Matrix3::operator!() const { return { -a, -b, -c }; }
bool Matrix3::operator==(const Matrix3& m) const { return a == m.a && b == m.b && c == m.c; }
bool Matrix3::operator!=(const Matrix3& m) const { return a != m.a || b != m.b || c != m.c; }

const Vector3& Matrix3::operator[] (U8 i) const { return (&a)[i]; }
Vector3& Matrix3::operator[] (U8 i) { return (&a)[i]; }

F32* Matrix3::Data() { return a.Data(); }
const F32* Matrix3::Data() const { return a.Data(); }

//Matrix4

Matrix4::Matrix4() : a{ 1.0f, 0.0f, 0.0f, 0.0f }, b{ 0.0f, 1.0f, 0.0f, 0.0f }, c{ 0.0f, 0.0f, 1.0f, 0.0f }, d{ 0.0f, 0.0f, 0.0f, 1.0f } {}
Matrix4::Matrix4(F32 ax, F32 ay, F32 az, F32 aw, F32 bx, F32 by, F32 bz, F32 bw, F32 cx, F32 cy, F32 cz, F32 cw, F32 dx, F32 dy, F32 dz, F32 dw) :
	a{ ax, ay, az, aw }, b{ bx, by, bz, bw }, c{ cx, cy, cz, cw }, d{ dx, dy, dz, dw }
{
}
Matrix4::Matrix4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d) : a{ a }, b{ b }, c{ c }, d{ d } {}
Matrix4::Matrix4(Vector4&& a, Vector4&& b, Vector4&& c, Vector4&& d) noexcept : a{ a }, b{ b }, c{ c }, d{ d } {}
Matrix4::Matrix4(const Matrix4& m) : a{ m.a }, b{ m.b }, c{ m.c }, d{ m.d } {}
Matrix4::Matrix4(const Matrix3& m) : a{ m.a.x, m.a.y, 0.0f, m.a.z }, b{ m.b.x, m.b.y, 0.0f, m.b.z }, c{ 0.0f, 0.0f, 1.0f, m.b.z }, d{ m.c.x, m.c.y, 0.0f, 1.0f } {}
Matrix4::Matrix4(Matrix4&& m) noexcept : a{ m.a }, b{ m.b }, c{ m.c }, d{ m.d } {}

Matrix4::Matrix4(const Vector3& position, const Vector3& rotation, const Vector3& scale)
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

Matrix4::Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
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

Matrix4& Matrix4::operator= (const Matrix4& m) { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
Matrix4& Matrix4::operator= (Matrix4&& m) noexcept { a = m.a; b = m.b; c = m.c; d = m.d; return *this; }
Matrix4& Matrix4::operator+= (const Matrix4& m) { a += m.a; b += m.b; c += m.c; d += m.d; return *this; }
Matrix4& Matrix4::operator-= (const Matrix4& m) { a -= m.a; b -= m.b; c -= m.c; d -= m.d; return *this; }

Matrix4& Matrix4::operator*= (const Matrix4& m)
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

Matrix4 Matrix4::operator+(const Matrix4& m) const { return { a + m.a, b + m.b, c + m.c, d + m.d }; }
Matrix4 Matrix4::operator-(const Matrix4& m) const { return { a - m.a, b - m.b, c - m.c, d + m.d }; }

Matrix4 Matrix4::operator*(const Matrix4& m) const
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

Vector2 Matrix4::operator*(const Vector2& v) const
{
	return {
		a.x * v.x + b.x * v.y,
		a.y * v.x + b.y * v.y
	};
}

Vector3 Matrix4::operator*(const Vector3& v) const
{
	return {
		a.x * v.x + b.x * v.y + c.x * v.z,
		a.y * v.x + b.y * v.y + c.y * v.z,
		a.z * v.x + b.z * v.y + c.z * v.z
	};
}

Vector4 Matrix4::operator*(const Vector4& v) const
{
	return {
		a.x * v.x + b.x * v.y + c.x * v.z + c.x * v.w,
		a.y * v.x + b.y * v.y + c.y * v.z + c.y * v.w,
		a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.w,
		a.z * v.x + b.z * v.y + c.z * v.z + c.z * v.w
	};
}

Matrix4 Matrix4::Inverse() const
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

	m.a.x = (t0 * b.y + t3 * c.y + t4 * d.y) - (t1 * b.y + t2 * c.y + t5 * d.y);
	m.a.y = (t1 * a.y + t6 * c.y + t9 * d.y) - (t0 * a.y + t7 * c.y + t8 * d.y);
	m.a.z = (t2 * a.y + t7 * b.y + t10 * d.y) - (t3 * a.y + t6 * b.y + t11 * d.y);
	m.a.w = (t5 * a.y + t8 * b.y + t11 * c.y) - (t4 * a.y + t9 * b.y + t10 * c.y);

	F32 determinant = (a.x * m.a.x + b.x * m.a.y + c.x * m.a.z + d.x * m.a.w);
	if (Math::Zero(determinant)) { return Identity; }
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

Matrix4& Matrix4::Inversed()
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
	if (Math::Zero(determinant)) { return *this = Identity; }
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

Matrix4 Matrix4::Invert() const
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

Matrix4& Matrix4::Inverted()
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

Matrix4 Matrix4::Transpose() const
{
	return {
		a.x, b.x, c.x, d.x,
		a.y, b.y, c.y, d.y,
		a.z, b.z, c.z, d.z,
		a.w, b.w, c.w, c.z
	};
}

Matrix4& Matrix4::Transposed()
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

void Matrix4::Set(const Vector3& position, const Vector3& rotation, const Vector3& scale)
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

void Matrix4::Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
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

void Matrix4::SetPerspective(F32 fov, F32 aspect, F32 near, F32 far)
{
	F32 yScale = 1.0f / Math::Tan(fov * DEG_TO_RAD_F * 0.5f);
	F32 xScale = yScale / aspect;
	F32 nearFar = 1.0f / (near - far);

	a.x = xScale;
	a.y = 0.0f;
	a.z = 0.0f;
	a.w = 0.0f;

	b.x = 0.0f;
	b.y = yScale;
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

void Matrix4::SetOrthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far)
{
	F32 rightLeft = 1.0f / (right - left);
	F32 topBottom = 1.0f / (top - bottom);
	F32 farNear = -1.0f / (far - near);

	a.x = 2.0f * rightLeft;
	a.y = 0.0f;
	a.z = 0.0f;
	a.w = -(right + left) * rightLeft;

	b.x = 0.0f;
	b.y = 2.0f * topBottom;
	b.z = 0.0f;
	b.w = -(top + bottom) * topBottom;

	c.x = 0.0f;
	c.y = 0.0f;
	c.z = 2.0f * farNear;
	c.w = (far + near) * farNear;

	d.x = 0.0f;
	d.y = 0.0f;
	d.z = 0.0f;
	d.w = 1.0f;
}

void Matrix4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	Vector3 f = center - eye;
	f.Normalize();

	Vector3 s = f.Cross(up);
	s.Normalize();

	Vector3 u = s.Cross(f);

	a.x = s.x;
	a.y = u.x;
	a.z = f.x;
	a.w = 0.0f;

	b.x = s.y;
	b.y = u.y;
	b.z = f.y;
	b.w = 0.0f;

	c.x = s.z;
	c.y = u.z;
	c.z = f.z;
	c.w = 0.0f;

	d.x = -s.Dot(eye);
	d.y = -u.Dot(eye);
	d.z = -f.Dot(eye);
	d.w = 1.0f;
}

Vector3 Matrix4::Forward() { return Vector3(-a.z, -b.z, -c.z).Normalize(); }
Vector3 Matrix4::Back() { return Vector3(a.z, b.z, c.z).Normalize(); }
Vector3 Matrix4::Right() { return Vector3(a.x, b.x, c.x).Normalize(); }
Vector3 Matrix4::Left() { return Vector3(-a.x, -b.x, -c.x).Normalize(); }
Vector3 Matrix4::Up() { return Vector3(a.y, b.y, c.y).Normalize(); }
Vector3 Matrix4::Down() { return Vector3(-a.y, -b.y, -c.y).Normalize(); }

Matrix4 Matrix4::operator-() { return { -a, -b, -c, -d }; }
Matrix4 Matrix4::operator~() { return { -a, -b, -c, -d }; }
Matrix4 Matrix4::operator!() { return { -a, -b, -c, -d }; }

bool Matrix4::operator==(const Matrix4& m) const { return a == m.a && b == m.b && c == m.c && d == m.d; }
bool Matrix4::operator!=(const Matrix4& m) const { return a != m.a || b != m.b || c != m.c || d != m.d; }

Vector4& Matrix4::operator[] (U8 i) { return (&a)[i]; }
const Vector4& Matrix4::operator[] (U8 i) const { return (&a)[i]; }

F32* Matrix4::Data() { return a.Data(); }
const F32* Matrix4::Data() const { return a.Data(); }

//Quaternion2

Quaternion2::Quaternion2() : x{ 0.0f }, y{ 1.0f } {}
Quaternion2::Quaternion2(F32 x, F32 y) : x{ x }, y{ y } {}
Quaternion2::Quaternion2(F32 angle) { F32 a = angle * DEG_TO_RAD_F; x = Math::Sin(a); y = Math::Cos(a); }
Quaternion2::Quaternion2(const Quaternion2& q) : x{ q.x }, y{ q.y } {}
Quaternion2::Quaternion2(Quaternion2&& q) noexcept : x{ q.x }, y{ q.y } {}

Quaternion2& Quaternion2::operator=(F32 angle)
{
	F32 a = angle * DEG_TO_RAD_F;
	x = Math::Sin(a);
	y = Math::Cos(a);
	return *this;
}

Quaternion2& Quaternion2::operator=(const Quaternion2& q) { x = q.y; y = q.y; return *this; }
Quaternion2& Quaternion2::operator=(Quaternion2&& q) noexcept { x = q.y; y = q.y; return *this; }

Quaternion2& Quaternion2::operator+=(const Quaternion2& q)
{
	x += q.x;
	y += q.y;

	return *this;
}

Quaternion2& Quaternion2::operator-=(const Quaternion2& q)
{
	x -= q.x;
	y -= q.y;

	return *this;
}

Quaternion2& Quaternion2::operator*=(const Quaternion2& q)
{
	x = y * q.x + x * q.y;
	y = y * q.y - x * q.x;

	return *this;
}

Quaternion2& Quaternion2::operator/=(const Quaternion2& q)
{
	F32 n2 = 1.0f / q.SqrNormal();

	x = (-y * q.x + x * q.y) * n2;
	y = (y * q.y + x * q.x) * n2;

	return *this;
}

Quaternion2 Quaternion2::operator+(const Quaternion2& q) const
{
	return {
		x + q.x,
		y + q.y
	};
}

Quaternion2 Quaternion2::operator-(const Quaternion2& q) const
{
	return {
		x - q.x,
		y - q.y
	};
}

Quaternion2 Quaternion2::operator*(const Quaternion2& q) const
{
	return {
		y * q.x + x * q.y,
		y * q.y - x * q.x,
	};
}

Quaternion2 Quaternion2::operator/(const Quaternion2& q) const
{
	F32 n2 = 1.0f / q.SqrNormal();

	return {
		(x * q.y - y * q.x) * n2,
		(y * q.y + x * q.x) * n2
	};
}

void Quaternion2::Set(F32 angle)
{
	F32 a = angle * DEG_TO_RAD_F;
	x = Math::Sin(a);
	y = Math::Cos(a);
}

void Quaternion2::Rotate(F32 angle)
{
	F32 a = angle * DEG_TO_RAD_F;
	x += Math::Sin(a);
	y += Math::Cos(a);
}

F32 Quaternion2::Angle() const
{
	return Math::Asin(x);
}

Quaternion2 Quaternion2::Slerp(const Quaternion2& q, F32 t) const
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

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

Quaternion2& Quaternion2::Slerped(const Quaternion2& q, F32 t)
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

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

F32 Quaternion2::Dot(const Quaternion2& q) const
{
	return x * q.x + y * q.y;
}

F32 Quaternion2::SqrNormal() const
{
	return x * x + y * y;
}

F32 Quaternion2::Normal() const
{
	return Math::Sqrt(x * x + y * y);
}

Quaternion2 Quaternion2::Normalize() const { F32 n = 1.0f / Normal(); return { x * n, y * n }; }
Quaternion2& Quaternion2::Normalized() { F32 n = 1.0f / Normal(); x *= n; y *= n; return *this; }
Quaternion2 Quaternion2::Conjugate() const { return { -x, y }; }
Quaternion2& Quaternion2::Conjugated() { x = -x; return *this; }

Quaternion2 Quaternion2::Inverse() const
{
	F32 n = 1.0f / Math::Sqrt(x * x + y * y);
	return { -x * n, y * n };
}

Quaternion2& Quaternion2::Inversed() { return Conjugated().Normalized(); }

F32& Quaternion2::operator[] (U8 i) { return (&x)[i]; }
const F32& Quaternion2::operator[] (U8 i) const { return (&x)[i]; }

F32* Quaternion2::Data() { return &x; }
const F32* Quaternion2::Data() const { return &x; }

//Quaternion3

Quaternion3::Quaternion3() : x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 1.0f } {}
Quaternion3::Quaternion3(F32 x, F32 y, F32 z, F32 w) : x{ x }, y{ y }, z{ z }, w{ w } {}

Quaternion3::Quaternion3(const Vector3& euler)
{
	F32 hx = euler.x * 0.5f;
	F32 hy = euler.y * 0.5f;
	F32 hz = euler.z * 0.5f;

	F32 c0 = Math::Cos(hx);
	F32 c1 = Math::Cos(hy);
	F32 c2 = Math::Cos(hz);
	F32 s0 = Math::Sin(hx);
	F32 s1 = Math::Sin(hy);
	F32 s2 = Math::Sin(hz);

	F32 c0c1 = c0 * c1;
	F32 s0s1 = s0 * s1;
	F32 s0c1 = s0 * c1;
	F32 c0s1 = c0 * s1;

	x = c0c1 * c2 + s0s1 * s2;
	y = s0c1 * c2 - c0c1 * s2;
	z = c0s1 * c2 + s0c1 * s2;
	w = c0c1 * s2 - s0s1 * c2;
}

Quaternion3::Quaternion3(const Vector3& axis, F32 angle)
{
	const F32 halfAngle = angle * 0.5f;
	F32 s = Math::Sin(halfAngle);
	F32 c = Math::Cos(halfAngle);

	x = s * axis.x;
	y = s * axis.y;
	z = s * axis.z;
	w = c;
}

Quaternion3::Quaternion3(const Quaternion3& q) : x{ q.x }, y{ q.y }, z{ q.z }, w{ q.w } {}
Quaternion3::Quaternion3(Quaternion3&& q) noexcept : x{ q.x }, y{ q.y }, z{ q.z }, w{ q.w } {}

Quaternion3& Quaternion3::operator=(const Vector3& euler)
{
	F32 hx = euler.x * 0.5f;
	F32 hy = euler.y * 0.5f;
	F32 hz = euler.z * 0.5f;

	F32 c0 = Math::Cos(hx);
	F32 c1 = Math::Cos(hy);
	F32 c2 = Math::Cos(hz);
	F32 s0 = Math::Sin(hx);
	F32 s1 = Math::Sin(hy);
	F32 s2 = Math::Sin(hz);

	F32 c0c1 = c0 * c1;
	F32 s0s1 = s0 * s1;
	F32 s0c1 = s0 * c1;
	F32 c0s1 = c0 * s1;

	x = c0c1 * c2 + s0s1 * s2;
	y = s0c1 * c2 - c0c1 * s2;
	z = c0s1 * c2 + s0c1 * s2;
	w = c0c1 * s2 - s0s1 * c2;

	return *this;
}

Quaternion3& Quaternion3::operator=(const Quaternion3& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
Quaternion3& Quaternion3::operator=(Quaternion3&& q) noexcept { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }

Quaternion3& Quaternion3::operator+=(const Quaternion3& q)
{
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;

	return *this;
}

Quaternion3& Quaternion3::operator-=(const Quaternion3& q)
{
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;

	return *this;
}

Quaternion3& Quaternion3::operator*=(const Quaternion3& q)
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

Quaternion3& Quaternion3::operator/=(const Quaternion3& q)
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

Quaternion3 Quaternion3::operator+(const Quaternion3& q) const
{
	return {
		x + q.x,
		y + q.y,
		z + q.z,
		w + q.w
	};
}

Quaternion3 Quaternion3::operator-(const Quaternion3& q) const
{
	return {
		x - q.x,
		y - q.y,
		z - q.z,
		w - q.w
	};
}

Quaternion3 Quaternion3::operator*(const Quaternion3& q) const
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

Quaternion3 Quaternion3::operator/(const Quaternion3& q) const
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

Matrix3 Quaternion3::ToMatrix3() const
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

Matrix4 Quaternion3::ToMatrix4() const
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

Matrix4 Quaternion3::RotationMatrix(Vector3 center) const
{
	Matrix4 matrix = Matrix4::Identity;

	F32 xx = x * x;
	F32 xy = x * y;
	F32 xz = x * z;
	F32 xw = x * w;
	F32 yy = y * y;
	F32 yz = y * z;
	F32 yw = y * w;
	F32 zz = z * z;
	F32 ww = w * w;

	matrix[0][0] = xx - yy - zz + ww;
	matrix[1][0] = 2.0f * (xy + xw);
	matrix[2][0] = 2.0f * (xz - yw);
	matrix[3][0] = center.x - center.x * matrix[0][0] - center.y * matrix[1][0] - center.z * matrix[2][0];

	matrix[0][1] = 2.0f * (xy - xw);
	matrix[1][1] = -xx + yy - zz + ww;
	matrix[2][1] = 2.0f * (yz + xw);
	matrix[3][1] = center.y - center.x * matrix[0][1] - center.y * matrix[1][1] - center.z * matrix[2][1];

	matrix[0][2] = 2.0f * (xz + yw);
	matrix[1][2] = 2.0f * (yz - xw);
	matrix[2][2] = -xx - yy + zz + ww;
	matrix[3][2] = center.z - center.x * matrix[0][2] - center.y * matrix[1][2] - center.z * matrix[2][2];

	return matrix;
}

Vector3 Quaternion3::Euler() const
{
	F32 v = x * y + z * w;

	if (Math::Abs(v - 0.5f) < FLOAT_EPSILON) { return { 2.0f * Math::Atan2(x, w), HALF_PI_F, 0.0f }; }
	if (Math::Abs(v + 0.5f) < FLOAT_EPSILON) { return { -2.0f * Math::Atan2(x, w), -HALF_PI_F, 0.0f }; }

	return { Math::Atan2(2.0f * (w * y - x * z), 1.0f - 2.0f * (y * y + z * z)),
			Math::Asin(2.0f * v),
			Math::Atan2(2.0f * (w * x - y * z), 1.0f - 2.0f * (x * x + z * z)) };
}

Quaternion3 Quaternion3::Slerp(const Quaternion3& q, F32 t) const
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

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

Quaternion3& Quaternion3::Slerped(const Quaternion3& q, F32 t)
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

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

F32 Quaternion3::Dot(const Quaternion3& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
F32 Quaternion3::SqrNormal() const { return x * x + y * y + z * z + w * w; }
F32 Quaternion3::Normal() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
Quaternion3 Quaternion3::Normalize() const { F32 n = 1.0f / Normal(); return { x * n, y * n, z * n, w * n }; }
Quaternion3& Quaternion3::Normalized() { F32 n = 1.0f / Normal(); x *= n; y *= n; z *= n; w *= n; return *this; }
Quaternion3 Quaternion3::Conjugate() const { return { -x, -y, -z, w }; }
Quaternion3& Quaternion3::Conjugated() { x = -x; y = -y; z = -z; return *this; }

Quaternion3 Quaternion3::Inverse() const
{
	F32 n = 1.0f / Math::Sqrt(x * x + y * y + z * z + w * w);
	return { -x * n, -y * n, -z * n, w * n };
}

Quaternion3& Quaternion3::Inversed() { return Conjugated().Normalized(); }

F32& Quaternion3::operator[] (U8 i) { return (&x)[i]; }
const F32& Quaternion3::operator[] (U8 i) const { return (&x)[i]; }

F32* Quaternion3::Data() { return &x; }
const F32* Quaternion3::Data() const { return &x; }