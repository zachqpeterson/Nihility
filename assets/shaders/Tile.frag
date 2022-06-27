#version 450

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec4 inAmbientColor;

layout(set = 1, binding = 0) uniform localUniformObject
{
    vec4 diffuseColor;
} objectUbo;

layout(set = 1, binding = 1) uniform sampler2D texSample;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSample, inTexcoord) * objectUbo.diffuseColor;
}