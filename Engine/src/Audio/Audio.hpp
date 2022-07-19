#pragma once

#include "Defines.hpp"

class NH_API Audio
{
public:


private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	friend class Engine;
};