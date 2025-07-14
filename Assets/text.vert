#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

layout (location = 2) in vec2 instPosition;
layout (location = 3) in vec2 instTexcoord;
layout (location = 4) in vec4 fgColor;
layout (location = 5) in float scale;
layout (location = 6) in uint textureIndex;

layout (location = 0) out vec2 outTexcoord;
layout (location = 1) out vec4 outFgColor;
layout (location = 2) out float outScale;
layout (location = 3) out flat uint outTextureIndex;

void main()
{
	gl_Position = vec4(position * scale + instPosition, 0.99, 1.0);
    outTexcoord = texcoord + instTexcoord;
	outFgColor = fgColor;
    outTextureIndex = textureIndex;
    outScale = scale;
}