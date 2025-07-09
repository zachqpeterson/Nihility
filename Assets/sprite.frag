#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];

layout (location = 0) in vec2 texcoord;
layout (location = 1) in vec4 color;
layout (location = 2) flat in uint textureIndex;

layout (location = 0) out vec4 fragColor;

const uint INVALID_TEXTURE_INDEX = 65535;

void main()
{
    vec4 baseColor = color;

    if(textureIndex != INVALID_TEXTURE_INDEX)
    {
        baseColor = texture(globalTextures[nonuniformEXT(textureIndex)], texcoord) * color;
    }

    fragColor = baseColor;
}