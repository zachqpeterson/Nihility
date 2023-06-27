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