#include "ThreadSafety.hpp"

#if defined(NH_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

#if defined(NH_PLATFORM_WINDOWS)

I64 ThreadSafety::SafeIncrement64(volatile I64* t) { return InterlockedIncrement64(t); }
L32 ThreadSafety::SafeIncrement32(volatile L32* t) { return InterlockedIncrement(t); }

I64 ThreadSafety::SafeAdd64(volatile I64* t, I64 value) { return InterlockedAdd64(t, value); }
L32 ThreadSafety::SafeAdd32(volatile L32* t, L32 value) { return InterlockedAdd(t, value); }

I64 ThreadSafety::SafeDecrement64(volatile I64* t) { return InterlockedDecrement64(t); }
L32 ThreadSafety::SafeDecrement32(volatile L32* t) { return InterlockedDecrement(t); }

I64 ThreadSafety::SafeSubtract64(volatile I64* t, I64 value) { return InterlockedAdd64(t, -value); }
L32 ThreadSafety::SafeSubtract32(volatile L32* t, L32 value) { return InterlockedAdd(t, -value); }

I64 ThreadSafety::SafeCheckAndSet64(volatile I64* t, I64 pos) { return InterlockedBitTestAndSet64(t, pos); }
L32 ThreadSafety::SafeCheckAndSet32(volatile L32* t, L32 pos) { return InterlockedBitTestAndSet(t, pos); }

I64 ThreadSafety::SafeCheckAndReset64(volatile I64* t, I64 pos) { return InterlockedBitTestAndReset64(t, pos); }
L32 ThreadSafety::SafeCheckAndReset32(volatile L32* t, L32 pos) { return InterlockedBitTestAndReset(t, pos); }

I64 ThreadSafety::SafeCompareAndExchange64(volatile I64* t, I64 exchange, I64 comperand) { return InterlockedCompareExchange64(t, exchange, comperand); }
L32 ThreadSafety::SafeCompareAndExchange32(volatile L32* t, L32 exchange, L32 comperand) { return InterlockedCompareExchange(t, exchange, comperand); }

#endif