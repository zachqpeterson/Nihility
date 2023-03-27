#pragma once

#include "RenderingDefines.hpp"

class NH_API Renderer
{
public:


private:
	static bool Initialize(const C8* applicationName);
	static void Shutdown();
	static void Update();

	STATIC_CLASS(Renderer);
	friend class Engine;
};