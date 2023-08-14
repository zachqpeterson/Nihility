#define TASK_WGSIZE 64
#define MESH_WGSIZE 64

#define TASK_WGLIMIT (1 << 22)

struct Vertex
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 texcoord;
};

struct Meshlet
{
	// vec3 keeps Meshlet aligned to 16 bytes which is important because C++ has an alignas() directive
	vec3 center;
	float radius;
	int8_t cone_axis[3];
	int8_t cone_cutoff;

	uint dataOffset;
	uint8_t vertexCount;
	uint8_t triangleCount;
};

struct Globals
{
	mat4 viewProjection;
	vec4 eye;
	vec4 directionalLight;
	vec4 directionalLightColor;
	vec4 ambientLight;
	float lightIntensity;
	uint skyboxIndex;

	float screenWidth, screenHeight, znear, zfar; // symmetric projection parameters
	vec4 frustum; // data for left/right/top/bottom frustum planes

	float pyramidWidth, pyramidHeight; // depth pyramid size in texels
	int clusterOcclusionEnabled;
};

struct DrawCullData
{
	float P00, P11, znear, zfar; // symmetric projection parameters
	vec4 frustum; // data for left/right/top/bottom frustum planes
	float lodBase, lodStep; // lod distance i = base * pow(step, i)
	float pyramidWidth, pyramidHeight; // depth pyramid size in texels

	uint drawCount;

	int cullingEnabled;
	int lodEnabled;
	int occlusionEnabled;
	int clusterOcclusionEnabled;
};

struct MeshLod
{
	uint indexOffset;
	uint indexCount;
	uint meshletOffset;
	uint meshletCount;
};

struct Mesh
{
	vec3 center;
	float radius;

	uint vertexOffset;
	uint vertexCount;

	uint lodCount;
	MeshLod lods[8];
};

struct MeshDraw
{
	mat4		model;
	uint		meshIndex;
	uint		vertexOffset;
	uint		meshletVisibilityOffset;

	uint16_t	diffuseTextureIndex;
	uint16_t	metalRoughOcclTextureIndex;
	uint16_t	normalTextureIndex;
	uint16_t	emissivityTextureIndex;

	vec4		baseColorFactor;
	vec2		metalRoughFactor;
	vec3		emissiveFactor;

	float		alphaCutoff;
	uint		flags;
};

struct MeshDrawCommand
{
	uint drawId;

	// VkDrawIndexedIndirectCommand
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

struct MeshTaskCommand
{
	uint drawId;
	uint taskOffset;
	uint taskCount;
	uint lateDrawVisibility;
	uint meshletVisibilityOffset;
};

struct MeshTaskPayload
{
	uint drawId;
	uint meshletIndices[TASK_WGSIZE];
};