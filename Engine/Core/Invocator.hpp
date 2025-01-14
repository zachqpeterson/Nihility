#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Function.hpp"

#include "Containers\Vector.hpp"

#include <initializer_list>

struct NH_API Invocator
{
public:
	Invocator() : functions() {}
	Invocator(std::initializer_list<Function<void()>> list) : functions(list) {}
	Invocator(const Invocator& other) : functions(other.functions) {}
	Invocator(Invocator&& other) noexcept : functions(Move(other.functions)) {}

	Invocator& operator+=(const Function<void()>& function) { functions.Push(function); return *this; }
	Invocator& operator+=(Function<void()>&& function) { functions.Push(Move(function)); return *this; }

	void operator()() const { for (const Function<void()>& function : functions) { function(); } }
	void Invoke() const { for (const Function<void()>& function : functions) { function(); } }

private:
	Vector<Function<void()>> functions;
};