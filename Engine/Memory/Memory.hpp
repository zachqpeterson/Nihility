#pragma once

#include "Defines.hpp"

/// <summary>
/// This is a general purpose memory allocator
/// </summary>
class NH_API Memory
{
public:
	static void Allocate();
	static void Free();
	static void Reallocate();

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	STATIC_CLASS(Memory);
	friend class Engine;
};