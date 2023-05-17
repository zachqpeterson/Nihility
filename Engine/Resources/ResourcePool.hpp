#include "ResourceDefines.hpp"

#include "Memory\Memory.hpp"

template<class Type, U64 Count>
struct ResourcePool
{
	static constexpr U64 ResourceSize = sizeof(Type);
	static constexpr U64 ResourceCount = Count;
	static constexpr U64 MemorySize = ResourceCount * (ResourceSize + sizeof(U32));

	void Create();
	void Destroy();

	U64 ObtainResource();
	void ReleaseResource(U64 handle);
	void FreeResources();

	Type* GetResource(U64 handle);
	const Type* GetResource(U64 handle) const;

	Type* memory{ nullptr };
	U64* freeHandles{ nullptr };
	U64 lastFree{ 0 };
};

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::Create()
{
	Memory::AllocateSize(&memory, MemorySize);
	freeHandles = (U64*)(memory + ResourceCount * ResourceSize);

	for (U64 i = 0; i < ResourceCount; ++i) { freeHandles[i] = i; }
}

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::Destroy()
{
	Memory::FreeSize(&memory);
}

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::FreeResources()
{
	lastFree = 0;

	for (U64 i = 0; i < ResourceCount; ++i) { freeHandles[i] = i; }
}

template<class Type, U64 Count>
inline U64 ResourcePool<Type, Count>::ObtainResource()
{
	// TODO: add bits for checking if resource is alive and use bitmasks.
	if (lastFree < ResourceCount) { return freeHandles[lastFree++]; }

	Logger::Error("No Free Resources Left!");
	return U64_MAX;
}

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::ReleaseResource(U64 handle)
{
	// TODO: add bits for checking if resource is alive and use bitmasks.
	freeHandles[--lastFree] = handle;
}

template<class Type, U64 Count>
inline Type* ResourcePool<Type, Count>::GetResource(U64 handle)
{
	if (handle != U64_MAX) { return &memory[handle * ResourceSize]; }

	return nullptr;
}

template<class Type, U64 Count>
inline const Type* ResourcePool<Type, Count>::GetResource(U64 handle) const
{
	if (handle != U64_MAX) { return &memory[handle * ResourceSize]; }

	return nullptr;
}