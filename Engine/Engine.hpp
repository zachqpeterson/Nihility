#pragma once

#include "Defines.hpp"

class NH_API Engine
{
public:
	static bool Initialize();
	static void Shutdown();

private:
	static void MainLoop();

	STATIC_CLASS(Engine);
};