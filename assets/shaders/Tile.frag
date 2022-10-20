#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inColor;
flat layout(location = 2) in uint inTexId;

layout(set = 0, binding = 1) uniform sampler2D texSamples[];

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 texSize = vec2(24.0, 24.0).xy / textureSize(texSamples[inTexId], 0).xy;
    vec2 texCoord = inTexcoord.xy * texSize.xy;

    outColor = texture(texSamples[inTexId], texCoord) * vec4(inColor, 1.0);
}