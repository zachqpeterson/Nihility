#version 450
#extension GL_EXT_nonuniform_qualifier : enable

uint DrawFlags_AlphaMask = 1 << 0;

layout(std140, binding = 0) uniform LocalConstants
{
    mat4  viewProjection;
    vec4  eye;
    vec4  light;
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

layout (set = 1, binding = 10) uniform sampler2D globalTextures[];
layout (set = 1, binding = 10) uniform sampler3D globalTextures3D[];

layout (location = 0) in vec2 texcoord0;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec3 position;

layout (location = 0) out vec4 fragColor;

#define PI 3.1415926538
#define INVALID_TEXTURE_INDEX 65535

// GGX/Trowbridge-Reitz Normal Distribution Function
float D(float alpha, vec3 N, vec3 H)
{
    float numerator = pow(alpha, 2.0);

    float NdotH = max(dot(N, H), 0.0);
    float denominator = PI * pow(pow(NdotH, 2.0) * (numerator - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Schlick-Beckmann Geometry Shadowing Function
float G1(float alpha, float numerator)
{
    float k = alpha / 2.0;
    float denominator = max(numerator * (1.0 - k) + k, 0.000001);

    return numerator / denominator;
}

// Smith Model
float G(float alpha, float numerator0, float numerator1)
{
    return G1(alpha, numerator0) * G1(alpha, numerator1);
}

// Fresnel-Schlick Function
float F(float F0, vec3 V, vec3 H)
{
    return F0 + (1.0 - F0) * pow(1.0 - max(dot(V, H), 0.0), 5.0);
}

void main()
{
    vec4 baseColor = texture(globalTextures[nonuniformEXT(textures.x)], texcoord0) * baseColorFactor;

    if ((flags & DrawFlags_AlphaMask) != 0 && baseColor.a < alphaCutoff)
    {
		discard; //TODO: May not want to discard
    }

    vec3 N = normalize(normal);
	vec3 T = normalize(tangent);
	vec3 B = normalize(bitangent);

	if(gl_FrontFacing == false)
	{
        N *= -1.0;
		T *= -1.0;
        B *= -1.0;
	}

	if (textures.z != INVALID_TEXTURE_INDEX)
	{
        //normal textures are encoded to [0, 1] but need to be mapped to [-1, 1] value
        vec3 bumpNormal = normalize( texture(globalTextures[nonuniformEXT(textures.z)], texcoord0).rgb * 2.0 - 1.0 );
        mat3 TBN = mat3(T, B, N);

        N = normalize(TBN * normalize(bumpNormal));
    }

	vec3 V = normalize(eye.xyz - position);
    //For Directional Lights: vec3 L = normalize(light.xyz);
    vec3 L = normalize(light.xyz - position);
	vec3 H = normalize(V + L);

	float metalness = metalRoughOcclFactor.x;
    float roughness = metalRoughOcclFactor.y;
    float occlusion = metalRoughOcclFactor.z;

	if (textures.w != INVALID_TEXTURE_INDEX)
	{
        vec4 rmo = texture(globalTextures[nonuniformEXT(textures.y)], texcoord0);

        // Green channel contains roughness values
        // Blue channel contains metalness
		// Red channel for occlusion value
		occlusion *= rmo.r;
        roughness *= rmo.g;
        metalness *= rmo.b;
    }

	float alpha = pow(roughness, 2.0);

	float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

	float distance = length(light.xyz - position);
    float intensity = lightIntensity * max(min(1.0 - pow(distance / lightRange, 4.0), 1.0), 0.0) / pow(distance, 2.0);

	float F0 = 0.04; // pow( ( 1 - ior ) / ( 1 + ior ), 2 ), where ior == 1.5

    float Ks = F(F0, V, H);
    float Kd = (1.0 - Ks) * (1.0 - metalness);

    vec3 lambert = baseColor.xyz / PI;
    
    float cookTorranceNumerator = D(alpha, N, H) * G(alpha, NdotV, NdotL) * Ks;
    float cookTorranceDenominator = max(4.0 * NdotV * NdotL, 0.000001);
    float cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    vec3 BRDF = lambert * Kd + cookTorrance;
    // TODO: Light Color
    vec3 lightColor = vec3(1.0, 1.0, 1.0) * intensity;

	//TODO: Emissivity
    fragColor = vec4(BRDF * lightColor * NdotL, 1.0);
}