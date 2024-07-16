#pragma once

#include "Defines.hpp"
#include "Introspection.hpp"

import Containers;

struct Scene;

struct NH_API Component
{
protected:
	Component() {}
	Component(Component&& other) noexcept : entityID(other.entityID) {}
	Component& operator=(Component&& other) noexcept { entityID = other.entityID; return *this; }

	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;
	virtual void Cleanup(Scene* scene) = 0;

	U32 entityID = U32_MAX;

	template<typename>
	friend struct ComponentPoolInternal;
	friend struct Scene;
	friend struct Entity;
};

template <class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component>;
template <class Type> concept ComponentType = IsComponent<Type>;