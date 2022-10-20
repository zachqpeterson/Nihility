#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inColor;
layout(location = 3) in uint inTexId;

layout(set = 0, binding = 0) uniform globalUniformObject 
{
    mat4 projection;
	mat4 view;
	vec3 ambientColor;
} globalUbo;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec3 outColor;
layout(location = 2) out uint outTexId;

void main()
{
    outTexcoord = inTexcoord;
    outTexId = inTexId;
    outColor = globalUbo.ambientColor * inColor;
    gl_Position = globalUbo.projection * globalUbo.view * vec4(inPosition, 1.0);
}