#CONFIG
name=Composition
language=GLSL
cull=NONE
front=CLOCKWISE
fill=SOLID
blend=ADD
#CONFIG_END

#VERTEX
#version 450

layout (location = 0) out vec2 outUV;

void main()
{
	outUV = vec2((gl_VertexIndex & 1) << 1, 1 - (gl_VertexIndex & 2));
	gl_Position = vec4(outUV.x - 1.0 + (gl_VertexIndex & 1) * 2.0, outUV.y - 2.0 + (gl_VertexIndex & 2) * 3.0, 0.0, 1.0);
}
#VERTEX_END

#FRAGMENT
#version 450

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform sampler2D samplerColor0; //Skybox
layout (binding = 1) uniform sampler2D samplerColor1; //Geometry

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(samplerColor0, inUV);
    vec4 geometry = texture(samplerColor1, inUV);

    if(geometry.a > 0.0) { outColor = geometry; }
}
#FRAGMENT_END