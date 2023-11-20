#pragma once

#include "Defines.hpp"

namespace Details
{
	I64 Increment64(volatile I64* i);
	L32 Increment(volatile L32* i);
	I64 Add64(volatile I64* i, I64 value);
	L32 Add(volatile L32* i, L32 value);
	I64 Decrement64(volatile I64* i);
	L32 Decrement(volatile L32* i);
	I64 CheckAndSet64(volatile I64* i, I64 pos);
	L32 CheckAndSet(volatile L32* i, L32 pos);
	I64 CheckAndReset64(volatile I64* i, I64 pos);
	L32 CheckAndReset(volatile L32* i, L32 pos);
	I64 CompareAndExchange64(volatile I64* i, I64 exchange, I64 comperand);
	L32 CompareAndExchange(volatile L32* i, L32 exchange, L32 comperand);
}

template<Integer Type>
inline Type SafeIncrement(volatile Type* t)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::Increment64((volatile I64*)t);
	}
	else
	{
		return Details::Increment((volatile L32*)t);
	}
}

template<Integer Type>
inline Type SafeAdd(volatile Type* t, Type value)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::Add64((volatile I64*)t, (I64)value);
	}
	else
	{
		return Details::Add((volatile L32*)t, (L32)value);
	}
}

template<Integer Type>
inline Type SafeDecrement(volatile Type* t)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::Decrement64((volatile I64*)t);
	}
	else
	{
		return Details::Decrement((volatile L32*)t);
	}
}

template<Integer Type>
inline Type SafeSubtract(volatile Type* t, Type value)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::Add64((volatile I64*)t, -(I64)value);
	}
	else
	{
		return Details::Add((volatile L32*)t, -(L32)value);
	}
}

template<class Type, Integer Pos>
inline Type SafeCheckAndSet(volatile Type* t, Pos pos)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::CheckAndSet64((volatile I64*)t, (I64)pos);
	}
	else
	{
		return Details::CheckAndSet((volatile L32*)t, (L32)pos);
	}
}

template<class Type, Integer Pos>
inline Type SafeCheckAndReset(volatile Type* t, Pos pos)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::CheckAndReset64((volatile I64*)t, (I64)pos);
	}
	else
	{
		return Details::CheckAndReset((volatile L32*)t, (L32)pos);
	}
}

template<class Type>
inline Type SafeCompareAndExchange(volatile Type* t, Type exchange, Type comperand)
{
	if constexpr (sizeof(Type) == 8)
	{
		return Details::CompareAndExchange64((volatile I64*)t, (I64)exchange, (I64)comperand);
	}
	else
	{
		return Details::CompareAndExchange((volatile L32*)t, (L32)exchange, (L32)comperand);
	}
}