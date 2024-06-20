module;

#include "Defines.hpp"

export module Multithreading:Semaphore;

#if defined(PLATFORM_WINDOWS)
#include "Windows.h"

export struct NH_API Semaphore
{
public:
	Semaphore() : handle(CreateSemaphoreExW(nullptr, 0, L32_MAX, nullptr, 0, SEMAPHORE_ALL_ACCESS)) {}
	~Semaphore() { Destroy(); }

	void Destroy() { ReleaseSemaphore(handle, L32_MAX, nullptr); CloseHandle(handle); }
	bool Wait() { return WaitForSingleObject(handle, INFINITE) != WAIT_FAILED; }
	bool Signal(I32 signalCount) { return ReleaseSemaphore(handle, signalCount, nullptr); }

private:
	void* handle;
};

#elif defined(PLATFORM_APPLE)
#elif defined(PLATFORM_LINUX)
#elif defined(PLATFORM_UNIX)
#elif defined(PLATFORM_POSIX)
#endif