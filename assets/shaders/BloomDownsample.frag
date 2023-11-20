#version 450

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outColor;

void main(void)
{
	ivec2 textureSize = textureSize(samplerColor, 0);
	vec2 srcTexelSize = 1.0 / textureSize;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

	float x2 = x * 2;
	float y2 = y * 2;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(samplerColor, vec2(inUV.x - x2, 	inUV.y + y2)).rgb;
    vec3 b = texture(samplerColor, vec2(inUV.x,  		inUV.y + y2)).rgb;
    vec3 c = texture(samplerColor, vec2(inUV.x + x2, 	inUV.y + y2)).rgb;

    vec3 d = texture(samplerColor, vec2(inUV.x - x2, 	inUV.y)).rgb;
    vec3 e = texture(samplerColor, vec2(inUV.x,  		inUV.y)).rgb;
    vec3 f = texture(samplerColor, vec2(inUV.x + x2, 	inUV.y)).rgb;

    vec3 g = texture(samplerColor, vec2(inUV.x - x2, 	inUV.y - y2)).rgb;
    vec3 h = texture(samplerColor, vec2(inUV.x,  		inUV.y - y2)).rgb;
    vec3 i = texture(samplerColor, vec2(inUV.x + x2, 	inUV.y - y2)).rgb;

    vec3 j = texture(samplerColor, vec2(inUV.x - x, 	inUV.y + y)).rgb;
    vec3 k = texture(samplerColor, vec2(inUV.x + x, 	inUV.y + y)).rgb;
    vec3 l = texture(samplerColor, vec2(inUV.x - x, 	inUV.y - y)).rgb;
    vec3 m = texture(samplerColor, vec2(inUV.x + x, 	inUV.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    vec3 color =  e * 0.125;
    color += (a + c + g + i) * 0.03125;
    color += (b + d + f + h) * 0.0625;
    color += (j + k + l + m) * 0.125;

    outColor = max(vec4(color, 1.0), 0.0001);
}