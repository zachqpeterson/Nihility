#include "Profiler.hpp"

#include "Renderer.hpp"
#include "Math\Color.hpp"

Timestamp* Profiler::timestamps{ nullptr };
U64* Profiler::timestampsData{ nullptr };
U16* Profiler::perFrameActive{ nullptr };

U32	Profiler::maxFrames;
U32	Profiler::currentFrame{ 0 };

F32	Profiler::maxTime{ 0.0f };
F32	Profiler::minTime{ 0.0f };
F32	Profiler::averageTime{ 0.0f };

U32 Profiler::initialFramesPaused{ 3 };
F32	Profiler::maxDuration{ 16.666f };
bool Profiler::paused{ false };

U32 Profiler::queriesPerFrame{ 0 };
U32 Profiler::currentQuery{ 0 };
U32 Profiler::parentIndex{ 0 };
U32 Profiler::depth{ 0 };

bool Profiler::currentFrameResolved{ false };

Hashmap<U64, U32> Profiler::colors{ 16, U32_MAX };

void Profiler::Initialize(U16 queriesPerFrame_, U32 maxFrames_)
{
	maxFrames = maxFrames_;
	queriesPerFrame = queriesPerFrame_;

	constexpr U32 dataPerQuery = 2;

	Memory::AllocateArray(&timestamps, maxFrames * queriesPerFrame);
	Memory::AllocateArray(&timestampsData, queriesPerFrame * maxFrames * dataPerQuery);
	Memory::AllocateArray(&perFrameActive, maxFrames);
}

void Profiler::Shutdown()
{
	colors.Destroy();

	Memory::FreeArray(&timestamps);
	Memory::FreeArray(&timestampsData);
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

	U32 activeTimestamps = Resolve(Renderer::previousFrame, &timestamps[32 * currentFrame]);
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

void Profiler::Query()
{
	if (Renderer::timestampsEnabled)
	{
		if (HasValidQueries())
		{
			// Query GPU for all timestamps.
			const U32 queryOffset = (currentFrame * queriesPerFrame) * 2;
			const U32 queryCount = currentQuery * 2;
			vkGetQueryPoolResults(Renderer::device, Renderer::timestampQueryPool, queryOffset, queryCount,
				sizeof(U64) * queryCount * 2, &timestampsData[queryOffset],
				sizeof(timestampsData[0]), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

			// Calculate and cache the elapsed time
			for (U32 i = 0; i < currentQuery; ++i)
			{
				U32 index = (currentFrame * queriesPerFrame) + i;

				Timestamp& timestamp = timestamps[index];

				F64 start = (F64)timestampsData[(index * 2)];
				F64 end = (F64)timestampsData[(index * 2) + 1];
				F64 range = end - start;
				F64 elapsedTime = range * Renderer::timestampFrequency;

				timestamp.elapsedMs = elapsedTime;
				timestamp.frameIndex = Renderer::absoluteFrame;
			}
		}
		else if (currentQuery)
		{
			Logger::Error("Asymmetrical GPU queries, missing pop of some markers!\n");
		}

		Reset();
		Renderer::timestampReset = true;
	}
	else
	{
		Renderer::timestampReset = false;
	}
}

bool Profiler::HasValidQueries()
{
	return currentQuery > 0 && depth == 0;
}

void Profiler::Reset()
{
	currentQuery = 0;
	parentIndex = 0;
	currentFrameResolved = false;
	depth = 0;
}

U32 Profiler::Resolve(U32 currentFrame, Timestamp* timestampsToFill)
{
	memcpy(timestampsToFill, &timestamps[currentFrame * queriesPerFrame], sizeof(Timestamp) * currentQuery);
	return currentQuery;
}

U32 Profiler::Push(U32 currentFrame, const String& name)
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

U32 Profiler::Pop(U32 currentFrame)
{
	U32 queryIndex = (currentFrame * queriesPerFrame) + parentIndex;
	Timestamp& timestamp = timestamps[queryIndex];
	parentIndex = timestamp.parentIndex;
	--depth;

	return (queryIndex * 2) + 1;
}