#include "ResourceDefines.hpp"

#include "Memory\Memory.hpp"

template<class Type, ResourceHandle Count>
struct ResourcePool
{
	static constexpr ResourceHandle ResourceSize = sizeof(Type);
	static constexpr ResourceHandle ResourceCount = Count;
	static constexpr ResourceHandle MemorySize = ResourceCount * (ResourceSize + sizeof(U32));

	void Create();
	void Destroy();

	ResourceHandle ObtainResource();
	void ReleaseResource(ResourceHandle handle);
	void FreeResources();

	Type* GetResource(ResourceHandle handle);
	const Type* GetResource(ResourceHandle handle) const;

	Type* memory{ nullptr };
	ResourceHandle* freeHandles{ nullptr };
	ResourceHandle lastFree{ 0 };
};

template<class Type, ResourceHandle Count>
inline void ResourcePool<Type, Count>::Create()
{
	Memory::AllocateSize(&memory, MemorySize);
	freeHandles = (ResourceHandle*)(memory + ResourceCount * ResourceSize);

	for (ResourceHandle i = 0; i < ResourceCount; ++i) { freeHandles[i] = i; }
}

template<class Type, ResourceHandle Count>
inline void ResourcePool<Type, Count>::Destroy()
{
	Memory::FreeSize(&memory);
}

template<class Type, ResourceHandle Count>
inline void ResourcePool<Type, Count>::FreeResources()
{
	lastFree = 0;

	for (ResourceHandle i = 0; i < ResourceCount; ++i) { freeHandles[i] = i; }
}

template<class Type, ResourceHandle Count>
inline ResourceHandle ResourcePool<Type, Count>::ObtainResource()
{
	// TODO: add bits for checking if resource is alive and use bitmasks.
	if (lastFree < ResourceCount) { return freeHandles[lastFree++]; }

	Logger::Error("No Free Resources Left!");
	return INVALID_HANDLE;
}

template<class Type, ResourceHandle Count>
inline void ResourcePool<Type, Count>::ReleaseResource(ResourceHandle handle)
{
	// TODO: add bits for checking if resource is alive and use bitmasks.
	freeHandles[--lastFree] = handle;
}

template<class Type, ResourceHandle Count>
inline Type* ResourcePool<Type, Count>::GetResource(ResourceHandle handle)
{
	if (handle != INVALID_HANDLE) { return &memory[handle * ResourceSize]; }

	return nullptr;
}

template<class Type, ResourceHandle Count>
inline const Type* ResourcePool<Type, Count>::GetResource(ResourceHandle handle) const
{
	if (handle != INVALID_HANDLE) { return &memory[handle * ResourceSize]; }

	return nullptr;
}