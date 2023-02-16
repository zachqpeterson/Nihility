#pragma once

#include "Defines.hpp"

class NH_API Resources
{
public:


private:
	static bool Initialize();
	static void Shutdown();

	STATIC_CLASS(Resources);
	friend class Engine;
};