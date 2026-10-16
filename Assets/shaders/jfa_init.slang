#include "includes/ShaderTypes.h"

[[vk::binding(0, 0)]] Texture2D<float4> gBufferCollisionMask;
[[vk::binding(1, 0)]] RWTexture2D<float2> seedOutput;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	float2 size;
	gBufferCollisionMask.GetDimensions(size.x, size.y);
	
	if (threadId.x >= (uint)size.x || threadId.y >= (uint)size.y) { return; }

	int2 coord = int2(threadId.xy);

    float isSolid = gBufferCollisionMask.Load(int3(coord, 0)).r;

    if (isSolid > 0.5)
    {
        seedOutput[coord] = float2(coord) + 0.5;
	}
	else
	{
		seedOutput[coord] = float2(99999.0, 99999.0);
	}
}