static const uint INVALID_TEXTURE_INDEX = 65535;

[[vk::binding(1, 10)]]
Sampler2D globalTextures[];

struct VSInput
{
    float2 position;
    float2 texcoord;

    float2 instPosition;
    float2 instScale;
    float2 instQuat;
    float4 instColor;
    float2 instTexcoord;
    float2 instTexcoordScale;
    uint textureIndex;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord;
    float4 color;
    uint textureIndex;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    output.position = float4(input.position * input.instScale + input.instPosition, 0.0, 1.0);
    output.texcoord = input.texcoord * input.instTexcoordScale + input.instTexcoord;
    output.color = input.instColor;
    output.textureIndex = input.textureIndex;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    if (input.textureIndex != INVALID_TEXTURE_INDEX) { return globalTextures[input.textureIndex].Sample(input.texcoord) * input.color; }
    else { return input.color; }
}