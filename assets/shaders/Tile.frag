#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec4 inAmbientColor;
flat layout(location = 2) in uint inTexId;

layout(set = 1, binding = 0) uniform localUniformObject
{
    vec4 diffuseColor;
} objectUbo;

layout(set = 1, binding = 1) uniform sampler2D texSamples[];

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSamples[inTexId], inTexcoord) * objectUbo.diffuseColor;
}