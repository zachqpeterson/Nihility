#version 450

layout(location = 0) in vec2 inTexcoord;

layout(set = 0, binding = 0) uniform sampler2D texSample;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSample, inTexcoord);
}