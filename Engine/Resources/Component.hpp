#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"
#include "Core/Logger.hpp"

template <class Type>
struct ComponentRef
{
	ComponentRef();
	ComponentRef(NullPointer);
	ComponentRef(U32 entityId, U32 sceneId, U32 typeId);
	void Destroy();

	ComponentRef(const ComponentRef& other);
	ComponentRef(ComponentRef&& other) noexcept;
	ComponentRef& operator=(NullPointer);
	ComponentRef& operator=(const ComponentRef& other);
	ComponentRef& operator=(ComponentRef&& other) noexcept;
	~ComponentRef();

	Type* Get();
	const Type* Get() const;
	Type* operator->();
	const Type* operator->() const;
	Type& operator*();
	const Type& operator*() const;
	operator Type* ();
	operator const Type* () const;

	bool operator==(const ComponentRef<Type>& other) const;

	bool Valid() const;
	operator bool() const;

private:
	U32 entityId = U32_MAX;
	U32 sceneId = U32_MAX;
	U32 typeId = U32_MAX;
};

template <class Type>
inline ComponentRef<Type>::ComponentRef() {}

template <class Type>
inline ComponentRef<Type>::ComponentRef(NullPointer) {}

template <class Type>
inline ComponentRef<Type>::ComponentRef(U32 entityId, U32 sceneId, U32 typeId) : entityId(entityId), sceneId(sceneId), typeId(typeId) {}

template <class Type>
inline void ComponentRef<Type>::Destroy()
{
	entityId = U32_MAX;
	sceneId = U32_MAX;
	typeId = U32_MAX;
}

template <class Type>
inline ComponentRef<Type>::ComponentRef(const ComponentRef& other) : entityId(other.entityId), sceneId(other.sceneId), typeId(other.typeId) {}

template <class Type>
inline ComponentRef<Type>::ComponentRef(ComponentRef&& other) noexcept : entityId(other.entityId), sceneId(other.sceneId), typeId(other.typeId)
{
	other.entityId = U32_MAX;
	other.sceneId = U32_MAX;
	other.typeId = U32_MAX;
}

template <class Type>
inline ComponentRef<Type>& ComponentRef<Type>::operator=(NullPointer)
{
	entityId = U32_MAX;
	sceneId = U32_MAX;
	typeId = U32_MAX;

	return *this;
}

template <class Type>
inline ComponentRef<Type>& ComponentRef<Type>::operator=(const ComponentRef<Type>& other)
{
	entityId = other.entityId;
	sceneId = other.sceneId;
	typeId = other.typeId;

	return *this;
}

template <class Type>
inline ComponentRef<Type>& ComponentRef<Type>::operator=(ComponentRef<Type>&& other) noexcept
{
	entityId = other.entityId;
	sceneId = other.sceneId;
	typeId = other.typeId;

	other.entityId = U32_MAX;
	other.sceneId = U32_MAX;
	other.typeId = U32_MAX;

	return *this;
}

template <class Type>
inline ComponentRef<Type>::~ComponentRef()
{
	entityId = U32_MAX;
	sceneId = U32_MAX;
	typeId = U32_MAX;
}

template <class Type>
inline Type* ComponentRef<Type>::Get()
{
	return Type::Get(sceneId, typeId);
}

template <class Type>
inline const Type* ComponentRef<Type>::Get() const
{
	return Type::Get(sceneId, typeId);
}

template <class Type>
inline Type* ComponentRef<Type>::operator->()
{
	return Type::Get(sceneId, typeId);
}

template <class Type>
inline const Type* ComponentRef<Type>::operator->() const
{
	return Type::Get(sceneId, typeId);
}

template <class Type>
inline Type& ComponentRef<Type>::operator*()
{
	return *Type::Get(sceneId, typeId);
}

template <class Type>
inline const Type& ComponentRef<Type>::operator*() const
{
	return *Type::Get(sceneId, typeId);
}

template <class Type>
inline ComponentRef<Type>::operator Type* ()
{
	return Type::Get(sceneId, typeId);
}

template <class Type>
inline ComponentRef<Type>::operator const Type* () const
{
	return Type::Get(sceneId, typeId);
}

template <class Type>
inline bool ComponentRef<Type>::operator==(const ComponentRef<Type>& other) const
{
	return sceneId == other.sceneId && typeId == other.typeId;
}

template <class Type>
inline bool ComponentRef<Type>::Valid() const
{
	return sceneId != U32_MAX && typeId != U32_MAX;
}

template <class Type>
inline ComponentRef<Type>::operator bool() const
{
	return sceneId != U32_MAX && typeId != U32_MAX;
}

#define COMPONENT(Type, instanceCount)																				\
private: 																											\
	static Vector<Vector<Type>> components;																			\
public: 																											\
	static Type* Get(U32 sceneId, U32 typeId) { return &components[sceneId][typeId]; }								\
																													\
	static ComponentRef<Type> Get(const EntityRef& entity)															\
	{																												\
		U32 sceneId = entity.SceneId();																				\
		U32 entityId = entity.EntityId();																			\
																													\
		const Vector<Type>& types = components[sceneId];															\
																													\
		U32 index = 0;																								\
		for (const Type& type : types)																				\
		{																											\
			if (type.entityIndex == entityId) { return { entityId, sceneId, index }; }								\
			++index;																								\
		}																											\
																													\
		return nullptr;																								\
	}																												\
																													\
	U32 entityIndex = 0;																							\
																													\
private:																											\
	static void AddScene(U32 sceneId)																				\
	{																												\
		if (sceneId < components.Size())																			\
		{																											\
			if (components[sceneId].Size())																			\
			{																										\
				Logger::Error("Scene Already Added!");																\
				return;																								\
			}																										\
																													\
			components[sceneId].Reserve(instanceCount);																\
		}																											\
		else if (sceneId == components.Size())																		\
		{																											\
			components.Push({ instanceCount });																		\
		}																											\
		else																										\
		{																											\
			Logger::Error("Invalid Scene!");																		\
		}																											\
	}																												\
																													\
	static void RemoveScene(U32 sceneId)																			\
	{																												\
		if (sceneId >= components.Size())																			\
		{																											\
			Logger::Error("Invalid Scene!");																		\
			return;																									\
		}																											\
																													\
		components[sceneId].Clear();																				\
	}