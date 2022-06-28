#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;

layout(set = 0, binding = 0) uniform globalUniformObject 
{
    mat4 projection;
	mat4 view;
	vec4 ambientColor;
} globalUbo;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outAmbientColor;

void main()
{
    outTexcoord = inTexcoord;
    gl_Position = globalUbo.projection * globalUbo.view * vec4(inPosition, 1.0);
}