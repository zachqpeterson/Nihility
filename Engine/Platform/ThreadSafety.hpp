#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include <xthreads.h>
#include <atomic>

inline void YieldThread() noexcept { _Thrd_yield(); }

struct NH_API SpinLock
{
	std::atomic<bool> lockFlag{ false };

public:
	void Lock()
	{
		while (true)
		{
			if (!lockFlag.exchange(true, std::memory_order_acquire)) { break; }
			while (lockFlag.load(std::memory_order_relaxed)) { YieldThread(); }
		}
	}

	void Unlock()
	{
		lockFlag.store(false, std::memory_order_release);
	}
};

template <class Mutex> 
struct NH_API NH_NODISCARD LockGuard
{
public:
	explicit LockGuard(Mutex& mutex) : mutex(mutex) { mutex.Lock(); }

	~LockGuard() noexcept { mutex.Unlock(); }

private:
	Mutex& mutex;

	LockGuard(const LockGuard&) = delete;
	LockGuard& operator=(const LockGuard&) = delete;
};

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

template<Integer Int>
NH_API Int SafeIncrement(volatile Int* t)
{
	if constexpr (sizeof(Int) == 8) { return (Int)ThreadSafety::SafeIncrement64((volatile I64*)t); }
	else { return (Int)ThreadSafety::SafeIncrement32((volatile L32*)t); }
}

template<Integer Int0, Integer Int1>
NH_API Int0 SafeAdd(volatile Int0* t, Int1 value)
{
	if constexpr (sizeof(Int0) == 8) { return (Int0)ThreadSafety::SafeAdd64((volatile I64*)t, (I64)value); }
	else { return (Int0)ThreadSafety::SafeAdd32((volatile L32*)t, (L32)value); }
}

template<Integer Int>
NH_API Int SafeDecrement(volatile Int* t)
{
	if constexpr (sizeof(Int) == 8) { return (Int)ThreadSafety::SafeDecrement64((volatile I64*)t); }
	else { return (Int)ThreadSafety::SafeDecrement32((volatile L32*)t); }
}

template<Integer Int0, Integer Int1>
NH_API Int0 SafeSubtract(volatile Int0* t, Int1 value)
{
	if constexpr (sizeof(Int0) == 8) { return (Int0)ThreadSafety::SafeSubtract64((volatile I64*)t, (I64)value); }
	else { return (Int0)ThreadSafety::SafeSubtract32((volatile L32*)t, (L32)value); }
}

template<Integer Int0, Integer Int1>
NH_API Int0 SafeCheckAndSet(volatile Int0* t, Int1 pos)
{
	if constexpr (sizeof(Int0) == 8) { return (Int0)ThreadSafety::SafeCheckAndSet64((volatile I64*)t, (I64)pos); }
	else { return (Int0)ThreadSafety::SafeCheckAndSet32((volatile L32*)t, (L32)pos); }
}

template<Integer Int0, Integer Int1>
NH_API Int0 SafeCheckAndReset(volatile Int0* t, Int1 pos)
{
	if constexpr (sizeof(Int0) == 8) { return (Int0)ThreadSafety::SafeCheckAndReset64((volatile I64*)t, (I64)pos); }
	else { return (Int0)ThreadSafety::SafeCheckAndReset32((volatile L32*)t, (L32)pos); }
}

template<Integer Int0, Integer Int1, Integer Int2>
NH_API Int0 SafeCompareAndExchange(volatile Int0* t, Int1 exchange, Int2 comperand)
{
	if constexpr (sizeof(Int0) == 8) { return (Int0)ThreadSafety::SafeCompareAndExchange64((volatile I64*)t, (I64)exchange, (I64)comperand); }
	else { return (Int0)ThreadSafety::SafeCompareAndExchange32((volatile L32*)t, (L32)exchange, (L32)comperand); }
}

//TODO: Atomic Types

typedef std::atomic<I32> I32_Atomic;
typedef std::atomic<U32> U32_Atomic;