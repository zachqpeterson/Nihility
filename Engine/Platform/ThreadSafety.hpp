#pragma once

#include "Defines.hpp"

I64 NH_API SafeIncrement(I64 volatile* i);
U64 NH_API SafeIncrement(U64 volatile* i);
I64 NH_API SafeDecrement(I64 volatile* i);
U64 NH_API SafeDecrement(U64 volatile* i);
bool NH_API SafeCheckAndSet(bool volatile* b);
bool NH_API SafeCheckAndReset(bool volatile* b);