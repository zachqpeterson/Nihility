#pragma once

#include "Defines.hpp"

template<typename>
struct ComponentPoolInternal;
template<typename>
struct Vector;

#define COMPONENT(class) \
friend struct Scene; \
friend struct ComponentPoolInternal<class>; \
friend struct Vector<class>; \
private:

struct Scene;

struct NH_API Component
{
protected:
	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;

	U32 entityID;

	friend struct Scene;
	friend struct Entity;
};

template <class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component>;
template <class Type> concept ComponentType = IsComponent<Type>;