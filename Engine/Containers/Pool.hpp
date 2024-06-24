#pragma once

import Core;
import Memory;

template<class Type, U64 Count>
struct Pool
{
	static constexpr U64 ResourceSize = sizeof(Type);
	static constexpr U64 ResourceCount = Count;
	static constexpr U64 MemorySize = ResourceCount * (ResourceSize + sizeof(U32));

	void Create();
	void Destroy();

	Type* Request();
	Type* Request(U64& handle);
	Type* Obtain(U64 handle);

	void Release(U64 handle);
	void ReleaseAll();

	Type* memory{ nullptr };
	U64* freeHandles{ nullptr };
	U64 lastFree{ 0 };
};

template<class Type, U64 Count>
inline void Pool<Type, Count>::Create()
{
	Memory::AllocateSize(&memory, MemorySize);
	freeHandles = (U64*)(memory + ResourceCount * ResourceSize);

	for (U64 i = 0; i < ResourceCount; ++i) { freeHandles[i] = i; }
}

template<class Type, U64 Count>
inline void Pool<Type, Count>::Destroy()
{
	Memory::Free(&memory);
	lastFree = 0;
}

template<class Type, U64 Count>
inline Type* Pool<Type, Count>::Request()
{
	if (lastFree < ResourceCount) { return &memory[freeHandles[lastFree++]]; }

	Logger::Error("No Free Resources Left!");
	return nullptr;
}

template<class Type, U64 Count>
inline Type* Pool<Type, Count>::Request(U64& handle)
{
	if (lastFree < ResourceCount) { handle = freeHandles[lastFree++]; return &memory[handle]; }

	Logger::Error("No Free Resources Left!");
	return nullptr;
}

template<class Type, U64 Count>
inline Type* Pool<Type, Count>::Obtain(U64 handle)
{
	if (handle != U64_MAX) { return &memory[handle]; }

	return nullptr;
}

template<class Type, U64 Count>
inline void Pool<Type, Count>::Release(U64 handle)
{
	freeHandles[--lastFree] = handle;
}

template<class Type, U64 Count>
inline void Pool<Type, Count>::ReleaseAll()
{
	lastFree = 0;

	for (U64 i = 0; i < ResourceCount; ++i) { freeHandles[i] = i; }
}