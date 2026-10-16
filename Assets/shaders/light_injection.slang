#include "includes/ShaderTypes.h"

struct PointLight
{
	float2 position;
	float radius;
	float intensity;
	float4 color;
};

struct PushConstants
{
    float2 cameraPos;
	 float cameraZoom;
    float overscanWidth;
    float overscanHeight;
    uint lightCount;
};
[[vk::push_constant]] PushConstants pc;

[[vk::binding(0, 0)]] StructuredBuffer<PointLight> lights;
[[vk::binding(1, 0)]] RWTexture2D<float4> emissiveOutput;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	float2 size;
	emissiveOutput.GetDimensions(size.x, size.y);
	int2 coord = int2(threadId.xy);

	if (coord.x >= (int)size.x || coord.y >= (int)size.y) { return; }

    float2 fullResPixel = (float2(coord) + 0.5) * 2.0;
    float2 center = float2(pc.overscanWidth, pc.overscanHeight) * 0.5;
    float safeZoom = max(pc.cameraZoom, 0.001);
    float2 worldPos = ((fullResPixel - center) / safeZoom) + pc.cameraPos;

    float3 accumulatedLight = float3(0.0);
    float minLightSDF = 4096.0;

	for (uint i = 0; i < pc.lightCount; ++i)
	{
		PointLight light = lights[i];

        float minRadius = 24.0;
        float giRadius = max(light.radius, minRadius);

        float dist = distance(worldPos, light.position);

        float distToLightEdge = max(dist - giRadius, 0.0) * safeZoom * 0.5;
        minLightSDF = min(minLightSDF, distToLightEdge);

        if (dist < giRadius)
        {
            float radiusRatio = light.radius / giRadius;
            float giIntensity = (light.intensity * radiusRatio * radiusRatio) / (giRadius * safeZoom * 0.25);

            float t = 1.0 - (dist / giRadius);
            float smoothFalloff = t * t * t;

            accumulatedLight += light.color.rgb * giIntensity * smoothFalloff;
		}
	}

    emissiveOutput[coord] = float4(accumulatedLight, minLightSDF);
}