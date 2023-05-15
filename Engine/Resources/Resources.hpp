#pragma once

#include "ResourceDefines.hpp"

#include "ResourcePool.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Math\Math.hpp"

struct alignas(16) NH_API MaterialData
{
	Vector4 baseColorFactor;
	Matrix4 model;
	Matrix4 modelInv;

	Vector3 emissiveFactor;
	F32   metallicFactor;

	F32   roughnessFactor;
	F32   occlusionFactor;
	U32   flags;
};

//TODO: Move these to a scene class
struct NH_API Scene
{
	I32* nodes;
	U32 nodesCount;
};

struct NH_API Texture
{
	I32		sampler;
	I32		source;
	String	name;
};

struct NH_API Accessor
{
	enum ComponentType
	{
		BYTE = 5120, UNSIGNED_BYTE = 5121, SHORT = 5122, UNSIGNED_SHORT = 5123, UNSIGNED_INT = 5125, FLOAT = 5126
	};

	enum Type
	{
		Scalar, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4
	};

	I32		bufferView;
	I32		byteOffset;

	I32		componentType;
	I32		count;
	U32		maxCount;
	F32* max;
	U32		minCount;
	F32* min;
	bool	normalized;
	I32		sparse;
	Type	type;
};

struct NH_API MeshPrimitive
{
	struct Attribute
	{
		String	key;
		I32		accessorIndex;
	};

	Attribute* attributes;
	U32			attributeCount;
	I32			indices;
	I32			material;
	I32			mode;
	// 0 POINTS
	// 1 LINES
	// 2 LINE_LOOP
	// 3 LINE_STRIP
	// 4 TRIANGLES
	// 5 TRIANGLE_STRIP
	// 6 TRIANGLE_FAN
};

struct NH_API Mesh
{
	MeshPrimitive* primitives;
	U32				primitivesCount;
	U32				weightsCount;
	F32* weights;
	String			name;
};

struct BufferView
{
	enum Target {
		ARRAY_BUFFER = 34962 /* Vertex Data */, ELEMENT_ARRAY_BUFFER = 34963 /* Index Data */
	};

	I32                         buffer;
	I32                         byteLength;
	I32                         byteOffset;
	I32                         byteStride;
	I32                         target;
	String						name;
};

struct TextureInfo
{
	I32                         index;
	I32                         texCoord;
};

struct MaterialPBRMetallicRoughness
{
	U32                         baseColorFactorCount;
	F32* baseColorFactor;
	TextureInfo* baseColorTexture;
	F32                         metallicFactor;
	TextureInfo* metallicRoughnessTexture;
	F32                         roughnessFactor;
};

struct MaterialNormalTextureInfo
{
	I32                         index;
	I32                         texCoord;
	F32                         scale;
};

struct MaterialOcclusionTextureInfo
{
	I32                         index;
	I32                         texCoord;
	F32                         strength;
};

struct Material
{
	F32                         alpha_cutoff;
	// OPAQUE The alpha value is ignored, and the rendered output is fully opaque.
	// MASK The rendered output is either fully opaque or fully transparent depending on the alpha value and the specified `alphaCutoff` value; the exact appearance of the edges **MAY** be subject to implementation-specific techniques such as "`Alpha-to-Coverage`".
	// BLEND The alpha value is used to composite the source and destination areas. The rendered output is combined with the background using the normal painting operation (i.e. the Porter and Duff over operator).
	String                alphaMode;
	bool                        doubleSided;
	U32                         emissiveFactorCount;
	F32* emissiveFactor;
	TextureInfo* emissiveTexture;
	MaterialNormalTextureInfo* normalTexture;
	MaterialOcclusionTextureInfo* occlusionTexture;
	MaterialPBRMetallicRoughness* pbrMetallicRoughness;
	String                name;
};

struct Camera
{
	I32                         orthographic;
	I32                         perspective;
	// perspective
	// orthographic
	String                type;
};

struct AnimationChannel
{

	enum TargetType
	{
		Translation, Rotation, Scale, Weights, Count
	};

	I32                         sampler;
	I32                         targetNode;
	TargetType                  targetType;
};

struct AnimationSampler
{
	I32                         inputKeyframeBufferIndex;  //"The index of an accessor containing keyframe timestamps."
	I32                         outputKeyframeBufferIndex; // "The index of an accessor, containing keyframe output values."

	enum Interpolation
	{
		Linear, Step, CubicSpline, Count
	};
	// LINEAR The animated values are linearly interpolated between keyframes. When targeting a rotation, spherical linear interpolation (slerp) **SHOULD** be used to interpolate quaternions. The float of output elements **MUST** equal the float of input elements.
	// STEP The animated values remain constant to the output of the first keyframe, until the next keyframe. The float of output elements **MUST** equal the float of input elements.
	// CUBICSPLINE The animation's interpolation is computed using a cubic spline with specified tangents. The float of output elements **MUST** equal three times the float of input elements. For each input element, the output stores three elements, an in-tangent, a spline vertex, and an out-tangent. There **MUST** be at least two keyframes when using this interpolation.
	Interpolation               interpolation;
};

struct Skin
{
	I32                         inverse_bind_matrices_buffer_index;
	I32                         skeleton_root_node_index;
	U32                         joints_count;
	I32* joints;
};

struct Animation
{
	U32					channels_count;
	AnimationChannel* channels;
	U32					samplers_count;
	AnimationSampler* samplers;
};

struct Image
{
	I32                         buffer_view;
	// image/jpeg
	// image/png
	String                mime_type;
	String                uri;
};

struct Node
{
	I32                         camera;
	U32                         children_count;
	I32* children;
	U32                         matrix_count;
	F32* matrix;
	I32                         mesh;
	U32                         rotation_count;
	F32* rotation;
	U32                         scale_count;
	F32* scale;
	I32                         skin;
	U32                         translation_count;
	F32* translation;
	U32                         weights_count;
	F32* weights;
	String                name;
};

struct Asset
{
	String                         copyright;
	String                         generator;
	String                         minVersion;
	String                         version;
};

struct SamplerScene
{
	enum Filter
	{
		NEAREST = 9728, LINEAR = 9729, NEAREST_MIPMAP_NEAREST = 9984, LINEAR_MIPMAP_NEAREST = 9985, NEAREST_MIPMAP_LINEAR = 9986, LINEAR_MIPMAP_LINEAR = 9987
	};

	enum Wrap
	{
		CLAMP_TO_EDGE = 33071, MIRRORED_REPEAT = 33648, REPEAT = 10497
	};

	I32                         magFilter;
	I32                         minFilter;
	I32                         wrapS;
	I32                         wrapT;
};

struct glTF
{
	Asset asset;
	Accessor* accessors;
	U32 accessorsCount;
	Animation* animations;
	U32 animationsCount;
	BufferView* bufferViews;
	U32 bufferViewsCount;
	Buffer* buffers;
	U32 buffersCount;
	Camera* cameras;
	U32 camerasCount;
	String* extensionsRequired;
	U32 extensionsRequiredCount;
	String* extensionsUsed;
	U32 extensionsUsedCount;
	Image* images;
	U32 imagesCount;
	Material* materials;
	U32 materialsCount;
	Mesh* meshes;
	U32 meshesCount;
	Node* nodes;
	U32 nodesCount;
	SamplerScene* samplers;
	U32 samplersCount;
	Scene* scenes;
	U32 scenesCount;
	I32 scene;
	Skin* skins;
	U32 skinsCount;
	Texture* textures;
	U32 texturesCount;
};

struct NH_API MeshDraw
{
	BufferHandle indexBuffer;
	BufferHandle positionBuffer;
	BufferHandle tangentBuffer;
	BufferHandle normalBuffer;
	BufferHandle texcoordBuffer;

	BufferHandle materialBuffer;
	MaterialData materialData;

	U32 indexOffset;
	U32 positionOffset;
	U32 tangentOffset;
	U32 normalOffset;
	U32 texcoordOffset;

	U32 count;

	VkIndexType indexType;

	DescriptorSetHandle descriptorSet;
};

class NH_API Resources
{
public:
	static Texture* LoadTexture(String& name);



	static bool LoadBinary(const String& name, String& result);

private:
	static bool Initialize();
	static void Shutdown();

	//Texture Loading
	static bool LoadBMP();
	static bool LoadPNG();
	static bool LoadJPG();
	static bool LoadPSD();
	static bool LoadTIFF();
	static bool LoadTGA();

	NH_HEADER_STATIC Hashmap<String, Texture>				textures{ 512, {} };
	NH_HEADER_STATIC ResourcePool<Buffer, 4096>				buffers;
	NH_HEADER_STATIC ResourcePool<Pipeline, 128>			pipelines;
	NH_HEADER_STATIC ResourcePool<Sampler, 32>				samplers;
	NH_HEADER_STATIC ResourcePool<DesciptorSetLayout, 128>	descriptorSetLayouts;
	NH_HEADER_STATIC ResourcePool<DesciptorSet, 256>		descriptorSets;
	NH_HEADER_STATIC ResourcePool<RenderPass, 256>			renderPasses;
	NH_HEADER_STATIC ResourcePool<ShaderState, 128>			shaders;

	STATIC_CLASS(Resources);
	friend class Engine;
	friend class Renderer;
};