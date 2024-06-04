#pragma once

#include "Defines.hpp"

class Steam
{
public:
	static bool Initialize();
	static void Shutdown();

private:

	STATIC_CLASS(Steam);
};