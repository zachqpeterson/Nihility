#pragma once

#include "Defines.hpp"

#undef Yield

class NH_API Jobs
{
public:
	static void Yield();

private:


	STATIC_CLASS(Jobs);
};