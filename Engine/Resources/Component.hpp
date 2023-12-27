#pragma once

#include "Defines.hpp"

struct Scene;

struct NH_API Component
{
public:
	virtual void Update() = 0;
	virtual void Load(Scene* scene) = 0;
};

template <class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component>;
template <class Type> concept ComponentType = IsComponent<Type>;