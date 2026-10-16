#include "includes/ShaderTypes.h"

[[vk::binding(0, 0)]] Sampler2D<float4> inputImage;
[[vk::binding(1, 0)]] RWTexture2D<float4> outputImage;

struct PushConstants {
    float2 direction;
};
[[vk::push_constant]] PushConstants pc;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    float2 size;
    outputImage.GetDimensions(size.x, size.y);
    int2 coord = int2(threadId.xy);
    
    if (coord.x >= (int)size.x || coord.y >= (int)size.y) return;

    float2 texelSize = 1.0 / size;
    
    float2 uv = (float2(coord) + 0.5) * texelSize;

    float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
    float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

    float3 result = inputImage.SampleLevel(uv, 0.0).rgb * weight[0];

    float blurRadius = 1.0;

    for (int i = 1; i < 3; ++i)
    {
        float2 d = pc.direction * offset[i] * texelSize * blurRadius;
        result += inputImage.SampleLevel(uv + d, 0.0).rgb * weight[i];
        result += inputImage.SampleLevel(uv - d, 0.0).rgb * weight[i];
    }

    outputImage[coord] = float4(result, 1.0);
}