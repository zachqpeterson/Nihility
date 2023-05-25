#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct NH_API Asset
{
	String copyright{ NO_INIT };
	String generator{ NO_INIT };
	String minVersion{ NO_INIT };
	String version{ NO_INIT };
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

	I32 bufferView;
	I32 byteOffset;

	I32 componentType;
	I32 count;
	U32 maxCount;
	F32* max;
	U32 minCount;
	F32* min;
	bool normalized;
	I32 sparse;
	Type type;
};

struct NH_API AnimationChannel
{
	enum TargetType
	{
		Translation, Rotation, Scale, Weights, Count
	};

	I32 sampler;
	I32 targetNode;
	TargetType targetType;
};

struct NH_API AnimationSampler
{
	I32 inputKeyframeBufferIndex;  //"The index of an accessor containing keyframe timestamps."
	I32 outputKeyframeBufferIndex; // "The index of an accessor, containing keyframe output values."

	enum Interpolation
	{
		Linear, Step, CubicSpline, Count
	};
	// LINEAR The animated values are linearly interpolated between keyframes. When targeting a rotation, spherical linear interpolation (slerp) **SHOULD** be used to interpolate quaternions. The float of output elements **MUST** equal the float of input elements.
	// STEP The animated values remain constant to the output of the first keyframe, until the next keyframe. The float of output elements **MUST** equal the float of input elements.
	// CUBICSPLINE The animation's interpolation is computed using a cubic spline with specified tangents. The float of output elements **MUST** equal three times the float of input elements. For each input element, the output stores three elements, an in-tangent, a spline vertex, and an out-tangent. There **MUST** be at least two keyframes when using this interpolation.
	Interpolation interpolation;
};

struct NH_API Animation
{
	U32 channelCount;
	AnimationChannel* channels;
	U32	samplerCount;
	AnimationSampler* samplers;
};

struct NH_API BufferView
{
	enum Target {
		ARRAY_BUFFER = 34962 /* Vertex Data */, ELEMENT_ARRAY_BUFFER = 34963 /* Index Data */
	};

	I32 buffer;
	I32 byteLength;
	I32 byteOffset;
	I32 byteStride;
	I32 target;
	String name{ NO_INIT };
};

struct NH_API BufferRef
{
	I32 byteLength;
	String uri{ NO_INIT };
	String name{ NO_INIT };
};

struct NH_API Camera
{
	I32 orthographic;
	I32 perspective;
	// perspective
	// orthographic
	String type{ NO_INIT };
};

struct NH_API Image
{
	I32 bufferView;
	// image/jpeg
	// image/png
	String mimeType{ NO_INIT };
	String uri{ NO_INIT };
};

struct NH_API TextureInfo
{
	I32 index;
	I32 texCoord;
};

struct NH_API MaterialPBRMetallicRoughness
{
	Vector4 baseColorFactor;
	TextureInfo baseColorTexture;
	F32 metallicFactor;
	TextureInfo metallicRoughnessTexture;
	F32 roughnessFactor;
};

struct NH_API MaterialNormalTextureInfo
{
	I32 index;
	I32 texCoord;
	F32 scale;
};

struct NH_API MaterialOcclusionTextureInfo
{
	I32 index;
	I32 texCoord;
	F32 strength;
};

enum MaterialFeatures {
	MaterialFeatures_ColorTexture = 1 << 0,
	MaterialFeatures_NormalTexture = 1 << 1,
	MaterialFeatures_RoughnessTexture = 1 << 2,
	MaterialFeatures_OcclusionTexture = 1 << 3,
	MaterialFeatures_EmissiveTexture = 1 << 4,

	MaterialFeatures_TangentVertexAttribute = 1 << 5,
	MaterialFeatures_TexcoordVertexAttribute = 1 << 6,
};

struct NH_API Material
{
	F32 alphaCutoff;
	String alphaMode{ NO_INIT };
	bool doubleSided;
	U32 emissiveFactorCount;
	Vector3 emissiveFactor;
	TextureInfo emissiveTexture;
	MaterialNormalTextureInfo normalTexture;
	MaterialOcclusionTextureInfo occlusionTexture;
	MaterialPBRMetallicRoughness pbrMetallicRoughness;
	String name{ NO_INIT };
};

struct UniformData
{
	Matrix4 m;
	Matrix4 vp;
	Vector4 eye;
	Vector4 light;
};

struct NH_API MeshPrimitive
{
	struct Attribute
	{
		String key{ NO_INIT };
		I32 accessorIndex;
	};

	Attribute* attributes;
	U32 attributeCount;
	I32 indices;
	I32 material;
	I32 mode;
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
	U32 primitiveCount;
	MeshPrimitive* primitives;
	U32 weightCount;
	F32* weights;
	String name{ NO_INIT };
};

struct NH_API Node
{
	I32 camera;
	U32 childrenCount;
	I32* children;
	U32 matrixCount;
	F32* matrix;
	I32 mesh;
	U32 rotationCount;
	F32* rotation;
	U32 scaleCount;
	F32* scale;
	I32 skin;
	U32 translationCount;
	F32* translation;
	U32 weightCount;
	F32* weights;
	String name{ NO_INIT };
};

struct NH_API SamplerRef
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

//struct NH_API Scene
//{
//	U32 nodeCount;
//	I32* nodes;
//};

struct NH_API Skin
{
	I32 inverseBindMatricesBufferIndex;
	I32 skeletonRootNodeIndex;
	U32 jointCount;
	I32* joints;
};

struct NH_API TextureRef
{
	I32 sampler;
	I32 source;
	String name{ NO_INIT };
};

struct NH_API glTF
{
	Asset asset;
	U32 accessorCount;
	Accessor* accessors;
	U32 animationCount;
	Animation* animations;
	U32 bufferViewCount;
	BufferView* bufferViews;
	U32 bufferCount;
	BufferRef* buffers;
	U32 cameraCount;
	Camera* cameras;
	U32 requiredExtentionCount;
	String* requiredExtentions;
	U32 usedExtensionCount;
	String* usedExtensions;
	U32 imageCount;
	Image* images;
	U32 materialCount;
	Material* materials;
	U32 meshCount;
	Mesh* meshes;
	U32 nodeCount;
	Node* nodes;
	U32 samplerCount;
	SamplerRef* samplers;
	U32 sceneCount;
	//Scene* scenes;
	I32 scene;
	U32 skinCount;
	Skin* skins;
	U32 textureCount;
	TextureRef* textures;

	String name{ NO_INIT };
};

enum AlphaMode
{
	ALPHA_MODE_OPAQUE,
	ALPHA_MODE_MASK,
	ALPHA_MODE_TRANSPARENT,
};

struct alignas(16) NH_API MaterialData
{
	Vector4 baseColorFactor;
	Matrix4 model;
	Matrix4 modelInv;

	Vector3 emissiveFactor;

	F32 metallicFactor;
	F32 roughnessFactor;
	F32 occlusionFactor;

	U32 flags;
};

struct NH_API MeshDraw
{
	Buffer* indexBuffer;
	Buffer* positionBuffer;
	Buffer* tangentBuffer;
	Buffer* normalBuffer;
	Buffer* texcoordBuffer;

	Buffer* materialBuffer;
	MaterialData materialData;

	U32 indexOffset;
	U32 positionOffset;
	U32 tangentOffset;
	U32 normalOffset;
	U32 texcoordOffset;

	U32 count;

	VkIndexType indexType;

	DescriptorSet* descriptorSet;
	
	Matrix4 model;
	Matrix4 modelInv;
};


struct NH_API Transform
{
	Vector3 position;
	Vector3 scale;
	Quaternion3 rotation;

	void CalculateMatrix(Matrix4& matrix)
	{
		matrix.Set(position, rotation, scale);
	}
};

struct Scene
{
public:

private:
	//U32 accessorCount;
	//Accessor* accessors;
	//U32 animationCount;
	//Animation* animations;
	//U32 bufferViewCount;
	//BufferView* bufferViews;
	//U32 bufferCount;
	//BufferRef* buffers;
	//U32 cameraCount;
	//Camera* cameras;
	//U32 requiredExtentionCount;
	//String* requiredExtentions;
	//U32 usedExtensionCount;
	//String* usedExtensions;
	//U32 imageCount;
	//Image* images;
	//U32 materialCount;
	//Material* materials;
	//U32 meshCount;
	//Mesh* meshes;
	//U32 nodeCount;
	//Node* nodes;
	//U32 samplerCount;
	//SamplerRef* samplers;
	//U32 sceneCount;
	//Scene* scenes;
	//I32 scene;
	//U32 skinCount;
	//Skin* skins;
	//U32 textureCount;
	//TextureRef* textures;

public:
	String				name{ NO_INIT };

	Vector<MeshDraw>	mesh_draws;

	Vector<Texture*>	images;
	Vector<Sampler*>	samplers;
	Vector<Buffer*>		buffers;
};

//bool SetupScene(Vector<MeshDraw>& draw);