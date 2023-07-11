#CONFIG
name=Skybox
language=GLSL
cull=NONE
front=CLOCKWISE
fill=SOLID
blend=ADD
clear=CLEAR
#CONFIG_END

#VERTEX
#version 450

layout (location = 0) in vec3 position;

layout(binding = 0) uniform LocalConstants
{
    mat4 viewProjection;
    vec4 eye;
    vec4 light;
    float lightRange;
    float lightIntensity;
};

layout (location = 0) out vec3 outUVW;

void main()
{
	outUVW = position;
	gl_Position = viewProjection * vec4(position, 1.0);
}
#VERTEX_END

#FRAGMENT
#version 450

layout (location = 0) in vec3 UVW;

layout (binding = 1) uniform samplerCube samplerEnvMap;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(UVW);
	fragColor = texture(samplerEnvMap, normal);
}
#FRAGMENT_END