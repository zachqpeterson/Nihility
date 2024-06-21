module;

#include "Defines.hpp"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

export module ThreadSafety;

#if defined(PLATFORM_WINDOWS)
export template<Integer Type>
inline Type SafeIncrement(volatile Type* t)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedIncrement64((volatile I64*)t);
	}
	else
	{
		return InterlockedIncrement((volatile L32*)t);
	}
}

export template<Integer Type>
inline Type SafeAdd(volatile Type* t, Type value)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedAdd64((volatile I64*)t, (I64)value);
	}
	else
	{
		return InterlockedAdd((volatile L32*)t, (L32)value);
	}
}

export template<Integer Type>
inline Type SafeDecrement(volatile Type* t)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedDecrement64((volatile I64*)t);
	}
	else
	{
		return InterlockedDecrement((volatile L32*)t);
	}
}

export template<Integer Type>
inline Type SafeSubtract(volatile Type* t, Type value)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedAdd64((volatile I64*)t, -(I64)value);
	}
	else
	{
		return InterlockedAdd((volatile L32*)t, -(L32)value);
	}
}

export template<class Type, Integer Pos>
inline Type SafeCheckAndSet(volatile Type* t, Pos pos)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedBitTestAndSet64((volatile I64*)t, (I64)pos);
	}
	else
	{
		return InterlockedBitTestAndSet((volatile L32*)t, (L32)pos);
	}
}

export template<class Type, Integer Pos>
inline Type SafeCheckAndReset(volatile Type* t, Pos pos)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedBitTestAndReset64((volatile I64*)t, (I64)pos);
	}
	else
	{
		return InterlockedBitTestAndReset((volatile L32*)t, (L32)pos);
	}
}

export template<class Type>
inline Type SafeCompareAndExchange(volatile Type* t, Type exchange, Type comperand)
{
	if constexpr (sizeof(Type) == 8)
	{
		return InterlockedCompareExchange64((volatile I64*)t, (I64)exchange, (I64)comperand);
	}
	else
	{
		return InterlockedCompareExchange((volatile L32*)t, (L32)exchange, (L32)comperand);
	}
}

#endif