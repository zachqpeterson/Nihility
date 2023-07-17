#CONFIG
name=PostProcess
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

layout(std140, binding = 0) uniform LocalConstants
{
    float contrast;
    float brightness;
    float saturation;
    float gammaCorrection;
};

layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outColor;

const vec3 greyscale = vec3(0.299, 0.587, 0.114);

void main()
{
    outColor = texture(samplerColor, inUV);

    //contrast and brightness
	outColor.rgb = clamp(contrast * (outColor.rgb - 0.5) + 0.5 + brightness, 0.0, 1.0);
	//saturation
	outColor.rgb = clamp(mix(vec3(dot(outColor.rgb, greyscale)), outColor.rgb, saturation), 0.0, 1.0);
	//gamma correction
	outColor.rgb = clamp(pow(outColor.rgb, vec3(gammaCorrection)), 0.0, 1.0);
}
#FRAGMENT_END