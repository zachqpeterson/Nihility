#CONFIG
language=GLSL
#CONFIG_END

#COMPUTE
#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require

#define TASK_WGSIZE 64

#define TASK_WGLIMIT (1 << 22)

layout (constant_id = 0) const bool LATE = false;
layout (constant_id = 1) const bool TASK = false;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

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
	mat4		model;
	uint		meshIndex;
	uint 		vertexOffset;
	uint 		vertexCount;
	uint		meshletVisibilityOffset;

	uint	    diffuseTextureIndex;
	uint	    metalRoughOcclTextureIndex;
	uint	    normalTextureIndex;
	uint	    emissivityTextureIndex;

	vec4		baseColorFactor;
	vec2		metalRoughFactor;
	vec3		emissiveFactor;

	float		alphaCutoff;
	uint		flags;

	vec3 center;
	float radius;

	uint lodCount;
	MeshLod lods[8];
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

layout(push_constant) uniform block
{
	DrawCullData cullData;
};

layout(binding = 0) readonly buffer Meshes
{
	Mesh meshes[];
};

layout(binding = 1) writeonly buffer DrawCommands
{
	MeshDrawCommand drawCommands[];
};

layout(binding = 1) writeonly buffer TaskCommands
{
	MeshTaskCommand taskCommands[];
};

layout(binding = 2) buffer CommandCount
{
	uint commandCount;
};

layout(binding = 3) buffer DrawVisibility
{
	uint drawVisibility[];
};

layout(binding = 4) uniform sampler2D depthPyramid;

// 2D Polyhedral Bounds of a Clipped, Perspective-Projected 3D Sphere. Michael Mara, Morgan McGuire. 2013
bool ProjectSphere(vec3 c, float r, float znear, float P00, float P11, out vec4 aabb)
{
	if (c.z < r + znear)
		return false;

	vec3 cr = c * r;
	float czr2 = c.z * c.z - r * r;

	float vx = sqrt(c.x * c.x + czr2);
	float minx = (vx * c.x - cr.z) / (vx * c.z + cr.x);
	float maxx = (vx * c.x + cr.z) / (vx * c.z - cr.x);

	float vy = sqrt(c.y * c.y + czr2);
	float miny = (vy * c.y - cr.z) / (vy * c.z + cr.y);
	float maxy = (vy * c.y + cr.z) / (vy * c.z - cr.y);

	aabb = vec4(minx * P00, miny * P11, maxx * P00, maxy * P11);
	aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f); // clip space -> uv space

	return true;
}

bool ConeCull(vec3 center, float radius, vec3 coneAxis, float coneCutoff, vec3 cameraPosition)
{
	return dot(center - cameraPosition, coneAxis) >= coneCutoff * length(center - cameraPosition) + radius;
}

void main()
{
	uint id = gl_GlobalInvocationID.x;

	if (id >= cullData.drawCount) { return; }

	// TODO: when occlusion culling is off, can we make sure everything is processed with LATE=false?
	if (!LATE && drawVisibility[id] == 0) { return; }

	Mesh mesh = meshes[id];

	vec3 center = (mesh.model * vec4(mesh.center, 1.0)).xyz;
	float radius = mesh.radius; //TODO: multiply by scale

	bool visible = true;
	// the left/top/right/bottom plane culling utilizes frustum symmetry to cull against two planes at the same time
	visible = visible && center.z * cullData.frustum[1] - abs(center.x) * cullData.frustum[0] > -radius;
	visible = visible && center.z * cullData.frustum[3] - abs(center.y) * cullData.frustum[2] > -radius;
	// the near/far plane culling uses camera space Z directly
	visible = visible && center.z + radius > cullData.znear && center.z - radius < cullData.zfar;

	visible = visible || cullData.cullingEnabled == 0;

	if (LATE && visible && cullData.occlusionEnabled == 1)
	{
		vec4 aabb;
		if (ProjectSphere(center, radius, cullData.znear, cullData.P00, cullData.P11, aabb))
		{
			float width = (aabb.z - aabb.x) * cullData.pyramidWidth;
			float height = (aabb.w - aabb.y) * cullData.pyramidHeight;

			float level = floor(log2(max(width, height)));

			// Sampler is set up to do min reduction, so this computes the minimum depth of a 2x2 texel quad
			float depth = textureLod(depthPyramid, (aabb.xy + aabb.zw) * 0.5, level).x;
			float depthSphere = cullData.znear / (center.z - radius);

			visible = visible && depthSphere > depth;
		}
	}

	// when meshlet occlusion culling is enabled, we actually *do* need to append the draw command if vis[]==1 in LATE pass,
	// so that we can correctly render now-visible previously-invisible meshlets. we also pass drawvis[] along to task shader
	// so that it can *reject* clusters that we *did* draw in the first pass
	if (visible && (!LATE || cullData.clusterOcclusionEnabled == 1 || drawVisibility[id] == 0))
	{
		// lod distance i = base * pow(step, i)
		// i = log2(distance / base) / log2(step)
		float lodIndexF = log2(length(center) / cullData.lodBase) / log2(cullData.lodStep);
		uint lodIndex = min(uint(max(lodIndexF + 1, 0)), mesh.lodCount - 1);

		lodIndex = cullData.lodEnabled == 1 ? lodIndex : 0;

		MeshLod lod = mesh.lods[lodIndex];

		if (TASK)
		{
			uint taskGroups = (lod.meshletCount + TASK_WGSIZE - 1) / TASK_WGSIZE;
			uint dci = atomicAdd(commandCount, taskGroups);

			uint lateDrawVisibility = drawVisibility[id];
			uint meshletVisibilityOffset = mesh.meshletVisibilityOffset;

			// drop draw calls on overflow; this limits us to ~4M visible draws or ~32B visible triangles, whichever is larger
			if (dci + taskGroups <= TASK_WGLIMIT)
			{
				for (uint i = 0; i < taskGroups; ++i)
				{
					taskCommands[dci + i].drawId = id;
					taskCommands[dci + i].taskOffset = lod.meshletOffset + i * TASK_WGSIZE;
					taskCommands[dci + i].taskCount = min(TASK_WGSIZE, lod.meshletCount - i * TASK_WGSIZE);
					taskCommands[dci + i].lateDrawVisibility = lateDrawVisibility;
					taskCommands[dci + i].meshletVisibilityOffset = meshletVisibilityOffset + i * TASK_WGSIZE;
				}
			}
		}
		else
		{
			uint dci = atomicAdd(commandCount, 1);

			drawCommands[dci].drawId = id;
			drawCommands[dci].indexCount = lod.indexCount;
			drawCommands[dci].instanceCount = 1;
			drawCommands[dci].firstIndex = lod.indexOffset;
			drawCommands[dci].vertexOffset = mesh.vertexOffset;
			drawCommands[dci].firstInstance = 0;
		}
	}

	if (LATE)
		drawVisibility[id] = visible ? 1 : 0;
}
#COMPUTE_END