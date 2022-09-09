#pragma once

#include "Defines.hpp"

class Time
{
public:
	static NH_API const F64& DeltaTime();
	static NH_API const F64 UpTime();
	static NH_API const F64 TimeSinceLastFrame();
	static NH_API const F64& FrameEndTime();
	static NH_API const U16& FrameRate();

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	Time() = delete;

	static F64 programStart;
	static F64 frameEndTime;
	static F64 delta;
	static F64 frameTimer;
	static U16 frameRate;
	static U16 frameCounter;

	friend class Engine;
};

struct Timer
{
	Timer();

	void Start();
	void Stop();
	const F64 CurrentTime() const;
	void Reset();
	void Restart();

private:
	F64 start;
	F64 elapsedTime;
	bool running;
};