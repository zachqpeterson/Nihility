module;

#include "Defines.hpp"

module Math:Functions;

import :Types;
import Containers;
import Core;

constexpr const char ALPHANUM_LOOKUP[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789";

U64 Random::TrueRandomInt()
{
	I64 time = Time::CoreCounter();
	time = Mix(time ^ secret0, seed ^ secret1);
	seed = Mix(time ^ secret0, secret2);
	return Mix(seed, seed ^ secret3);
}

String Random::RandomString(U32 length) noexcept
{
	String str{};
	str.Resize(16);

	C8* it = str.Data();

	for (U32 i = 0; i < length; ++i)
	{
		*it++ = ALPHANUM_LOOKUP[RandomRange(0, CountOf(ALPHANUM_LOOKUP))];
	}

	return str;
}
