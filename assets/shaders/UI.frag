#version 450

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec4 inColor;

layout(set = 1, binding = 0) uniform sampler2D texSample;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSample, inTexcoord) * inColor;
}