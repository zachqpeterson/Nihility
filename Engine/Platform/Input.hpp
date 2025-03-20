#pragma once

#include "Defines.hpp"

class NH_API Input
{
public:


private:
	static bool Initialize();
	static void Shutdown();

	static void Reset();
	static void Update();
	static void UpdateRawInput(I64 lParam);
	static void UpdateConnectionStatus(void* deviceHandle, U64 status);

	friend class Engine;
	friend class Platform;

	STATIC_CLASS(Input);
};