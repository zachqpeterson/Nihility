#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

layout (location = 2) in vec2 instPosition;
layout (location = 3) in vec2 instScale;
layout (location = 4) in vec2 instQuat;
layout (location = 5) in vec4 instColor;
layout (location = 6) in vec2 instTexcoord;
layout (location = 7) in vec2 instTexcoordScale;
layout (location = 8) in uint textureIndex;

layout (location = 0) out vec2 outTexcoord;
layout (location = 1) out vec4 outColor;
layout (location = 2) out flat uint outTextureIndex;

void main()
{
    //vec2 scaled = position * instScale + instPosition;
    //vec2 rotated = vec2(instQuat.y * scaled.x - instQuat.x * scaled.y, instQuat.x * scaled.x + instQuat.y * scaled.y);
	//gl_Position = vec4(rotated + instPosition, 0.0, 1.0);

	gl_Position = vec4(position * instScale + instPosition, 0.0, 1.0);

    outTexcoord = texcoord * instTexcoordScale + instTexcoord;
    outColor = instColor;
    outTextureIndex = textureIndex;
}
