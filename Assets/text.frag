#version 450
#extension GL_EXT_nonuniform_qualifier : require

const uint INVALID_TEXTURE_INDEX = 65535;

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];

layout (location = 0) in vec2 texcoord;
layout (location = 1) in vec4 fgColor;
layout (location = 2) in float scale;
layout (location = 3) in flat uint textureIndex;

layout (location = 0) out vec4 outColor;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    float boldness = 0.5; //TODO: Input

    float s = scale * 0.4;
    vec4 samp = texture(globalTextures[textureIndex], texcoord);
    float dist = (median(samp.r, samp.g, samp.b) - 0.5) * s;
    float alpha = clamp(dist + 0.5, 0.0, 1.0);
    outColor = vec4(fgColor.rgb, fgColor.a * alpha);
}