#version 450

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inArea;

layout(set = 1, binding = 0) uniform sampler2D texSample;

layout(location = 0) out vec4 outColor;

void main()
{
    if(gl_FragCoord.x < inArea.x || gl_FragCoord.y < inArea.y || gl_FragCoord.x > inArea.z || gl_FragCoord.y > inArea.w)
    {
        discard;
    }

    outColor = texture(texSample, inTexcoord) * inColor;
}