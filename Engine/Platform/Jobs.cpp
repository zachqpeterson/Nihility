#include "Jobs.hpp"

#include "ThreadSafety.hpp"
#include "Resources\Settings.hpp"
#include "Core\Logger.hpp"


bool Jobs::Busy()
{
	return runningJobs;
}

void Jobs::Wait()
{
	while (Busy()) { Poll(); }
}

void Jobs::Poll()
{
	ReleaseSemaphore(semaphore, 1, nullptr);
	SwitchToThread(); //std::this_thread::yield(), _Thrd_yield()
}

bool Jobs::Execute(const Function<void()>& job)
{
	SafeIncrement(&runningJobs);

	while (!jobs.Push(job)) { Poll(); }

	ReleaseSemaphore(semaphore, 1, nullptr);

	return true;
}

bool Jobs::Dispatch(U32 jobCount, U32 groupSize, const Function<void(JobDispatchArgs)>& job)
{
	if (jobCount == 0 || groupSize == 0) { return false; }

	const U32 groupCount = (jobCount + groupSize - 1) / groupSize;

	SafeAdd(&runningJobs, groupCount);

	for (U32 groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		const auto& jobGroup = [jobCount, groupSize, job, groupIndex]() {

			const U32 groupJobOffset = groupIndex * groupSize;
			U32 end = groupJobOffset + groupSize;
			const U32 groupJobEnd = end < jobCount ? end : jobCount;

			JobDispatchArgs args;
			args.groupIndex = groupIndex;

			for (U32 i = groupJobOffset; i < groupJobEnd; ++i)
			{
				args.jobIndex = i;
				job(args);
			}
		};

		while (!jobs.Push(jobGroup)) { Poll(); }

		ReleaseSemaphore(semaphore, 1, nullptr);
	}

	return true;
}

U32 __stdcall Jobs::RunThread(void*)
{
	Function<void()> job;

	while (running)
	{
		if (jobs.Pop(job))
		{
			job();
			SafeDecrement(&runningJobs);
		}
		else { WaitForSingleObjectEx(semaphore, INFINITE, false); }
	}

	_endthreadex(0);
	return 0;
}

#endif