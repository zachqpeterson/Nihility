#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inTangent;

layout(push_constant) uniform pushConstants
{
	mat4 model;
} push;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outColor;

void main()
{
    outTexcoord = inTexcoord;
    outColor = inColor;
    gl_Position = push.model * vec4(inPosition, 1.0);
}