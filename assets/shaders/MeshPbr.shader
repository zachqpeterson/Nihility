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

#extension GL_ARB_shader_draw_parameters: require

struct Globals
{
	mat4 viewProjection;
	vec4 eye;

	float screenWidth, screenHeight, znear, zfar; // symmetric projection parameters
	vec4 frustum; // data for left/right/top/bottom frustum planes

	float pyramidWidth, pyramidHeight; // depth pyramid size in texels
	int clusterOcclusionEnabled;
};

struct MeshDrawCommand
{
	uint drawID;

	// VkDrawIndexedIndirectCommand
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

struct MeshLod
{
	uint indexOffset;
	uint indexCount;
	uint meshletOffset;
	uint meshletCount;
};

struct Mesh
{
	mat4	model;
	uint	meshIndex;
	uint 	vertexOffset;
	uint 	vertexCount;
	uint	meshletVisibilityOffset;

	uint	diffuseTextureIndex;
	uint	metalRoughOcclTextureIndex;
	uint	normalTextureIndex;
	uint	emissivityTextureIndex;

	vec4	baseColorFactor;
	vec2	metalRoughFactor;
	vec3	emissiveFactor;

	float	alphaCutoff;
	uint	flags;

	vec3    center;
	float   radius;

	uint    lodCount;
	MeshLod lods[8];
};

struct Vertex
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 texcoord;
};

layout(push_constant) uniform block
{
	Globals globals;
};

layout(binding = 0) readonly buffer DrawCommands
{
	MeshDrawCommand drawCommands[];
};

layout(binding = 1) readonly buffer Meshes
{
	Mesh meshes[];
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
layout (location = 5) flat out uint outDrawID;

void main()
{
    uint drawID = drawCommands[gl_DrawIDARB].drawID;
    Mesh mesh = meshes[drawID];
    Vertex vertex = vertices[gl_VertexIndex];

    vec4 worldPosition = mesh.model * vec4(vertex.position, 1.0);
    gl_Position = globals.viewProjection * worldPosition;
    outPosition = worldPosition.xyz / worldPosition.w;
    outNormal = vertex.normal;
    outTexcoord = vertex.texcoord;
    outTangent = vertex.tangent;
    outBitangent = vertex.bitangent;
    outDrawID = drawID;
}
#VERTEX_END

#FRAGMENT
#version 450
#extension GL_EXT_nonuniform_qualifier : require

const uint MATERIAL_FLAG_ALPHA_MASK = 1 << 0;
const uint MATERIAL_FLAG_NO_TANGENTS = 1 << 1;
const uint MATERIAL_FLAG_NO_TEXURE_COORDS = 1 << 2;

struct Globals
{
	mat4 viewProjection;
	vec4 eye;

	float screenWidth, screenHeight, znear, zfar; // symmetric projection parameters
	vec4 frustum; // data for left/right/top/bottom frustum planes

	float pyramidWidth, pyramidHeight; // depth pyramid size in texels
	int clusterOcclusionEnabled;
};

struct Mesh
{
	mat4		model;
	uint		meshIndex;
	uint		vertexOffset;
	uint		meshletVisibilityOffset;

	uint	    diffuseTextureIndex;
	uint	    metalRoughOcclTextureIndex;
	uint	    normalTextureIndex;
	uint	    emissivityTextureIndex;

	vec4		baseColorFactor;
	vec2		metalRoughFactor;
	vec3		emissiveFactor;

	float		alphaCutoff;
	uint		flags;
};

layout(push_constant) uniform block
{
	Globals globals;
};

layout(binding = 1) readonly buffer Meshes
{
	Mesh meshes[];
};

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];
layout (set = 1, binding = 10) uniform samplerCube globalTexturesCubes[];

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 texcoord;
layout (location = 5) flat in uint drawID;

layout (location = 0) out vec4 fragColor;

//NOTE: F0 in the formula notation refers to the value derived from ior = 1.5, (index of refraction)
const float F0 = 0.04; //pow((1 - ior) / (1 + ior), 2)
const float PI = 3.141592654;
const float RecPI = 1.0 / PI;
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

float Heaviside(float v)
{
	if (v > 0.0) { return 1.0; }
	else { return 0.0; }
}

void main()
{
    Mesh mesh = meshes[drawID];

    vec4 baseColor = vec4(1.0);

    if(mesh.diffuseTextureIndex != INVALID_TEXTURE_INDEX)
    {
        baseColor = texture(globalTextures[nonuniformEXT(mesh.diffuseTextureIndex)], texcoord) * mesh.baseColorFactor;
        baseColor.rgb = DecodeSRGB(baseColor.rgb);
    }

    if ((mesh.flags & MATERIAL_FLAG_ALPHA_MASK) != 0 && baseColor.a < mesh.alphaCutoff) { discard; }

    vec3 I = normalize(position - globals.eye.xyz);
    vec3 R = reflect(I, normalize(normal));
    R.y = -R.y;

    vec3 N = normalize(normal);

    if (mesh.normalTextureIndex != INVALID_TEXTURE_INDEX)
    {
        vec3 bumpNormal = normalize(texture(globalTextures[nonuniformEXT(mesh.normalTextureIndex)], texcoord).rgb * 2.0 - 1.0);

        if((mesh.flags & MATERIAL_FLAG_NO_TANGENTS) == 0)
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

    vec3 lightPos = vec3(0.0, 2.0, 1.0);

	vec3 V = normalize(globals.eye.xyz - position);
    vec3 lightDirection = position - lightPos;
    float lightDistance = length(lightDirection);
    vec3 L = normalize(lightDirection);
	vec3 H = normalize(V + L);

	float metallicness = mesh.metalRoughFactor.x;
    float roughness = mesh.metalRoughFactor.y;
    float occlusion = 0.0f;

	if (mesh.metalRoughOcclTextureIndex != INVALID_TEXTURE_INDEX)
	{
        vec4 rmo = texture(globalTextures[nonuniformEXT(mesh.metalRoughOcclTextureIndex)], texcoord);

		// Red channel contains occlusion values
        // Green channel contains roughness values
        // Blue channel contains metallicness
		occlusion = rmo.r;
        roughness *= rmo.g;
        metallicness *= rmo.b;
    }

    vec3 emissivity = mesh.emissiveFactor.rgb;

    if(mesh.emissivityTextureIndex != INVALID_TEXTURE_INDEX)
    {
        emissivity += texture(globalTextures[nonuniformEXT(mesh.emissivityTextureIndex)], texcoord).rgb;
    }

	float alpha = roughness * roughness;
    float alphaSqr = alpha * alpha;
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float HdotL = clamp(dot(H, L), 0.0, 1.0);
    float HdotV = clamp(dot(H, V), 0.0, 1.0);

    float lightIntensity = 10.0;
    float lightRange = 10.0;
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    vec3 materialColor = vec3(0.0, 0.0, 0.0);
    if (NdotL > 0.0 || NdotV > 0.0)
    {
        float intensity = (lightIntensity * clamp(1.0 - pow(lightDistance / lightRange, 4), 0.0, 1.0) / (lightDistance * lightDistance)) * NdotL;
        //float intensity = (globals.lightIntensity / (lightDistance * lightDistance)) * NdotL;

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

    vec3 ambientLight = vec3(0.2, 0.2, 0.2);

    fragColor = vec4(emissivity + ambientLight * baseColor.rgb + EncodeSRGB(materialColor) * lightColor, baseColor.a);
}
#FRAGMENT_END