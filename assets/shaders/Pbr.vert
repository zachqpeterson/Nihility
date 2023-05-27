#version 450

layout(std140, binding = 0) uniform LocalConstants
{
    mat4 viewProjection;
    vec4 eye;
    vec4 light;
    float lightRange;
    float lightIntensity;
};

layout(std140, binding = 1) uniform MaterialConstant
{
    mat4 model;
    mat4 modelInv;

    // x = diffuse index, y = roughness index, z = normal index, w = occlusion index.
    uvec4       textures;
    vec4        baseColorFactor;
    vec4        metalRoughOcclFactor;
    float       alphaCutoff;
    uint        flags;
};

layout(location=0) in vec3 position;
layout(location=1) in vec4 tangent;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texCoord0;

layout (location = 0) out vec2 outTexcoord0;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBiTangent;
layout (location = 4) out vec3 outPosition;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0);
    gl_Position = viewProjection * worldPosition;
    outPosition = worldPosition.xyz / worldPosition.w;
    outTexcoord0 = texCoord0;
    outNormal = normalize(mat3(modelInv) * normal);
    outTangent = normalize(mat3(model) * tangent.xyz);
    outBiTangent = cross(outNormal, outTangent ) * tangent.w;
}