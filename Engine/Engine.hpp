#pragma once

#include "Defines.hpp"

class NH_API Engine
{
public:
	static bool Initialize();

private:
	static void Shutdown();
	static void MainLoop();

	STATIC_CLASS(Engine);
};