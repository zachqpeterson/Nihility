#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require

#define DEBUG 1

struct Tilemap
{
	vec2 eye;
    vec2 tileSize;
    vec2 offset;
    uint width;
    uint height;
};

layout (set = 0, binding = 0) readonly buffer tilemapData
{
    Tilemap tilemaps[];
};

layout (set = 0, binding = 1) readonly buffer tileData
{
    uint16_t tiles[];
};

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];

layout (location = 0) in flat uint instance;
layout (location = 1) in flat uint tileOffset;

layout (location = 0) out vec4 fragColor;

const int INVALID_TEXTURE_INDEX = 65535;

void main()
{
    Tilemap tilemap = tilemaps[instance];

    vec2 position = vec2(gl_FragCoord.x + tilemap.eye.x - tilemap.offset.x, gl_FragCoord.y - tilemap.eye.y - tilemap.offset.y);
    ivec2 tilePos = ivec2(position / tilemap.tileSize);
    vec2 texcoord = fract(position / tilemap.tileSize);

    uint index = tilePos.x + tilePos.y * tilemap.width;

    if(position.x < 0 || position.y < 0 || tilePos.x >= tilemap.width || tilePos.y >= tilemap.height) { discard; }

#if DEBUG == 0
    if(texcoord.x < 0.02 || texcoord.y < 0.02 || texcoord.x > 0.98 || texcoord.y > 0.98) { fragColor = vec4(1.0, 0.0, 0.0, 1.0); return; }
#endif

    uint16_t tileType = tiles[index + tileOffset];

    if(tileType == INVALID_TEXTURE_INDEX) { discard; }

    fragColor = texture(globalTextures[nonuniformEXT(tileType)], texcoord);
}