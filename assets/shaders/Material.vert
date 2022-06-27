#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inTangent;

layout(set = 0, binding = 0) uniform globalUniformObject 
{
    mat4 projection;
	mat4 view;
	vec4 ambientColor;
	vec3 viewPosition;
	int mode;
} globalUbo;

layout(push_constant) uniform pushConstants
{
	mat4 model;
} push;

layout(location = 0) out int outMode;

layout(location = 1) out struct dto
{
	vec4 ambient;
	vec2 texCoord;
	vec3 normal;
	vec3 viewPosition;
	vec3 fragPosition;
	vec4 color;
	vec4 tangent;
} outDto;

void main() {
	outDto.texCoord = inTexcoord;
	outDto.color = inColor;
    
	outDto.fragPosition = vec3(push.model * vec4(inPosition, 1.0));
    
	mat3 m3Model = mat3(push.model);
	outDto.normal = normalize(m3Model * inNormal);
	outDto.tangent = vec4(normalize(m3Model * inTangent.xyz), inTangent.w);
	outDto.ambient = globalUbo.ambientColor;
	outDto.viewPosition = globalUbo.viewPosition;

    gl_Position = globalUbo.projection * globalUbo.view * push.model * vec4(inPosition, 1.0);

	outMode = globalUbo.mode;
}
