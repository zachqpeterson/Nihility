module;

#include "Defines.hpp"

module Math:Functions;

import :Types;
import Containers;
import Core;

U64 Random::TrueRandomInt()
{
	I64 time = Time::CoreCounter();
	time = Mix(time ^ secret0, seed ^ secret1);
	seed = Mix(time ^ secret0, secret2);
	return Mix(seed, seed ^ secret3);
}
