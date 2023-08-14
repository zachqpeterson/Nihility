#CONFIG
language=GLSL
cull=FRONT
front=CLOCKWISE
fill=SOLID
depth=LESS
blend=ADD
#CONFIG_END

#VERTEX
#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require

#extension GL_GOOGLE_include_directive: require

#extension GL_ARB_shader_draw_parameters: require

#include "mesh.h"
#include "math.h"

layout(push_constant) uniform block
{
	Globals globals;
};

layout(binding = 0) readonly buffer DrawCommands
{
	MeshDrawCommand drawCommands[];
};

layout(binding = 1) readonly buffer Draws
{
	MeshDraw draws[];
};

layout(binding = 2) readonly buffer Vertices
{
	Vertex vertices[];
};

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBitangent;
layout (location = 4) out vec2 outTexcoord;

void main()
{
    uint drawId = drawCommands[gl_DrawIDARB].drawId;
    MeshDraw meshDraw = draws[drawId];
    Vertex vertex = vertices[gl_VertexIndex];

    vec4 worldPosition = meshDraw.model * vec4(vertex.position, 1.0);
    gl_Position = globals.viewProjection * worldPosition;
    outPosition = worldPosition.xyz / worldPosition.w;
    outNormal = vertex.normal;
    outTexcoord = vertex.texCoord;
    outTangent = vertex.tangent;
    outBitangent = vertex.bitangent;
}
#VERTEX_END

#FRAGMENT
#version 450
#extension GL_EXT_nonuniform_qualifier : require

#include "mesh.h"
#include "math.h"

const uint MATERIAL_FLAG_ALPHA_MASK = 1 << 0;
const uint MATERIAL_FLAG_NO_TANGENTS = 1 << 1;
const uint MATERIAL_FLAG_NO_TEXURE_COORDS = 1 << 2;

layout(push_constant) uniform block
{
	Globals globals;
};

layout(binding = 0) readonly buffer DrawCommands
{
	MeshDrawCommand drawCommands[];
};

layout(binding = 1) readonly buffer Draws
{
	MeshDraw draws[];
};

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];
layout (set = 1, binding = 10) uniform samplerCube globalTexturesCubes[];

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 texcoord;

layout (location = 0) out vec4 fragColor;

//NOTE: F0 in the formula notation refers to the value derived from ior = 1.5, (index of refraction)
const float F0 = 0.04; //pow((1 - ior) / (1 + ior), 2)
const uint INVALID_TEXTURE_INDEX = 65535;

vec3 DecodeSRGB(vec3 c) 
{
    vec3 result;
    if (c.r <= 0.04045) { result.r = c.r / 12.92; }
    else { result.r = pow((c.r + 0.055) / 1.055, 2.4); }

    if (c.g <= 0.04045) { result.g = c.g / 12.92; }
    else { result.g = pow((c.g + 0.055) / 1.055, 2.4); }

    if (c.b <= 0.04045) { result.b = c.b / 12.92; }
    else { result.b = pow((c.b + 0.055) / 1.055, 2.4); }

    return clamp(result, 0.0, 1.0);
}

vec3 EncodeSRGB(vec3 c) 
{
    vec3 result;
    if (c.r <= 0.0031308) { result.r = c.r * 12.92; }
    else { result.r = 1.055 * pow(c.r, 1.0 / 2.4) - 0.055; }

    if (c.g <= 0.0031308) { result.g = c.g * 12.92; }
    else { result.g = 1.055 * pow(c.g, 1.0 / 2.4) - 0.055; }

    if (c.b <= 0.0031308) { result.b = c.b * 12.92; }
    else { result.b = 1.055 * pow(c.b, 1.0 / 2.4) - 0.055; }

    return clamp(result, 0.0, 1.0);
}

void main()
{
    uint drawId = drawCommands[gl_DrawIDARB].drawId;
    MeshDraw meshDraw = draws[drawId];

    vec4 baseColor = vec4(1.0);

    if(meshDraw.diffuseTextureIndex != INVALID_TEXTURE_INDEX)
    {
        baseColor = texture(globalTextures[nonuniformEXT(meshDraw.diffuseTextureIndex)], texcoord) * meshDraw.baseColorFactor;
        baseColor.rgb = DecodeSRGB(baseColor.rgb);
    }

    if ((flags & MATERIAL_FLAG_ALPHA_MASK) != 0 && baseColor.a < alphaCutoff) { discard; }

    vec3 I = normalize(position - globals.eye.xyz);
    vec3 R = reflect(I, normalize(normal));
    R.y = -R.y;
    vec3 environment = vec3(0.0);
    if(globals.skyboxIndex != INVALID_TEXTURE_INDEX) { environment = texture(globalTexturesCubes[globals.skyboxIndex], R).rgb; }

    vec3 N = normalize(normal);

    if (meshDraw.normalTextureIndex != INVALID_TEXTURE_INDEX)
    {
        vec3 bumpNormal = normalize(texture(globalTextures[nonuniformEXT(meshDraw.normalTextureIndex)], texcoord).rgb * 2.0 - 1.0);

        if((meshDraw.flags & MATERIAL_FLAG_NO_TANGENTS) == 0)
        {
            vec3 T = normalize(tangent);
	        vec3 B = normalize(bitangent);

            mat3 TBN = mat3(T, B, N);

            N = normalize(TBN * normalize(bumpNormal));
        }
    }

    //if(gl_FrontFacing == false)
	//{
    //    N *= -1.0;
	//	  T *= -1.0;
    //    B *= -1.0;
	//}

	vec3 V = normalize(globals.eye.xyz - position);
    vec3 lightDirection = globals.directionalLight.xyz;
    float lightDistance = length(lightDirection);
    vec3 L = normalize(lightDirection);
	vec3 H = normalize(V + L);

	float metallicness = meshDraw.metalRoughFactor.x;
    float roughness = meshDraw.metalRoughFactor.y;
    float occlusion = 0.0f;

	if (meshDraw.metalRoughOcclTextureIndex != INVALID_TEXTURE_INDEX)
	{
        vec4 rmo = texture(globalTextures[nonuniformEXT(meshDraw.metalRoughOcclTextureIndex)], texcoord);

		// Red channel contains occlusion values
        // Green channel contains roughness values
        // Blue channel contains metallicness
		occlusion = rmo.r;
        roughness *= rmo.g;
        metallicness *= rmo.b;
        environment = clamp(environment * (metallicness * (1.0 - roughness)), 0.0, 1.0);
    }

    vec3 emissivity = meshDraw.emissiveFactor.rgb;

    if(meshDraw.emissivityTextureIndex != INVALID_TEXTURE_INDEX)
    {
        emissivity += texture(globalTextures[nonuniformEXT(meshDraw.emissivityTextureIndex)], texcoord).rgb;
    }

	float alpha = roughness * roughness;
    float alphaSqr = alpha * alpha;
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float HdotL = clamp(dot(H, L), 0.0, 1.0);
    float HdotV = clamp(dot(H, V), 0.0, 1.0);
    
    vec3 materialColor = vec3(0.0, 0.0, 0.0);
    if (NdotL > 0.0 || NdotV > 0.0)
    {
        //float intensity = (lightIntensity * clamp(1.0 - pow(lightDistance / lightRange, 4), 0.0, 1.0) / (lightDistance * lightDistance)) * NdotL;
        float intensity = (globals.lightIntensity / (lightDistance * lightDistance)) * NdotL;

        float denominator = (abs(NdotL) + sqrt(alphaSqr + (1.0 - alphaSqr) * (NdotL * NdotL))) *
            (abs(NdotV) + sqrt(alphaSqr + (1.0 - alphaSqr) * (NdotV * NdotV)));
        float numerator = Heaviside(HdotL) * Heaviside(HdotV);

        float visibility = numerator / denominator;

        float distDen = (NdotH * NdotH) * (alphaSqr - 1.0) + 1.0;
        float distribution = (alphaSqr * Heaviside(NdotH)) / (PI * distDen * distDen);

        vec3 colorDifference = mix(baseColor.rgb, vec3(0.0), metallicness);
        vec3 f0 = mix(vec3(F0), baseColor.rgb, metallicness);

        vec3 F = f0 + (1.0 - f0) * pow(1.0 - abs(HdotV), 5);

        vec3 specularBRDF = F * visibility * distribution;
        vec3 diffuseBRDF = (1.0 - F) * RecPI * colorDifference;

        materialColor = (diffuseBRDF + specularBRDF) * intensity;
    }

    fragColor = vec4(emissivity + globals.ambientLight.xyz * baseColor.rgb + environment + EncodeSRGB(materialColor) * globals.directionalLightColor.xyz, baseColor.a);
}
#FRAGMENT_END