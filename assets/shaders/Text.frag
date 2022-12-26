#version 450

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec4 inArea;

layout(set = 1, binding = 0) uniform sampler2D texSample;

layout(location = 0) out vec4 outColor;

void main()
{
    if(gl_FragCoord.x < inArea.x || gl_FragCoord.y < inArea.y || gl_FragCoord.x > inArea.z || gl_FragCoord.y > inArea.w)
    {
        discard;
    }

    float thickness = 0.6;
    float softness = 0.0;

    float a = texture(texSample, inTexcoord).a;
    a = smoothstep(1.0 - thickness - softness, 1.0 - thickness + softness, a);
    outColor = vec4(inColor, a);
}