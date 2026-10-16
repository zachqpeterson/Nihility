#include "includes/ShaderTypes.h"

struct PushConstants
{
	int stepSize;
	int isPing;
};

[[vk::push_constant]] PushConstants pc;

[[vk::binding(0, 0)]] RWTexture2D<float2> texA;
[[vk::binding(1, 0)]] RWTexture2D<float2> texB;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	float2 size;
	texA.GetDimensions(size.x, size.y);
	int2 coord = int2(threadId.xy);

	if (coord.x >= (int)size.x || coord.y >= (int)size.y) { return; }

	float2 bestSeed;
	if (pc.isPing == 1) { bestSeed = texA.Load(coord); }
	else { bestSeed = texB.Load(coord); }

    float2 pixelCenter = float2(coord) + 0.5;
	float bestDist = distance(pixelCenter, bestSeed);

	const int2 offsets[8] = {
		int2(-1, -1), int2(0, -1), int2(1, -1),
		int2(-1,  0),              int2(1,  0),
		int2(-1,  1), int2(0,  1), int2(1,  1)
	};

	for (int i = 0; i < 8; ++i)
	{
		int2 neighborCoord = coord + (offsets[i] * pc.stepSize);

		neighborCoord = clamp(neighborCoord, int2(0, 0), int2(int(size.x - 1.0), int(size.y - 1.0)));

		float2 neighborSeed;
		if (pc.isPing == 1) { neighborSeed = texA.Load(neighborCoord); }
		else { neighborSeed = texB.Load(neighborCoord); }

        float distToNeighborSeed = distance(pixelCenter, neighborSeed);

		if (distToNeighborSeed < bestDist)
		{
			bestDist = distToNeighborSeed;
			bestSeed = neighborSeed;
		}
	}

	if (pc.isPing == 1) { texB[coord] = bestSeed; }
	else { texA[coord] = bestSeed; }
}