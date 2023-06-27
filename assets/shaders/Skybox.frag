#version 450

layout (location = 0) in vec3 UVW;

layout (binding = 1) uniform samplerCube samplerEnvMap;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(UVW);
	fragColor = texture(samplerEnvMap, normal);
}