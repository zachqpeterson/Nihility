#version 450

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outColor;

void main(void)
{
    float filterRadius = 0.005; //TODO: Pass in

	// The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(samplerColor, vec2(inUV.x - x, inUV.y + y)).rgb;
    vec3 b = texture(samplerColor, vec2(inUV.x,     inUV.y + y)).rgb;
    vec3 c = texture(samplerColor, vec2(inUV.x + x, inUV.y + y)).rgb;

    vec3 d = texture(samplerColor, vec2(inUV.x - x, inUV.y)).rgb;
    vec3 e = texture(samplerColor, vec2(inUV.x,     inUV.y)).rgb;
    vec3 f = texture(samplerColor, vec2(inUV.x + x, inUV.y)).rgb;

    vec3 g = texture(samplerColor, vec2(inUV.x - x, inUV.y - y)).rgb;
    vec3 h = texture(samplerColor, vec2(inUV.x,     inUV.y - y)).rgb;
    vec3 i = texture(samplerColor, vec2(inUV.x + x, inUV.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |

    vec3 color = e * 4.0;
    color += (b + d + f + h) * 2.0;
    color += (a + c + g + i) ;
    color *= 1.0 / 16.0;

    outColor = vec4(color, 1.0);
}