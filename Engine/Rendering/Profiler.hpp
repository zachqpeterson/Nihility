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

class Profiler
{
private:
	static void Initialize(U16 queriesPerFrame, U32 maxFrames);
	static void Shutdown();

	static void Update();
	static void Query();

	static bool HasValidQueries();

	static U32 Push(U32 currentFrame, const String& name); // Returns the timestamp query index.
	static U32 Pop(U32 currentFrame);

private:
	static void Reset();
	static U32 Resolve(U32 currentFrame, Timestamp* timestampsToFill); // Returns the total queries for this frame.

	static Timestamp* timestamps;
	static U64* timestampsData;
	static U16* perFrameActive;

	static U32 maxFrames;
	static U32 currentFrame;
			   
	static F32 maxTime;
	static F32 minTime;
	static F32 averageTime;
			   
	static U32 initialFramesPaused;
	static F32 maxDuration;
	static bool paused;

	static U32 queriesPerFrame;
	static U32 currentQuery;
	static U32 parentIndex;
	static U32 depth;

	static bool currentFrameResolved;

	static Hashmap<U64, U32> colors;

	STATIC_CLASS(Profiler);
	friend class Renderer;
};