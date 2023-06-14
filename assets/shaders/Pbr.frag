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
    vec3        metalRoughOcclFactor;
    vec3        emissiveFactor;
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

layout (location = 0) out vec4 fragColor0;
layout (location = 1) out vec4 fragColor1;

#define PI 3.1415926538
#define RecPI 1.0 / PI
//NOTE: F0 in the formula notation refers to the value derived from ior = 1.5, (index of refraction)
float F0 = 0.04; //pow((1 - ior) / (1 + ior), 2)
#define INVALID_TEXTURE_INDEX 65535

vec3 DecodeSRGB(vec3 c)
{
    vec3 result;
    if (c.r <= 0.04045) { result.r = c.r / 12.92; }
    else { result.r = pow( ( c.r + 0.055 ) / 1.055, 2.4 ); }

    if (c.g <= 0.04045) { result.g = c.g / 12.92; }
    else { result.g = pow( ( c.g + 0.055 ) / 1.055, 2.4 ); }

    if (c.b <= 0.04045) { result.b = c.b / 12.92; }
    else { result.b = pow( ( c.b + 0.055 ) / 1.055, 2.4 ); }

    return clamp(result, 0.0, 1.0);
}

vec3 EncodeSRGB(vec3 c)
{
    vec3 result;
    if (c.r <= 0.0031308) { result.r = c.r * 12.92; }
    else { result.r = 1.055 * pow( c.r, 1.0 / 2.4 ) - 0.055; }

    if (c.g <= 0.0031308) { result.g = c.g * 12.92; }
    else { result.g = 1.055 * pow( c.g, 1.0 / 2.4 ) - 0.055; }

    if (c.b <= 0.0031308) { result.b = c.b * 12.92; }
    else { result.b = 1.055 * pow( c.b, 1.0 / 2.4 ) - 0.055; }

    return clamp(result, 0.0, 1.0);
}

float Heaviside(float v) {
    if (v > 0.0) { return 1.0; }
    else { return 0.0; }
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
        vec3 bumpNormal = normalize(texture(globalTextures[nonuniformEXT(textures.z)], texcoord0).rgb * 2.0 - 1.0);
        mat3 TBN = mat3(T, B, N);

        N = normalize(TBN * normalize(bumpNormal));
    }

	vec3 V = normalize(eye.xyz - position);
    //For Directional Lights: vec3 L = normalize(light.xyz);
    vec3 L = normalize(light.xyz - position);
	vec3 H = normalize(V + L);

	float metallicness = metalRoughOcclFactor.x;
    float roughness = metalRoughOcclFactor.y;
    float occlusion = metalRoughOcclFactor.z;

	if (textures.y != INVALID_TEXTURE_INDEX)
	{
        vec4 rmo = texture(globalTextures[nonuniformEXT(textures.y)], texcoord0);

		// Red channel contains occlusion values
        // Green channel contains roughness values
        // Blue channel contains metallicness
		occlusion *= rmo.r;
        roughness *= rmo.g;
        metallicness *= rmo.b;
    }

    vec3 emissivity = emissiveFactor.rgb;

    if(textures.w != INVALID_TEXTURE_INDEX)
    {
        emissivity += texture(globalTextures[nonuniformEXT(textures.w)], texcoord0).rgb;
    }

    baseColor.rgb = DecodeSRGB(baseColor.rgb);

	float alpha = roughness * roughness;
    float alphaSqr = alpha * alpha;
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float HdotL = clamp(dot(H, L), 0.0, 1.0);
    float HdotV = clamp(dot(H, V), 0.0, 1.0);

    //TODO: pass in light color
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    vec3 materialColor = vec3(0.0, 0.0, 0.0);
    if (NdotL > 0.0 || NdotV > 0.0)
    {
        float distance = length(light.xyz - position);
        float intensity = (lightIntensity * clamp(1.0 - pow(distance / lightRange, 4), 0.0, 1.0) / (distance * distance)) * NdotL;

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

    vec4 color = vec4(clamp(emissivity + EncodeSRGB(materialColor) * lightColor, 0.0, 1.0), baseColor.a);

    // Color with manual exposure into attachment 0
    float exposure = 1.0;
	fragColor0.rgb = vec3(1.0) - exp(-(color.rgb + emissivity * 5.0f) * exposure);

	// Bright parts for bloom into attachment 1
	float l = dot(fragColor0.rgb, vec3(0.2126, 0.7152, 0.0722));
	float threshold = 0.75;
	fragColor1.rgb = (l > threshold) ? fragColor0.rgb : vec3(0.0);
	fragColor1.a = 1.0;

    fragColor0 = color;
}