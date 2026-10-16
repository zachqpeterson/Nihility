#include "includes/ShaderTypes.h"

[[vk::binding(0, 0)]] Texture2D<float2> jfaMap;
[[vk::binding(1, 0)]] Sampler2D<float4> emissiveMap;
[[vk::binding(2, 0)]] RWTexture2D<float4> rcAtlas;
[[vk::binding(3, 0)]] RWTexture2D<float4> giOutput;

struct PushConstants
 {
    uint cascadeIndex;
    uint maxCascades;
    float baseRayLength;
};
[[vk::push_constant]] PushConstants pc;

static const int BASE_RAYS = 16;

int GetOffsetY(int c, int H)
{
    return (H * 2 * ((1 << c) - 1)) >> c;
}

void WriteProbe(int c, int2 spatial, int angleIdx, float3 color, int H)
{
    int A_c = BASE_RAYS << c;
    int2 coord = int2(spatial.x * A_c + angleIdx, GetOffsetY(c, H) + spatial.y);
    rcAtlas[coord] = float4(color, 1.0);
}

float3 ReadProbe(int c, int2 spatial, int angleIdx, int W, int H)
{
    int A_c = BASE_RAYS << c;
    spatial.x = clamp(spatial.x, 0, (W >> c) - 1);
    spatial.y = clamp(spatial.y, 0, (H >> c) - 1);
    
    int2 coord = int2(spatial.x * A_c + angleIdx, GetOffsetY(c, H) + spatial.y);
    return rcAtlas[coord].rgb;
}

float3 BilinearReadProbe(int c, float2 spatialPos, int angleIdx, int W, int H) {
    float2 p = spatialPos - 0.5;
    int2 p0 = int2(floor(p));
    int2 p1 = p0 + 1;
    float2 f = frac(p);

    int maxX = (W >> c) - 1;
    int maxY = (H >> c) - 1;

    int2 p00 = clamp(p0, int2(0, 0), int2(maxX, maxY));
    int2 p10 = clamp(int2(p1.x, p0.y), int2(0, 0), int2(maxX, maxY));
    int2 p01 = clamp(int2(p0.x, p1.y), int2(0, 0), int2(maxX, maxY));
    int2 p11 = clamp(p1, int2(0, 0), int2(maxX, maxY));

    float3 v00 = ReadProbe(c, p00, angleIdx, W, H);
    float3 v10 = ReadProbe(c, p10, angleIdx, W, H);
    float3 v01 = ReadProbe(c, p01, angleIdx, W, H);
    float3 v11 = ReadProbe(c, p11, angleIdx, W, H);

    float3 v0 = lerp(v00, v10, f.x);
    float3 v1 = lerp(v01, v11, f.x);
    return lerp(v0, v1, f.y);
}

//float3 StochasticReadProbe(int c, float2 spatialPos, int angleIdx, int W, int H, float randomSeed)
//{
//    float2 p = spatialPos - 0.5; 
//    int2 p0 = int2(floor(p));
//    float2 f = frac(p);
//    
//    float rx = hash(spatialPos * 1.337 + randomSeed);
//    float ry = hash(spatialPos * 2.184 + randomSeed);
//    
//    int2 offset = int2(
//        rx < f.x ? 1 : 0,
//        ry < f.y ? 1 : 0
//    );
//    
//    int2 sampleCoord = p0 + offset;
//    
//    int maxX = (W >> c) - 1;
//    int maxY = (H >> c) - 1;
//    sampleCoord = clamp(sampleCoord, int2(0, 0), int2(maxX, maxY));
//    
//    return ReadProbe(c, sampleCoord, angleIdx, W, H);
//}

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    float2 giSize;
    giOutput.GetDimensions(giSize.x, giSize.y);

    float2 jfaSize;
    jfaMap.GetDimensions(jfaSize.x, jfaSize.y);

    int W = int(giSize.x);
    int H = int(giSize.y);

    int c = (int)pc.cascadeIndex;
    int WC = W >> c;
    int HC = H >> c;

    if (threadId.x >= (uint)WC || threadId.y >= (uint)HC) { return; }
    int2 spatial = int2(threadId.xy);

    float spacing = float(1 << c);
    float2 pixelPos = (float2(spatial) + 0.5) * spacing;

    int rayCount = BASE_RAYS << c;

    float tStart = (c == 0) ? 0.0 : (pc.baseRayLength * pow(4.0, float(c - 1)));
    float tEnd = pc.baseRayLength * pow(4.0, float(c));
    float PI = 3.14159265;

    float3 totalIrradiance = float3(0.0);

    for (int a = 0; a < rayCount; ++a)
    {
        float angle = (float(a) + 0.5) * (2.0 * PI / float(rayCount));
        float2 dir = float2(cos(angle), sin(angle));

        float t = tStart;
        if (c == 0) { t = max(t, 2.0); }

        float3 rayColor = float3(0.0);
        bool hitWall = false;

        while (t < tEnd)
        {
            float2 pos = pixelPos + dir * t;
            if (pos.x < 0.0 || pos.y < 0.0 || pos.x >= jfaSize.x || pos.y >= jfaSize.y) { break; }

            float2 nearestWall = jfaMap.Load(int3(int(pos.x), int(pos.y), 0));
            float wallSDF = distance(pos, nearestWall);

            if (wallSDF <= 1.0)
            {
                hitWall = true;
                break;
            }

            float2 uv = pos / jfaSize;
            float4 emisSample = emissiveMap.SampleLevel(uv, 0.0);
            float lightSDF = emisSample.a;

            float stepSize = max(min(wallSDF - 0.5, lightSDF), 1.0);
            stepSize = min(stepSize, tEnd - t);

            if (lightSDF <= 1.0)
            {
                float2 midPos = pixelPos + dir * (t + stepSize * 0.5);
                float3 midEmission = emissiveMap.SampleLevel(midPos / jfaSize, 0.0).rgb;
                rayColor += midEmission * stepSize;
            }

            t += stepSize;
        }

        if (!hitWall && c < (int)pc.maxCascades - 1)
        {
            float2 upperSpatial = (float2(spatial) + 0.5) / 2.0;

            int child1 = a * 2;
            int child2 = a * 2 + 1;

            float3 upperLight1 = BilinearReadProbe(c + 1, upperSpatial, child1, W, H);
            float3 upperLight2 = BilinearReadProbe(c + 1, upperSpatial, child2, W, H);

            rayColor += (upperLight1 + upperLight2) * 0.5;
        }

        WriteProbe(c, spatial, a, rayColor, H);
        totalIrradiance += rayColor;
    }

    if (c == 0)
    {
        giOutput[spatial] = float4(totalIrradiance / float(rayCount), 1.0);
    }
}