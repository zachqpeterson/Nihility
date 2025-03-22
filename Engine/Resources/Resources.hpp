#pragma once

#include "Defines.hpp"

class NH_API Resources
{
public:


private:
	static bool Initialize();
	static void Shutdown();

	friend class Engine;

	STATIC_CLASS(Resources);
};