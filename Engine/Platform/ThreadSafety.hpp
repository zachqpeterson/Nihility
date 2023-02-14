#pragma once

#include "Defines.hpp"

I64 SafeIncrement(I64 volatile* i);
I64 SafeDecrement(I64 volatile* i);
bool SafeCheckAndSet(bool volatile* b);
bool SafeCheckAndReset(bool volatile* b);