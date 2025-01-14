#include "Semaphore.hpp"

#if defined(NH_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(NH_PLATFORM_APPLE)
#elif defined(NH_PLATFORM_LINUX)
#elif defined(NH_PLATFORM_UNIX)
#elif defined(NH_PLATFORM_POSIX)
#endif

Semaphore::Semaphore() : handle(CreateSemaphoreExW(nullptr, 0, L32_MAX, nullptr, 0, SEMAPHORE_ALL_ACCESS)) {}
Semaphore::~Semaphore() { Destroy(); }

void Semaphore::Destroy() { ReleaseSemaphore(handle, L32_MAX, nullptr); CloseHandle(handle); }
bool Semaphore::Wait() { return WaitForSingleObjectEx(handle, INFINITE, false) != WAIT_FAILED; }
bool Semaphore::Signal(I32 signalCount) { return ReleaseSemaphore(handle, signalCount, nullptr); }