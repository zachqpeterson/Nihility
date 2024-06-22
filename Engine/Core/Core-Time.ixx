module;

#include "Defines.hpp"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

export module Core:Time;

export class NH_API Time
{
public:
	static const F64& DeltaTime() { return delta; }
	static const F64& FrameEndTime() { return frameEndTime; }
	static const U32& FrameRate(){ return frameRate; }
	static F64 UpTime() { return AbsoluteTime() - programStart; }
	static F64 FrameUpTime() { return AbsoluteTime() - frameEndTime; }

	static F64 AbsoluteTime()
	{
#if defined(PLATFORM_WINDOWS)
		LARGE_INTEGER nowTime;
		QueryPerformanceCounter(&nowTime);

		return (F64)nowTime.QuadPart * clockFrequency;
#endif
	}

	static I64 CoreCounter()
	{
#if defined(PLATFORM_WINDOWS)
		LARGE_INTEGER nowTime;
		QueryPerformanceCounter(&nowTime);

		return nowTime.QuadPart;
#endif
	}

private:
	static bool Initialize()
	{
		frameEndTime = programStart;
		delta = 0.0;
		frameTimer = 0.0;
		frameRate = 0;
		frameCounter = 0;

		return true;
	}

	static void Shutdown() {}

	static void Update()
	{
		F64 now = AbsoluteTime();

		delta = now - frameEndTime;
		frameEndTime = now;
		frameTimer += delta;
		++frameCounter;

		if (frameTimer >= 1.0)
		{
			frameTimer -= 1.0;
			frameRate = frameCounter;
			frameCounter = 0;
		}
	}

	static F64 ClockFrequency()
	{
#if defined(PLATFORM_WINDOWS)
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return 1.0 / (F64)frequency.QuadPart;
#endif
	}

	static F64 ProgramStart()
	{
#if defined(PLATFORM_WINDOWS)
		LARGE_INTEGER startTime;
		QueryPerformanceCounter(&startTime);

		return (F64)startTime.QuadPart * clockFrequency;
#endif
	}

	static inline F64 clockFrequency{ ClockFrequency() };
	static inline F64 programStart{ ProgramStart() };
	static inline F64 frameEndTime;
	static inline F64 delta;
	static inline F64 frameTimer;
	static inline U32 frameRate;
	static inline U32 frameCounter;

	STATIC_CLASS(Time);
	friend class Engine;
};

export struct NH_API Timer
{
	Timer() : start{ 0.0 }, elapsedTime{ 0.0 }, running{ false } {}

	void Start()
	{
		if (!running) { start = Time::AbsoluteTime(); }
		running = true;
	}

	void Stop()
	{
		if (running) { elapsedTime += Time::AbsoluteTime() - start; }
		running = false;
	}

	void Reset()
	{
		start = 0.0;
		elapsedTime = 0.0;
		running = false;
	}

	void Restart()
	{
		start = Time::AbsoluteTime();
		elapsedTime = 0.0;
		running = true;
	}

	F64 CurrentTime() const
	{
		if (running) { return elapsedTime + Time::AbsoluteTime() - start; }

		return elapsedTime;
	}

private:
	F64 start;
	F64 elapsedTime;
	bool running;
};