#pragma once

#include "Defines.hpp"

struct NH_API Semaphore
{
public:
	Semaphore();
	~Semaphore();

	void Destroy();
	bool Wait();
	bool Signal(I32 signalCount = 1);

private:
	void* handle;
};
