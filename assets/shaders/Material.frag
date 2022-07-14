#version 450

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform localUniformObject 
{
    vec4 diffuseColor;
    float shininess;
} objectUbo;

struct directionalLight 
{
    vec3 direction;
    vec4 color;
};

struct pointLight 
{
    vec3 position;
    vec4 color;
    float constant;
    float linear;
    float quadratic;
};

// TODO: feed in from cpu
directionalLight dirLight = {
    vec3(-0.57735, -0.57735, -0.57735),
    //vec4(0.6, 0.6, 0.6, 1.0)
    vec4(0.4, 0.4, 0.2, 1.0)
};

// TODO: feed in from cpu
pointLight pLight0 = {
    vec3(-5.5, 0.0, -5.5),
    vec4(0.0, 1.0, 0.0, 1.0),
    1.0,
    0.35,
    0.44
};

// TODO: feed in from cpu
pointLight pLight1 = {
    vec3(5.5, 0.0, -5.5),
    vec4(1.0, 0.0, 0.0, 1.0),
    1.0,
    0.35,
    0.44
};

const int SAMP_DIFFUSE = 0;
const int SAMP_SPECULAR = 1;
const int SAMP_NORMAL = 2;
layout(set = 0, binding = 1) uniform sampler2D samplers[3];

layout(location = 0) flat in int inMode;

layout(location = 1) in struct dto 
{
    vec4 ambient;
	vec2 texCoord;
	vec3 normal;
	vec3 viewPosition;
	vec3 fragPosition;
    vec4 color;
	vec4 tangent;
} inDto;

mat3 TBN;

vec4 calculateDirectionalLight(directionalLight light, vec3 normal, vec3 viewDirection);
vec4 calculatePointLight(pointLight light, vec3 normal, vec3 fragPosition, vec3 viewDirection);

void main() 
{
    vec3 normal = inDto.normal;
    vec3 tangent = inDto.tangent.xyz;
    tangent = (tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(inDto.normal, inDto.tangent.xyz) * inDto.tangent.w;
    TBN = mat3(tangent, bitangent, normal);

    vec3 localNormal = 2.0 * texture(samplers[SAMP_NORMAL], inDto.texCoord).rgb - 1.0;
    normal = normalize(TBN * localNormal);

    vec3 viewDirection = normalize(inDto.viewPosition - inDto.fragPosition);

    outColor = calculateDirectionalLight(dirLight, normal, viewDirection);

    outColor += calculatePointLight(pLight0, normal, inDto.fragPosition, viewDirection);
    outColor += calculatePointLight(pLight1, normal, inDto.fragPosition, viewDirection);
}

vec4 calculateDirectionalLight(directionalLight light, vec3 normal, vec3 viewDirection) 
{
    float diffuseFactor = max(dot(normal, -light.direction), 0.0);

    vec3 halfDirection = normalize(viewDirection - light.direction);
    float specularFactor = pow(max(dot(halfDirection, normal), 0.0), objectUbo.shininess);

    vec4 diffSamp = texture(samplers[SAMP_DIFFUSE], inDto.texCoord);
    vec4 ambient = vec4(vec3(inDto.ambient * objectUbo.diffuseColor), diffSamp.a);
    vec4 diffuse = vec4(vec3(light.color * diffuseFactor), diffSamp.a);
    vec4 specular = vec4(vec3(light.color * specularFactor), diffSamp.a);
    
    diffuse *= diffSamp;
    ambient *= diffSamp;
    specular *= vec4(texture(samplers[SAMP_SPECULAR], inDto.texCoord).rgb, diffuse.a);

    return (ambient + diffuse + specular);
}

vec4 calculatePointLight(pointLight light, vec3 normal, vec3 fragPosition, vec3 viewDirection) 
{
    vec3 lightDirection =  normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), objectUbo.shininess);

    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec4 ambient = inDto.ambient;
    vec4 diffuse = light.color * diff;
    vec4 specular = light.color * spec;
    
    vec4 diffSamp = texture(samplers[SAMP_DIFFUSE], inDto.texCoord);
    diffuse *= diffSamp;
    ambient *= diffSamp;
    specular *= vec4(texture(samplers[SAMP_SPECULAR], inDto.texCoord).rgb, diffuse.a);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}