#include "ThreadSafety.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>

I64 SafeIncrement(I64 volatile* i)
{
	return InterlockedIncrement64(i);
}

I64 SafeDecrement(I64 volatile* i)
{
	return InterlockedDecrement64(i);
}

bool SafeCheckAndSet(bool volatile* b)
{
	return InterlockedBitTestAndSet64((I64 volatile*)b, 0);
}

bool SafeCheckAndReset(bool volatile* b)
{
	return InterlockedBitTestAndSet64((I64 volatile*)b, 0);
}

#endif