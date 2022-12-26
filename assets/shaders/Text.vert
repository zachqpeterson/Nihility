#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inColor;

layout(push_constant) uniform pushConstants
{
	vec4 area;
	vec2 position;
} push;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec4 outArea;

void main()
{
	outTexcoord = inTexcoord;
	outColor = inColor;
	outArea = push.area;
	gl_Position = vec4((inPosition + vec3(push.position, 0.0)) * 2.0 - 1.0, 1.0);
}