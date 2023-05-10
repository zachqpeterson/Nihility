#include "Math.hpp"

#include "Containers\String.hpp"

//TRIGONOMETRY



//Vector2

const Vector2 Vector2::Zero{ 0.0f };
const Vector2 Vector2::One{ 1.0f };
const Vector2 Vector2::Left{ -1.0f, 0.0f };
const Vector2 Vector2::Right{ 1.0f, 0.0f };
const Vector2 Vector2::Up{ 0.0f, 1.0f };
const Vector2 Vector2::Down{ 0.0f, -1.0f };

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
	F32 cos = Math::Cos(angle);
	F32 sin = Math::Sin(angle);
	F32 temp = cos * (x - center.x) - sin * (y - center.y) + center.x;
	y = sin * (x - center.x) + cos * (y - center.y) + center.y;
	x = temp;

	return *this;
}

Vector2 Vector2::Rotated(const Vector2& center, F32 angle) const
{
	F32 cos = Math::Cos(angle);
	F32 sin = Math::Sin(angle);
	return Vector2{ cos * (x - center.x) - sin * (y - center.y) + center.x,
	sin * (x - center.x) + cos * (y - center.y) + center.y };
}

Vector2& Vector2::Rotate(const Vector2& center, const Quaternion2& quat)
{
	return *this;
}

Vector2 Vector2::Rotated(const Vector2& center, const Quaternion2& quat) const
{
	return {};
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

const Vector3 Vector3::Zero{ 0.0f };
const Vector3 Vector3::One{ 1.0f };
const Vector3 Vector3::Left{ -1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Right{ 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Up{ 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::Down{ 0.0f, -1.0f, 0.0f };
const Vector3 Vector3::Forward{ 0.0f, 0.0f, 1.0f };
const Vector3 Vector3::Back{ 0.0f, 0.0f, -1.0f };

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
Vector3 Vector3::Cross(const Vector3& v) const { return { y * v.z - z * v.y, z * v.x - v.x * z, x * v.y - y * v.x }; }

Vector3& Vector3::Rotate(const Vector3& center, const Quaternion3& quat)
{
	return *this;
}

Vector3 Vector3::Rotated(const Vector3& center, const Quaternion3& quat) const
{
	return {};
}

Vector3& Vector3::Clamp(const Vector3& min, const Vector3& max)
{
	x = Math::Clamp(x, min.x, max.x);
	y = Math::Clamp(y, min.y, max.y);
	y = Math::Clamp(z, min.z, max.z);
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