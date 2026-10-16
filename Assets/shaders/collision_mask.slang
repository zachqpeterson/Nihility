#include "includes/ShaderTypes.h"

struct PushConstants
{
    float4x4 viewProj;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct VertexInput
{
    float3 position;
    uint typeData;
};

struct VertexOutput
{
	float4 position : SV_Position;
	uint collisionType : BLENDINDICES;
};

[shader("vertex")]
VertexOutput vsMain(VertexInput input)
{
    VertexOutput output;
	output.position = mul(pc.viewProj, float4(input.position, 1.0));
	
	output.collisionType = input.typeData >> 4; 
	return output;
}

[shader("fragment")]
float4 fsMain(VertexOutput input) : SV_Target
{
	if (input.collisionType != 1) 
	{
		discard;
	}
	
	return float4(1.0, 1.0, 1.0, 1.0);
}