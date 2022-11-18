#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inColor;

layout(set = 0, binding = 0) uniform globalUniformObject 
{
    mat4 projection;
	mat4 view;
	vec4 ambientColor;
} globalUbo;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outColor;

layout(push_constant) uniform pushConstants
{
	mat4 model;
} push;

void main()
{
    outColor = vec4(inColor, 1.0);
    outTexcoord = inTexcoord;
    gl_Position = globalUbo.projection * globalUbo.view * push.model * vec4(inPosition, 1.0);
}