#version 450
uint MATERIAL_FEATURE_COLOR = 1 << 0;
uint MATERIAL_FEATURE_NORMAL = 1 << 1;
uint MATERIAL_FEATURE_ROUGHNESS = 1 << 2;
uint MATERIAL_FEATURE_OCCLUSION = 1 << 3;
uint MATERIAL_FEATURE_EMISSIVE = 1 << 4;
uint MATERIAL_FEATURE_TANGENT_VERTEX = 1 << 5;
uint MATERIAL_FEATURE_TEXCOORD_VERTEX = 1 << 6;

layout(std140, binding = 0) uniform LocalConstants {
    mat4 m;
    mat4 vp;
    vec4 eye;
    vec4 light;
};

layout(std140, binding = 1) uniform MaterialConstant {
    vec4 baseColorFactor;
    mat4 model;
    mat4 modelInv;

    vec3  emissiveFactor;
    float metallicFactor;

    float roughnessFactor;
    float occlusionFactor;
    uint  flags;
};

layout (binding = 2) uniform sampler2D diffuseTexture;
layout (binding = 3) uniform sampler2D roughnessMetalnessTexture;
layout (binding = 4) uniform sampler2D occlusionTexture;
layout (binding = 5) uniform sampler2D emissiveTexture;
layout (binding = 6) uniform sampler2D normalTexture;

layout (location = 0) in vec2 texcoord0;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec4 position;

layout (location = 0) out vec4 fragColor;

#define PI 3.1415926538

// GGX/Trowbridge-Reitz Normal Distribution Function
float D(float alpha, vec3 N, vec3 H)
{
    float numerator = pow(alpha, 2.0);

    float NdotH = max(dot(N, H), 0.0);
    float denominator = PI * pow(pow(NdotH, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Schlick-Beckmann Geometry Shadowing Function
float G1(float alpha, vec3 N, vec3 X)
{
    float numerator = max(dot(N, X), 0.0);

    float k = alpha / 2.0;
    float denominator = max(dot(N, X), 0.0) * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Smith Model
float G(float alpha, vec3 N, vec3 V, vec3 L)
{
    return G1(alpha, N, V) * G1(alpha, N, L);
}

// Fresnel-Schlick Function
vec3 F(vec3 F0, vec3 V, vec3 H)
{
    return F0 + (vec3(1.0) - F0) * pow(1 - max(dot(V, H), 0.0), 5.0);
}

void main()
{
	mat3 TBN = mat3(1.0);
    vec3 N = normalize(normal);
	vec3 V = normalize(eye.xyz - position.xyz);
    //For Directional Lights: vec3 L = normalize(light.xyz);
    vec3 L = normalize(light.xyz - position.xyz);
	vec3 H = normalize(V + L);

    if ((flags & MATERIAL_FEATURE_TANGENT_VERTEX) != 0)
	{
        vec3 T = normalize(tangent.xyz);
        vec3 B = cross(N, T) * tangent.w;

        TBN = mat3(T, B, N);
    }
    else
	{
        // NOTE(marco): taken from https://community.khronos.org/t/computing-the-tangent-space-in-the-fragment-shader/52861
        vec3 Q1 = dFdx(position.xyz);
        vec3 Q2 = dFdy(position.xyz);
        vec2 st1 = dFdx(texcoord0);
        vec2 st2 = dFdy(texcoord0);

        vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
        vec3 B = normalize(-Q1 * st2.s + Q2 * st1.s);

        // the transpose of texture-to-eye space matrix
        TBN = mat3(T, B, N);
    }

	if ((flags & MATERIAL_FEATURE_NORMAL) != 0)
	{
        N = normalize(texture(normalTexture, texcoord0).rgb * 2.0 - 1.0);
        N = normalize(TBN * N);
    }

	float roughness = roughnessFactor;
    float metalness = metallicFactor;
	
	if ((flags & MATERIAL_FEATURE_ROUGHNESS) != 0)
	{
        // Red channel for occlusion value
        // Green channel contains roughness values
        // Blue channel contains metalness
        vec4 rm = texture(roughnessMetalnessTexture, texcoord0);

        roughness *= rm.g;
        metalness *= rm.b;
    }

	float ambientOcclussion = 1.0f;
    if ((flags & MATERIAL_FEATURE_OCCLUSION) != 0)
	{
        ambientOcclussion = texture(occlusionTexture, texcoord0).r;
    }

	float alpha = pow(roughness, 2.0);

	vec4 baseColor = baseColorFactor;
    if ((flags & MATERIAL_FEATURE_COLOR) != 0)
	{
        vec4 albedo = texture(diffuseTexture, texcoord0);
        baseColor.rgb *= albedo.rgb;
        baseColor.a *= albedo.a;
    }

    vec3 emissive = vec3( 0 );
    if ((flags & MATERIAL_FEATURE_EMISSIVE) != 0)
	{
        vec4 e = texture(emissiveTexture, texcoord0);

        emissive += e.rgb * emissiveFactor;
    }

	float NdotL = clamp( dot(N, L), 0, 1 );

	float F0 = 0.04; // pow( ( 1 - ior ) / ( 1 + ior ), 2 )
    float fr = F0 + ( 1 - F0 ) * pow(1 - abs( HdotV ), 5 );

    vec3 Ks = F(F0, V, H);
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallicFactor);

    vec3 lambert;

    if(flags & MATERIAL_FEATURE_COLOR) { lambert = texture(diffuseTexture, texcoord0).xyz / PI; }
    else { lambert = vec3(1.0, 1.0, 1.0) / PI; }
    
    vec3 cookTorranceNumerator = D(alpha, N, H) * G(alpha, N, V, L) * F(F0, V, H);
    float cookTorranceDenominator = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.000001);
    vec3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    vec3 BRDF = Kd * lambert + cookTorrance;
    // TODO: Add emissivity
    // TODO: Light Intensity and Color
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    fragColor = emissiveFactor + BRDF * lightColor * max(dot(L, N), 0.0);
}