#pragma once

#include "Defines.hpp"
#include "Introspection.hpp"

import Containers;
import Core;
import Memory;

struct Scene;

template<class Type>
struct NH_API Component
{
	U32 GetEntityID() const { return entityID; }

protected:
	Component() {}
	Component(const Component& other) noexcept : entityID(other.entityID) {}
	Component(Component&& other) noexcept : entityID(other.entityID) {}
	Component& operator=(Component&& other) noexcept { entityID = other.entityID; return *this; }

	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;
	virtual void Cleanup(Scene* scene) = 0;

	U32 entityID = U32_MAX;

private:

	friend struct Scene;
	friend struct Entity;

	template<class Type>
	struct ComponentFactory;
	friend struct ComponentFactory<Type>;
};

template <class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component<Type>>;
template <class Type> concept ComponentType = IsComponent<Type>;

template<ComponentType Type>
struct NH_API ComponentRef
{
public:
	ComponentRef() {}
	ComponentRef(U32 index, Vector<Type>* array) : index(index), array(array) {}
	ComponentRef(NullPointer) {}
	ComponentRef(const ComponentRef& other) : index(other.index), array(other.array) {}
	~ComponentRef() { index = U32_MAX; array = nullptr; }

	ComponentRef& operator=(NullPointer) { index = U32_MAX; array = nullptr; return *this; }
	ComponentRef& operator=(const ComponentRef& other) { index = other.index; array = other.array; return *this; }

	bool operator==(const ComponentRef<Type>& other) const { return index == other.index; }
	bool operator!=(const ComponentRef<Type>& other) const { return index != other.index; }

	Type* operator->();
	const Type* operator->() const;

	Type& operator*();
	const Type& operator*() const;

	operator bool() const { return index != U32_MAX; }

private:
	U32 index = U32_MAX;
	Vector<Type>* array = nullptr;
};

template<ComponentType Type>
struct NH_API ComponentFactory
{
protected:
	virtual ComponentRef<Type> CreateComponent(Scene* scene, U32 entityID, Type** type)
	{
		U32 index = freelist.GetFree();

		if (index == components.Size()) { components.PushEmpty(); }

		*type = &components[index];

		return ComponentRef<Type>(index, &components);
	}

	virtual void DestroyComponent(Scene* scene, U32 entityID)
	{
		for (Type* it = components.begin(); it != components.end(); ++it)
		{
			if (it->GetEntityID() == entityID)
			{
				freelist.Release((U32)(it - components.begin()));

				it->Cleanup(scene);
			}
		}
	}

	virtual ComponentRef<Type> GetComponent(U32 entityID)
	{
		for (Type* it = components.begin(); it != components.end(); ++it)
		{
			if (it->GetEntityID() == entityID)
			{
				return ComponentRef<Type>((U32)(it - components.begin()), &components);
			}
		}

		return nullptr;
	}

public:
	virtual I32 GetComponentID(Type& component)
	{
		return (I32)components.Index(&component);
	}

	Freelist freelist = 256;
	Vector<Type> components;

	friend struct Entity;
	friend struct ComponentRef<Type>;
	friend struct ComponentRegistry;
};

template<ComponentType Type>
inline Type* ComponentRef<Type>::operator->() { return &array->Get(index); }

template<ComponentType Type>
inline const Type* ComponentRef<Type>::operator->() const { return &array->Get(index); }

template<ComponentType Type>
inline Type& ComponentRef<Type>::operator*() { return array->Get(index); }

template<ComponentType Type>
inline const Type& ComponentRef<Type>::operator*() const { return array->Get(index); }

template <ComponentType Type>
using ComponentCreateFn = Function<ComponentRef<Type>(Scene*, U32, Type**)>;
template <ComponentType Type>
using ComponentDestroyFn = Function<void(Scene*, U32)>;
template <ComponentType Type>
using ComponentGetFn = Function<ComponentRef<Type>(U32)>;

struct NH_API ComponentRegistry
{
private:
	template<ComponentType Type>
	struct FactoryStorage
	{
		ComponentFactory<Type> factory;

		ComponentCreateFn<Type> CreateFunction;
		ComponentDestroyFn<Type> DestroyFunction;
		ComponentGetFn<Type> GetFunction;
	};

public:
	template<ComponentType Type>
	static void RegisterComponent(const ComponentCreateFn<Type>& createFn, const ComponentDestroyFn<Type>& destroyFn, const ComponentGetFn<Type>& getFn)
	{
		StringView name = NameOf<Type>;

		FactoryStorage<Type>* existing = *(FactoryStorage<Type>**)hashmap.Request(name);

		if (!existing)
		{
			Memory::Allocate(&existing);

			existing->CreateFunction = createFn;
			existing->DestroyFunction = destroyFn;
			existing->GetFunction = getFn;
		}
	}

	template<ComponentType Type>
	static void RegisterComponent()
	{
		StringView name = NameOf<Type>;

		FactoryStorage<Type>* existing = *(FactoryStorage<Type>**)hashmap.Request(name);

		if (!existing)
		{
			Memory::Allocate(&existing);

			existing->factory = {};
			existing->CreateFunction = existing->factory.CreateComponent;
			existing->DestroyFunction = existing->factory.DestroyComponent;
			existing->GetFunction = existing->factory.GetComponent;
		}
	}

	template<ComponentType Type>
	static ComponentCreateFn<Type>& CreateFunction()
	{
		StringView name = NameOf<Type>;

		return (*(FactoryStorage<Type>**)hashmap.Get(name))->CreateFunction;
	}

	template<ComponentType Type>
	static ComponentDestroyFn<Type>& DestroyFunction()
	{
		StringView name = NameOf<Type>;

		return (*(FactoryStorage<Type>**)hashmap.Get(name))->DestroyFunction;
	}

	template<ComponentType Type>
	static ComponentGetFn<Type>& GetFunction()
	{
		StringView name = NameOf<Type>;

		return (*(FactoryStorage<Type>**)hashmap.Get(name))->GetFunction;
	}

	static inline Hashmap<StringView, void*> hashmap = 64;
};