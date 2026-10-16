#include "includes/ShaderTypes.h"

struct PushConstants {
	uint albedoId;
    uint giId;
    uint jfaId;
};
[[vk::push_constant]] PushConstants pc;

[[vk::binding(0, 0)]]
[GlobalTextures]
Sampler2D globalTextures[];

struct VSOutput {
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
};

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

[shader("vertex")]
VSOutput vsMain(uint vertexId : SV_VertexID)
{
	VSOutput output;
	
	output.uv = float2((vertexId << 1) & 2, vertexId & 2);
	output.position = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
	return output;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target
{
    float4 albedo = globalTextures[pc.albedoId].Sample(input.uv);

    float2 logicalScreen = float2(1920.0, 1080.0);
    float2 padding = float2(512.0, 512.0);
    float2 overscanSize = logicalScreen + (padding * 2.0);

    float2 overscanUV = (padding + logicalScreen * input.uv) / overscanSize;

    float2 giSize;
    globalTextures[pc.giId].GetDimensions(giSize.x, giSize.y);
    float2 texelSize = 1.0 / giSize;

    float3 blurredGI = globalTextures[pc.giId].Sample(overscanUV).rgb;

    float3 ambient = float3(0.005, 0.005, 0.008);

    float3 finalColor = albedo.rgb * (blurredGI + ambient);
    finalColor = ACESFilm(finalColor);
    finalColor += (hash(input.uv) - 0.5) / 255.0;

    return float4(finalColor, albedo.a);
}