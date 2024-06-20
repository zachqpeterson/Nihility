#pragma once

#include "Defines.hpp"

#include "Containers\Freelist.hpp"
#include "Containers\SafeQueue.hpp"
#include "Core\Function.hpp"

/*
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for semaphore/fence
* TODO: This syntax would be ideal: StartJob<func>(param, param, ...);
*/
class NH_API Jobs
{
public:
	static bool Execute(const Function<void()>& job);
	static bool Dispatch(U32 jobCount, U32 groupSize, const Function<void(JobDispatchArgs)>& job);

	static bool Busy();
	static void Wait();

private:

	static void Poll();

};