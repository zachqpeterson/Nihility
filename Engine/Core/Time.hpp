#pragma once

#include "Defines.hpp"

#if defined PLATFORM_WINDOWS
#include <Windows.h>
#endif

struct NH_API Timer
{
	Timer();

	void Start();
	void Stop();
	void Reset();
	void Restart();
	F64 CurrentTime() const;

private:
	F64 start;
	F64 elapsedTime;
	bool running;
};

class NH_API Time
{
public:
	static const F64& DeltaTime();
	static const F64& FrameEndTime();
	static const U32& FrameRate();
	static F64 UpTime();
	static F64 FrameUpTime();
	static F64 AbsoluteTime();

private:
	static bool Initialize();
	static void Update();
	static void Shutdown();

	static inline F64 clockFrequency;
	static inline F64 programStart;
	static inline F64 frameEndTime;
	static inline F64 delta;
	static inline F64 frameTimer;
	static inline U32 frameRate;
	static inline U32 frameCounter;

	STATIC_CLASS(Time);
	friend class Engine;
};

inline bool Time::Initialize()
{
#if defined PLATFORM_WINDOWS
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	clockFrequency = 1.0 / (F64)frequency.QuadPart;

	programStart = (F64)startTime.QuadPart * clockFrequency;
#endif

	frameEndTime = programStart;
	delta = 0.0;
	frameTimer = 0.0;
	frameRate = 0;
	frameCounter = 0;

	return true;
}

inline void Time::Shutdown()
{

}

inline void Time::Update()
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

inline const F64& Time::DeltaTime()
{
	return delta;
}

inline const F64& Time::FrameEndTime()
{
	return frameEndTime;
}

inline const U32& Time::FrameRate()
{
	return frameRate;
}

inline F64 Time::UpTime()
{
	return AbsoluteTime() - programStart;
}

inline F64 Time::FrameUpTime()
{
	return AbsoluteTime() - frameEndTime;
}

inline F64 Time::AbsoluteTime()
{
#if defined PLATFORM_WINDOWS
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter(&nowTime);

	return (F64)nowTime.QuadPart * clockFrequency;
#endif
}

inline Timer::Timer() : start{ Time::AbsoluteTime() }, elapsedTime{ 0.0 }, running{ false } {}

inline void Timer::Start()
{
	start += !running * (Time::AbsoluteTime() - start);
	running = true;
}

inline void Timer::Stop()
{
	elapsedTime += running * (Time::AbsoluteTime() - start);
	running = false;
}

inline void Timer::Reset()
{
	start = 0.0;
	elapsedTime = 0.0;
	running = false;
}

inline void Timer::Restart()
{
	start = Time::AbsoluteTime();
	elapsedTime = 0.0;
	running = true;
}

inline F64 Timer::CurrentTime() const
{
	return elapsedTime + running * (Time::AbsoluteTime() - start);
}