#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Freelist.hpp"
#include "Core/Logger.hpp"

template <class Type>
struct ComponentRef
{
	ComponentRef();
	ComponentRef(NullPointer);
	ComponentRef(U32 entityId, U32 typeId);
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
	bool operator!() const;

private:
	U32 entityId = U32_MAX;
	U32 typeId = U32_MAX;
};

template <class Type>
inline ComponentRef<Type>::ComponentRef() {}

template <class Type>
inline ComponentRef<Type>::ComponentRef(NullPointer) {}

template <class Type>
inline ComponentRef<Type>::ComponentRef(U32 entityId, U32 typeId) : entityId(entityId), typeId(typeId) {}

template <class Type>
inline void ComponentRef<Type>::Destroy()
{
	entityId = U32_MAX;
	typeId = U32_MAX;
}

template <class Type>
inline ComponentRef<Type>::ComponentRef(const ComponentRef& other) : entityId(other.entityId), typeId(other.typeId) {}

template <class Type>
inline ComponentRef<Type>::ComponentRef(ComponentRef&& other) noexcept : entityId(other.entityId), typeId(other.typeId)
{
	other.entityId = U32_MAX;
	other.typeId = U32_MAX;
}

template <class Type>
inline ComponentRef<Type>& ComponentRef<Type>::operator=(NullPointer)
{
	entityId = U32_MAX;
	typeId = U32_MAX;

	return *this;
}

template <class Type>
inline ComponentRef<Type>& ComponentRef<Type>::operator=(const ComponentRef<Type>& other)
{
	entityId = other.entityId;
	typeId = other.typeId;

	return *this;
}

template <class Type>
inline ComponentRef<Type>& ComponentRef<Type>::operator=(ComponentRef<Type>&& other) noexcept
{
	entityId = other.entityId;
	typeId = other.typeId;

	other.entityId = U32_MAX;
	other.typeId = U32_MAX;

	return *this;
}

template <class Type>
inline ComponentRef<Type>::~ComponentRef()
{
	entityId = U32_MAX;
	typeId = U32_MAX;
}

template <class Type>
inline Type* ComponentRef<Type>::Get()
{
	return Type::Get(typeId);
}

template <class Type>
inline const Type* ComponentRef<Type>::Get() const
{
	return Type::Get(typeId);
}

template <class Type>
inline Type* ComponentRef<Type>::operator->()
{
	return Type::Get(typeId);
}

template <class Type>
inline const Type* ComponentRef<Type>::operator->() const
{
	return Type::Get(typeId);
}

template <class Type>
inline Type& ComponentRef<Type>::operator*()
{
	return *Type::Get(typeId);
}

template <class Type>
inline const Type& ComponentRef<Type>::operator*() const
{
	return *Type::Get(typeId);
}

template <class Type>
inline ComponentRef<Type>::operator Type* ()
{
	return Type::Get(typeId);
}

template <class Type>
inline ComponentRef<Type>::operator const Type* () const
{
	return Type::Get(typeId);
}

template <class Type>
inline bool ComponentRef<Type>::operator==(const ComponentRef<Type>& other) const
{
	return typeId == other.typeId;
}

template <class Type>
inline bool ComponentRef<Type>::Valid() const
{
	return typeId != U32_MAX;
}

template <class Type>
inline ComponentRef<Type>::operator bool() const
{
	return typeId != U32_MAX;
}

template <class Type>
inline bool ComponentRef<Type>::operator!() const
{
	return typeId == U32_MAX;
}

#define COMPONENT(Type)																\
private:																			\
	static Vector<Type> components;													\
	static Freelist freeComponents;													\
																					\
	static Type& Create(U32& index)													\
	{																				\
		index = freeComponents.GetFree();											\
		return components[index];													\
	}																				\
																					\
	static void Destroy(Type& component)  											\
	{																				\
		component.entityIndex = U32_MAX;											\
		freeComponents.Release((U32)(&component - components.Data()));				\
	}																				\
																					\
	static void Clear()																\
	{																				\
		freeComponents.Reset();														\
		components.Clear();															\
	}																				\
																					\
	U32 entityIndex = U32_MAX;														\
																					\
public:																				\
	static Type* Get(U32 id) { return &components[id]; }							\
																					\
	static ComponentRef<Type> Get(const EntityRef& entity)							\
	{																				\
		U32 entityId = entity.EntityId();											\
																					\
		U32 index = 0;																\
		for (const Type& component : components)									\
		{																			\
			if (component.entityIndex == entityId) { return { entityId, index }; }	\
			++index;																\
		}																			\
																					\
		return nullptr;																\
	}