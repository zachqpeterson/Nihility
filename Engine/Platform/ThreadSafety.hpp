#pragma once

#include "Defines.hpp"

I64 SafeIncrement(I64 volatile* i);
U64 SafeIncrement(U64 volatile* i);
I64 SafeDecrement(I64 volatile* i);
U64 SafeDecrement(U64 volatile* i);
bool SafeCheckAndSet(bool volatile* b);
bool SafeCheckAndReset(bool volatile* b);