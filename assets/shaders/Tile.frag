#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec4 inAmbientColor;
flat layout(location = 2) in uint inTexId;

layout(set = 1, binding = 0) uniform localUniformObject
{
    vec4 diffuseColor;
} objectUbo;

layout(set = 0, binding = 1) uniform sampler2D texSamples[];

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 texSize = vec2(24.0, 24.0) / textureSize(texSamples[inTexId], 0);
    vec2 texCoord = inTexcoord.xy * texSize.xy;

    outColor = texture(texSamples[inTexId], texCoord) * objectUbo.diffuseColor;
}