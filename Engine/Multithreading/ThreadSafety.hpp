#pragma once

#include "Defines.hpp"

class NH_API ThreadSafety
{
public:
	static I64 SafeIncrement64(volatile I64* t);
	static L32 SafeIncrement32(volatile L32* t);

	static I64 SafeAdd64(volatile I64* t, I64 value);
	static L32 SafeAdd32(volatile L32* t, L32 value);

	static I64 SafeDecrement64(volatile I64* t);
	static L32 SafeDecrement32(volatile L32* t);

	static I64 SafeSubtract64(volatile I64* t, I64 value);
	static L32 SafeSubtract32(volatile L32* t, L32 value);

	static I64 SafeCheckAndSet64(volatile I64* t, I64 pos);
	static L32 SafeCheckAndSet32(volatile L32* t, L32 pos);

	static I64 SafeCheckAndReset64(volatile I64* t, I64 pos);
	static L32 SafeCheckAndReset32(volatile L32* t, L32 pos);

	static I64 SafeCompareAndExchange64(volatile I64* t, I64 exchange, I64 comperand);
	static L32 SafeCompareAndExchange32(volatile L32* t, L32 exchange, L32 comperand);
};