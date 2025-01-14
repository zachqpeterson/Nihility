#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

struct Scene;

struct Component
{
	virtual void Update(Scene* scene, U32 entityID) = 0;
	virtual void Load(Scene* scene, U32 entityID) = 0;
	virtual void Cleanup(Scene* scene, U32 entityID) = 0;
};

template<class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component>;
template<class Type> concept ComponentType = IsComponent<Type>;