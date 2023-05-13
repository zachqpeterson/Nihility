#include "Profiler.hpp"

#include "Renderer.hpp"
#include "Math\Color.hpp"

Timestamp* Profiler::timestamps;
U16* Profiler::perFrameActive;

U32	Profiler::maxFrames;
U32	Profiler::currentFrame;

F32	Profiler::maxTime;
F32	Profiler::minTime;
F32	Profiler::averageTime;

U32 Profiler::initialFramesPaused{ 3 };
F32	Profiler::maxDuration;
bool Profiler::paused;

Hashmap<U64, U32> Profiler::colors{ 16, U32_MAX };

// GPU TIMESTAMP MANAGER

void TimestampManager::Create(U16 queriesPerFrame, U16 maxFrames)
{
	this->queriesPerFrame = queriesPerFrame;

	const U32 dataPerQuery = 2;
	const U64 allocatedSize = sizeof(Timestamp) * queriesPerFrame * maxFrames + sizeof(U64) * queriesPerFrame * maxFrames * dataPerQuery;
	Memory::AllocateSize(&timestamps, allocatedSize);

	timestampsData = (U64*)((U8*)timestamps + sizeof(Timestamp) * queriesPerFrame * maxFrames);

	Reset();
}

void TimestampManager::Destroy()
{
	Memory::FreeSize(&timestamps);
}

bool TimestampManager::HasValidQueries() const
{
	return currentQuery > 0 && (depth == 0);
}

void TimestampManager::Reset()
{
	currentQuery = 0;
	parentIndex = 0;
	currentFrameResolved = false;
	depth = 0;
}

U32 TimestampManager::Resolve(U32 currentFrame, Timestamp* timestampsToFill)
{
	memcpy(timestampsToFill, &timestamps[currentFrame * queriesPerFrame], sizeof(Timestamp) * currentQuery);
	return currentQuery;
}

U32 TimestampManager::Push(U32 currentFrame, const char* name)
{
	U32 queryIndex = (currentFrame * queriesPerFrame) + currentQuery;

	Timestamp& timestamp = timestamps[queryIndex];
	timestamp.parentIndex = (U16)parentIndex;
	timestamp.start = queryIndex * 2;
	timestamp.end = timestamp.start + 1;
	timestamp.name = name;
	timestamp.depth = (U16)depth++;

	parentIndex = currentQuery;
	++currentQuery;

	return (queryIndex * 2);
}

U32 TimestampManager::Pop(U32 currentFrame)
{
	U32 queryIndex = (currentFrame * queriesPerFrame) + parentIndex;
	Timestamp& timestamp = timestamps[queryIndex];
	parentIndex = timestamp.parentIndex;
	--depth;

	return (queryIndex * 2) + 1;
}

//GPU PROFILER

void Profiler::Create(U32 maxFrames_)
{
	maxFrames = maxFrames_;
	Memory::AllocateArray(&timestamps, maxFrames * 32);
	Memory::AllocateArray(&perFrameActive, maxFrames);

	maxDuration = 16.666f;
	currentFrame = 0;
	minTime = maxTime = averageTime = 0.f;
	paused = false;

	memset(perFrameActive, 0, 2 * maxFrames);
}

void Profiler::Destroy()
{
	colors.Destroy();

	Memory::FreeArray(&timestamps);
	Memory::FreeArray(&perFrameActive);
}

void Profiler::Update()
{
	Renderer::SetGpuTimestampsEnable(!paused);

	if (initialFramesPaused)
	{
		--initialFramesPaused;
		return;
	}

	if (paused && !Renderer::resized) { return; }

	U32 activeTimestamps = Renderer::GetGpuTimestamps(&timestamps[32 * currentFrame]);
	perFrameActive[currentFrame] = (U16)activeTimestamps;

	for (U32 i = 0; i < activeTimestamps; ++i)
	{
		Timestamp& timestamp = timestamps[32 * currentFrame + i];

		U32 colorIndex = colors[timestamp.name.Hash()];

		if (colorIndex == U32_MAX)
		{
			colorIndex = (U32)colors.Size();
			colors.Insert(timestamp.name.Hash(), colorIndex);
		}

		timestamp.color = ColorRGB::DistinctColor(colorIndex);
	}

	currentFrame = (currentFrame + 1) % maxFrames;

	// Reset Min/Max/Average after few frames
	if (currentFrame == 0)
	{
		maxTime = F32_MIN;
		minTime = F32_MAX;
		averageTime = 0.f;
	}
}