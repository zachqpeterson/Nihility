#pragma once

template<typename T, T... Args>
struct Initializer
{
	constexpr Initializer() {}

	const U64 size = sizeof...(Args);
	const T list[sizeof...(Args)] = { Args... };
};