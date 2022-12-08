#pragma once

#include "Defines.hpp"

class NH_API Time
{
public:


	static const F64& DeltaTime();
	static const F64 UpTime();
	static const F64 AbsoluteTime();
	static const F64 TimeSinceLastFrame();
	static const F64& FrameEndTime();
	static const U16& FrameRate();

private:
	static bool Initialize();
	static void Update();

	static F64 programStart;
	static F64 frameEndTime;
	static F64 delta;
	static F64 frameTimer;
	static U16 frameRate;
	static U16 frameCounter;
	static F64 clockFrequency;

	Time() = delete;

	friend class Engine;
};

struct NH_API Timer
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