#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require

#define DEBUG 1

layout(push_constant) uniform block
{
	vec2 eye;
    vec2 tileSize;
    uint width;
    uint height;
};

layout (set = 0, binding = 0) readonly buffer tilemap
{
    uint16_t tiles[];
};

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];

layout (location = 0) out vec4 fragColor;

const int INVALID_TEXTURE_INDEX = 65535;

void main()
{
    vec2 position = vec2(gl_FragCoord.x + eye.x, gl_FragCoord.y - eye.y);
    ivec2 tilePos = ivec2(uint(position.x * tileSize.x), uint(position.y * tileSize.y));
    vec2 texcoord = fract(position * tileSize);

    uint index = tilePos.x + tilePos.y * width;

    if(position.x < 0 || position.y < 0 || tilePos.x >= width || tilePos.y >= height) { discard; }

#if DEBUG == 1
    if(texcoord.x < 0.02 || texcoord.y < 0.02 || texcoord.x > 0.98 || texcoord.y > 0.98) { fragColor = vec4(1.0, 0.0, 0.0, 1.0); return; }
#endif

    uint16_t tileType = tiles[index];

    if(tileType == INVALID_TEXTURE_INDEX) { discard; }

    fragColor = texture(globalTextures[nonuniformEXT(tileType)], texcoord);
}