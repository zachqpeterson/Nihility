module;

#include "Defines.hpp"

export module Containers:Pool;

export template<class Type, U64 Capacity>
struct Pool
{
	Pool() {}

	Type* Request();
	Type* Request(U64& handle);
	Type* Obtain(U64 handle);

	void Release(U64 handle);
	void Reset();

	bool Available() const;

private:
	static constexpr inline U64 capacity = Capacity;

	U64 lastFree = 0;
	U64 freeCount = 0;
	U64 freeHandles[capacity];
	Type buffer[capacity];
};

template<class Type, U64 Capacity>
inline Type* Pool<Type, Capacity>::Request()
{
	if (!Available()) { return nullptr; }

	if (freeCount < capacity) { return buffer + freeHandles[freeCount--]; }

	return buffer + lastFree++;
}

template<class Type, U64 Capacity>
inline Type* Pool<Type, Capacity>::Request(U64& handle)
{
	if (!Available()) { return nullptr; }

	if (freeCount < capacity) { return buffer + (handle = freeHandles[freeCount--]); }

	return buffer + (handle = lastFree++);
}

template<class Type, U64 Capacity>
inline Type* Pool<Type, Capacity>::Obtain(U64 handle)
{
	if (handle == U64_MAX) { return nullptr; }

	return buffer + handle;
}

template<class Type, U64 Capacity>
inline void Pool<Type, Capacity>::Release(U64 handle)
{
	freeHandles[freeCount++] = handle;
}

template<class Type, U64 Capacity>
inline void Pool<Type, Capacity>::Reset()
{
	freeCount = 0;
	lastFree = 0;
	Zero(freeHandles, sizeof(U32) * capacity);
}

template<class Type, U64 Capacity>
inline bool Pool<Type, Capacity>::Available() const
{
	return freeCount > 0 || lastFree < capacity;
}