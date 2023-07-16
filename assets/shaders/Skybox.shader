#CONFIG
name=Skybox
language=GLSL
cull=NONE
front=CLOCKWISE
fill=SOLID
depth=LESS_EQUAL
blend=ADD
clear=CLEAR
#CONFIG_END

#VERTEX
#version 450

layout (location = 0) in vec3 position;

layout(binding = 0) uniform LocalConstants
{
    mat4 viewProjection;
};

layout (location = 0) out vec3 outTexCoord;

void main()
{
	outTexCoord = vec3(position.x, -position.y, position.z);
    vec4 pos = viewProjection * vec4(position, 1.0);
	gl_Position = pos.xyww;
}
#VERTEX_END

#FRAGMENT
#version 450

layout (location = 0) in vec3 texCoord;

layout (binding = 1) uniform samplerCube samplerEnvMap; //TODO: Use bindless

layout (location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(samplerEnvMap, texCoord);
}
#FRAGMENT_END