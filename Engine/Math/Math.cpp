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
Vector2& Vector2::Tangent() { F32 t = x; x = -y; y = t; return *this; }
Vector2& Vector2::TangentPerp() { F32 t = x; x = y; y = -t; return *this; }
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

Vector2& Vector2::Clamp(const Vector2& xBound, const Vector2& yBound)
{
	x = Math::Clamp(x, xBound.x, xBound.y);
	y = Math::Clamp(y, yBound.x, yBound.y);
	return *this;
}

Vector2 Vector2::Clamped(const Vector2& xBound, const Vector2& yBound) const
{
	return {
		Math::Clamp(x, xBound.x, xBound.y),
		Math::Clamp(y, yBound.x, yBound.y)
	};
}

Vector2& Vector2::SetClosest(const Vector2& xBound, const Vector2& yBound)
{
	x = Math::Closest(x, xBound.x, xBound.y);
	y = Math::Closest(y, yBound.x, yBound.y);
	return *this;
}

Vector2 Vector2::Closest(const Vector2& xBound, const Vector2& yBound) const
{
	return {
		Math::Closest(x, xBound.x, xBound.y),
		Math::Closest(y, yBound.x, yBound.y)
	};
}

F32& Vector2::operator[] (U64 i) { return (&x)[i]; }
const F32& Vector2::operator[] (U64 i) const { return (&x)[i]; }
F32* Vector2::Data() { return &x; }
const F32* Vector2::Data() const { return &x; }

Vector2::operator String() const { return String(x, ", ", y); }
Vector2::operator String16() const { return String16(x, u", ", y); }
Vector2::operator String32() const { return String32(x, U", ", y); }