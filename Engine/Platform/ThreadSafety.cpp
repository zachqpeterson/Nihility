#include "ThreadSafety.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>

namespace Details
{
	I64 Increment64(volatile I64* i)
	{
		return InterlockedIncrement64(i);
	}

	I64 Decrement64(volatile I64* i)
	{
		return InterlockedDecrement64(i);
	}

	L32 Increment(volatile L32* i)
	{
		return InterlockedIncrement(i);
	}

	L32 Decrement(volatile L32* i)
	{
		return InterlockedDecrement(i);
	}

	I64 CheckAndSet64(volatile I64* i, I64 pos)
	{
		return InterlockedBitTestAndSet64(i, pos);
	}

	L32 CheckAndSet(volatile L32* i, L32 pos)
	{
		return InterlockedBitTestAndSet(i, pos);
	}

	I64 CheckAndReset64(volatile I64* i, I64 pos)
	{
		return InterlockedBitTestAndReset64(i, pos);
	}

	L32 CheckAndReset(volatile L32* i, L32 pos)
	{
		return InterlockedBitTestAndReset(i, pos);
	}

	I64 CompareAndExchange64(volatile I64* i, I64 exchange, I64 comperand)
	{
		return InterlockedCompareExchange64(i, exchange, comperand);
	}

	L32 CompareAndExchange(volatile L32* i, L32 exchange, L32 comperand)
	{
		return InterlockedCompareExchange(i, exchange, comperand);
	}
}

#endif