#version 450

layout(push_constant) uniform constants
{
	mat4 viewProjection;
} Globals;

layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;

layout (location = 0) out vec4 outColor;

void main()
{
    gl_Position = Globals.viewProjection * vec4(position, 0.0, 1.0);

    outColor = color;
}
