#pragma once

#include "Defines.hpp"
#include "Introspection.hpp"
#include "Containers\Hashmap.hpp"
#include "Containers\String.hpp"

struct Scene;

struct NH_API Component
{
protected:
	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;
	virtual void Cleanup(Scene* scene) = 0;

	U32 entityID;

	template<typename>
	friend struct ComponentPoolInternal;
	friend struct Scene;
	friend struct Entity;
};

template <class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component>;
template <class Type> concept ComponentType = IsComponent<Type>;