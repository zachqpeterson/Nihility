#pragma once

#include "RenderingDefines.hpp"
#include "Math\Color.hpp"
#include "Containers\Hashmap.hpp"

struct Timestamp
{
	U32			start;
	U32			end;

	F64			elapsedMs;

	U16			parentIndex;
	U16			depth;

	ColorRGB	color;
	U32			frameIndex;

	String		name;
};

struct TimestampManager
{
	void Create(U16 queriesPerFrame, U16 maxFrames);
	void Destroy();

	bool HasValidQueries() const;
	void Reset();
	U32 Resolve(U32 currentFrame, Timestamp* timestampsToFill);    // Returns the total queries for this frame.

	U32 Push(U32 currentFrame, const char* name);    // Returns the timestamp query index.
	U32 Pop(U32 currentFrame);

	Timestamp* timestamps = nullptr;
	U64* timestampsData = nullptr;

	U32				queriesPerFrame = 0;
	U32				currentQuery = 0;
	U32				parentIndex = 0;
	U32				depth = 0;

	bool			currentFrameResolved = false;    // Used to query the GPU only once per frame if get_gpu_timestamps is called more than once per frame.
};

class Profiler
{
private:
	static void Create(U32 maxFrames);
	static void Destroy();

	static void Update();

private:
	static Timestamp* timestamps;
	static U16* perFrameActive;

	static U32 maxFrames;
	static U32 currentFrame;
			   
	static F32 maxTime;
	static F32 minTime;
	static F32 averageTime;
			   
	static U32 initialFramesPaused;
	static F32 maxDuration;
	static bool paused;

	static Hashmap<U64, U32> colors;

	STATIC_CLASS(Profiler);
	friend class Renderer;
};