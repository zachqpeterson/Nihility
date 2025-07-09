#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"

template<class Type>
struct Resource
{
public:
	Type* operator->();

	Type type = {};
	U32 refCount = 0;

	friend class Resources;
};

template<class Type>
struct ResourceRef
{
public:
	ResourceRef();
	ResourceRef(NullPointer);
	ResourceRef(Resource<Type>& resource, U64 hash);
	void Destroy();

	ResourceRef(const ResourceRef& other);
	ResourceRef(ResourceRef&& other) noexcept;
	ResourceRef& operator=(const ResourceRef& other);
	ResourceRef& operator=(ResourceRef&& other) noexcept;
	~ResourceRef();

	Type* Get();
	const Type* Get() const;
	Type* operator->();
	const Type* operator->() const;
	Type& operator*();
	const Type& operator*() const;
	operator Type* ();
	operator const Type* () const;
	U32 Handle() const;

	bool operator==(const ResourceRef<Type>& other) const;

	bool Valid() const;
	operator bool() const;

private:
	Type* type = nullptr;
	U32* refCount = nullptr;
	U32 hash = U16_MAX;

	friend class Resources;
};

template<class Type>
inline Type* Resource<Type>::operator->() { return &type; }

template<class Type>
inline ResourceRef<Type>::ResourceRef() {}

template<class Type>
inline ResourceRef<Type>::ResourceRef(NullPointer) {}

template<class Type>
inline ResourceRef<Type>::ResourceRef(Resource<Type>& resource, U64 hash)
{
	type = &resource.type;
	refCount = &resource.refCount;
	this->hash = (U32)hash;
	++(*refCount);
}

template<class Type>
inline void ResourceRef<Type>::Destroy()
{
	if (refCount && --(*refCount) == 0)
	{
		//TODO: Delete Resource
	}

	type = nullptr;
	refCount = nullptr;
	hash = U16_MAX;
}

template<class Type>
inline ResourceRef<Type>::ResourceRef(const ResourceRef& other) : refCount(other.refCount), type(other.type), hash(other.hash) {}

template<class Type>
inline ResourceRef<Type>::ResourceRef(ResourceRef&& other) noexcept : refCount(other.refCount), type(other.type), hash(other.hash)
{
	other.refCount = nullptr;
	other.type = nullptr;
	other.hash = U16_MAX;
}

template<class Type>
inline ResourceRef<Type>& ResourceRef<Type>::operator=(const ResourceRef<Type>& other)
{
	if (&other == this) { return *this; }

	Destroy();

	type = other.type;
	refCount = other.refCount;
	hash = other.hash;

	if (refCount) { ++*refCount; }

	return *this;
}

template<class Type>
inline ResourceRef<Type>& ResourceRef<Type>::operator=(ResourceRef<Type>&& other) noexcept
{
	if (&other == this) { return *this; }

	Destroy();

	type = other.type;
	refCount = other.refCount;
	hash = other.hash;

	other.type = nullptr;
	other.refCount = nullptr;
	other.hash = U16_MAX;

	return *this;
}

template<class Type>
inline ResourceRef<Type>::~ResourceRef() { Destroy(); }

template<class Type>
inline Type* ResourceRef<Type>::Get() { return type; }

template<class Type>
inline const Type* ResourceRef<Type>::Get() const { return type; }

template<class Type>
inline Type* ResourceRef<Type>:: operator->() { return type; }

template<class Type>
inline const Type* ResourceRef<Type>::operator->() const { return type; }

template<class Type>
inline Type& ResourceRef<Type>::operator*() { return *type; }

template<class Type>
inline const Type& ResourceRef<Type>::operator*() const { return *type; }

template<class Type>
inline ResourceRef<Type>::operator Type* () { return type; }

template<class Type>
inline ResourceRef<Type>::operator const Type* () const { return type; }

template<class Type>
inline U32 ResourceRef<Type>::Handle() const { return hash; }

template<class Type>
inline bool ResourceRef<Type>::operator==(const ResourceRef<Type>& other) const { return type == other.type; }

template<class Type>
inline bool ResourceRef<Type>::Valid() const { return type; }

template<class Type>
inline ResourceRef<Type>::operator bool() const { return type; }

struct VmaAllocation_T;
struct VkBuffer_T;

struct NH_API GlobalPushConstant
{
	Matrix4 viewProjection;
};