module;

#include "Defines.hpp"

#if defined(NH_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(NH_PLATFORM_APPLE)
#elif defined(NH_PLATFORM_LINUX)
#elif defined(NH_PLATFORM_UNIX)
#elif defined(NH_PLATFORM_POSIX)
#endif

export module Multithreading:Semaphore;

#if defined(NH_PLATFORM_WINDOWS)

export struct NH_API Semaphore
{
public:
	Semaphore() : handle(CreateSemaphoreExW(nullptr, 0, L32_MAX, nullptr, 0, SEMAPHORE_ALL_ACCESS)) {}
	~Semaphore() { Destroy(); }

	void Destroy() { ReleaseSemaphore(handle, L32_MAX, nullptr); CloseHandle(handle); }
	bool Wait() { return WaitForSingleObjectEx(handle, INFINITE, false) != WAIT_FAILED; }
	bool Signal(I32 signalCount = 1) { return ReleaseSemaphore(handle, signalCount, nullptr); }

private:
	void* handle;
};

#elif defined(NH_PLATFORM_APPLE)
#elif defined(NH_PLATFORM_LINUX)
#elif defined(NH_PLATFORM_UNIX)
#elif defined(NH_PLATFORM_POSIX)
#endif