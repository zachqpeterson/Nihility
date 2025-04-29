#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Hashmap.hpp"

enum class TextureType : I32
{
    None = 0,
    Diffuse = 1,
    Specular = 2,
    Ambient = 3,
    Emissive = 4,
    Height = 5,
    Normals = 6,
    Shininess = 7,
    Opacity = 8,
	Displacement = 9,
    Lightmap = 10,
    Reflection = 11,
    BaseColor = 12,
    NormalCamera = 13,
    EmissionColor = 14,
    Metalness = 15,
    DiffuseRoughness = 16,
    AmbientOcclusion = 17,
    Unknown = 18,
    Sheen = 19,
    Clearcoat = 20,
    Transmission = 21,
    MayaBase = 22,
    MayaSpecular = 23,
    MayaSpecularColor = 24,
    MayaSpecularRoughness = 25,
    Anisotropy = 26,
    GltfMetallicRoughness = 27
};

enum class AnimationBehaviour : I32
{
	Default = 0,
	Constant = 1,
	Linear = 2,
	Repeat = 3
};

template<class Type>
struct NH_API Resource
{
public:
	Type* operator->();

	Type type = {};
	U32 refCount = 0;

	friend class Resources;
};

template<class Type>
struct NH_API ResourceRef
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

	bool operator==(const ResourceRef<Type>& other) const;

	bool Valid() const;
	operator bool() const;

private:
	Type* type = nullptr;
	U32* refCount = nullptr;
	U64 hash = U64_MAX;

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
	this->hash = hash;
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
	hash = U64_MAX;
}

template<class Type>
inline ResourceRef<Type>::ResourceRef(const ResourceRef& other) : refCount(other.refCount), type(other.type), hash(other.hash) {}

template<class Type>
inline ResourceRef<Type>::ResourceRef(ResourceRef&& other) noexcept : refCount(other.refCount), type(other.type), hash(other.hash)
{
	other.refCount = nullptr;
	other.type = nullptr;
	other.hash = U64_MAX;
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
	hash = U64_MAX;

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
inline bool ResourceRef<Type>::operator==(const ResourceRef<Type>& other) const { return type == other.type; }

template<class Type>
inline bool ResourceRef<Type>::Valid() const { return type; }

template<class Type>
inline ResourceRef<Type>::operator bool() const { return type; }

struct VmaAllocation_T;
struct VkBuffer_T;

struct NH_API MatrixData
{
	Matrix4 viewMatrix{};
	Matrix4 projectionMatrix{};
};

struct NH_API VertexData
{
	Vector3 position = Vector3::Zero;
	Vector4 color = Vector4::One;
	Vector3 normal = Vector3::Zero;
	Vector2 uv = Vector2::Zero;
	Vector4Int boneNumber = Vector4Int::Zero;
	Vector4 boneWeight = Vector4::Zero;
};

struct NH_API MeshData
{
	Vector<VertexData> vertices{};
	Vector<U32> indices{};
	Hashmap<TextureType, String> textures{};
	bool usesPBRColors = false;
};