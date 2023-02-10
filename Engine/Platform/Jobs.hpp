#pragma once

#include "Defines.hpp"

/*
* TODO: Function Pointer type for starting a job
* TODO: Have to call CloseHandle() after thread ends
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for seconds
* TODO: Wait for semaphore/fence
*/
class NH_API Jobs
{
public:
	static bool StartJob();

private:
	static bool Initialize();
	static void Shutdown();

#if defined PLATFORM_WINDOWS
	static U32 __stdcall StartThread(void* func);
#endif

	STATIC_CLASS(Jobs);
	friend class Engine;
};