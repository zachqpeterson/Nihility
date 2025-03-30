#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

class NH_API Math {
public:

	template <class Type> static constexpr Type Clamp(const Type& n, const Type& min, const Type& max) noexcept { return n < min ? min : n > max ? max : n; }
	template <FloatingPoint Type> static constexpr Type Round(const Type& n) noexcept { return (Type)(I64)(n + 0.5); }

private:


	STATIC_CLASS(Math);
};