#version 450

layout (location = 0) in float depth;
layout (location = 1) in uint tileOffset;

layout (location = 0) out flat uint outInstance;
layout (location = 1) out flat uint outTileOffset;

void main()
{
	gl_Position = vec4(vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0 + -1.0, depth, 1.0);

	outInstance = gl_InstanceIndex;
	outTileOffset = tileOffset;
}
