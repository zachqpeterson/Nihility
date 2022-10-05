#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in uint inTexId;

layout(set = 0, binding = 0) uniform globalUniformObject 
{
    mat4 projection;
	mat4 view;
	vec4 ambientColor;
} globalUbo;

layout(push_constant) uniform pushConstants
{
	mat4 model;
} push;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outAmbientColor;
layout(location = 2) out uint outTexId;

void main()
{
    outTexcoord = inTexcoord;
    outTexId = inTexId;
    gl_Position = globalUbo.projection * globalUbo.view * push.model * vec4(inPosition, 1.0);
}