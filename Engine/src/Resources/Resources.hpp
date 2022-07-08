#pragma once

#include "Defines.hpp"

#include "Shader.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Containers/HashMap.hpp"
#include "Math/Math.hpp"

#undef LoadImage

enum ImageType
{
	IMAGE_TYPE_BMP,
	IMAGE_TYPE_PNG,
	IMAGE_TYPE_JPG,
	IMAGE_TYPE_TGA,
};

enum ImageLayout
{
	IMAGE_LAYOUT_RGBA32,
	IMAGE_LAYOUT_BGRA32,
	IMAGE_LAYOUT_RGB24,
	IMAGE_LAYOUT_BGR24,
};

enum TextureUse
{
	TEXTURE_USE_UNKNOWN = 0x00,
	TEXTURE_USE_MAP_DIFFUSE = 0x01,
	TEXTURE_USE_MAP_SPECULAR = 0x02,
	TEXTURE_USE_MAP_NORMAL = 0x03
};

enum TextureFlag
{
	TEXTURE_FLAG_HAS_TRANSPARENCY = 0x1,
	TEXTURE_FLAG_IS_WRITEABLE = 0x2,
	TEXTURE_FLAG_IS_WRAPPED = 0x4,
};

enum TextureFilter
{
	TEXTURE_FILTER_MODE_NEAREST = 0x0,
	TEXTURE_FILTER_MODE_LINEAR = 0x1
};

enum TextureRepeat
{
	TEXTURE_REPEAT_REPEAT = 0x1,
	TEXTURE_REPEAT_MIRRORED_REPEAT = 0x2,
	TEXTURE_REPEAT_CLAMP_TO_EDGE = 0x3,
	TEXTURE_REPEAT_CLAMP_TO_BORDER = 0x4
};

enum RenderpassClearFlag
{
	RENDERPASS_CLEAR_NONE_FLAG = 0x0,
	RENDERPASS_CLEAR_COLOR_BUFFER_FLAG = 0x1,
	RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
	RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
};

struct Binary
{
	String name;
	Vector<U8> data;
};

struct Image
{
	String name;
	U32 width;
	U32 height;
	U8 channelCount;
	ImageLayout layout;
	Vector<U8> pixels;
};

struct Texture
{
	String name;
	U32 generation;
	U32 width;
	U32 height;
	U8 flags;
	U8 channelCount;
	ImageLayout layout;
	void* internalData;
};

struct TextureMap
{
	Texture* texture;
	TextureUse use;
	TextureFilter filterMinify;
	TextureFilter filterMagnify;
	TextureRepeat repeatU;
	TextureRepeat repeatV;
	TextureRepeat repeatW;
	void* internalData;
};

struct RenderTarget
{
	bool syncToWindowSize;
	Vector<Texture*> attachments;
	void* internalFramebuffer;
};

struct Renderpass
{
	String name;

	Vector4 renderArea;

	Vector4 clearColor; //TODO: color struct
	Vector<RenderTarget> targets;

	U8 clearFlags;
	F32 depth;
	U32 stencil;

	void* internalData;
};

struct MaterialConfig
{
	String name;
	String shaderName;
	bool autoRelease{ false };
	Vector4 diffuseColor; //TODO: Color struct
	F32 shininess;
	String diffuseMapName;
	String specularMapName;
	String normalMapName;
};

struct Material
{
	U32 id;
	String name;
	Shader* shader;
	U32 generation;
	U32 internalId;
	Vector4 diffuseColor; //TODO: Color struct
	TextureMap diffuseMap;
	TextureMap specularMap;
	TextureMap normalMap;
	F32 shininess;
	U64 renderFrameNumber;
};

struct MeshConfig
{
	String name;
	String MaterialName;

	Vector<Vertex> vertices;
	Vector<U32> indices;

	Vector3 center;
	Vector3 minExtents;
	Vector3 maxExtents;
};

struct Mesh
{
	String name;
	Material* material;
	void* internalData;
};

struct Model
{
	String name;
	Vector<Mesh*> meshes;
	Transform transform;
};

class NH_API Resources
{
public:
	static bool Initialize();
	static void Shutdown();

	static Binary* LoadBinary(const String& name);
	static void UnloadBinary(Binary* binary);
	static Texture* LoadTexture(const String& name);
	static Material* LoadMaterial(const String& name);
	static Mesh* LoadMesh(const String& name);
	static Model* LoadModel(const String& name);

	static Texture* CreateWritableTexture(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency);
	static Texture* CreateTextureFromInternal(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency, bool isWriteable, bool registerTexture, void* internalData);
	static bool SetTextureInternal(Texture* texture, void* internalData);
	static bool ResizeTexture(Texture* texture, U32 width, U32 height, bool regenerateInternalData);

	static Vector<Renderpass*>& GetRenderpasses() { return renderpasses; }
	static Vector<Material*>& GetMaterials() { return materials; }

	static Mesh* CreateMesh(MeshConfig& config);

	static Texture* DefaultTexture() { return defaultTexture; }
	static Texture* DefaultDiffuse() { return defaultDiffuse; }
	static Texture* DefaultSpecular() { return defaultSpecular; }
	static Texture* DefaultNormal() { return defaultNormal; }

	static Shader* DefaultMaterialShader() { return defaultMaterialShader; }

	static Material* DefaultMaterial() { return defaultMaterial; }

	//TODO: Get copies of default meshes

private:
	static void CreateShaders();
	static void LoadSettings();
	static void WriteSettings();

	static Image* LoadImage(const String& name);
	static void UnloadImage(Image* image);
	static bool LoadBMP(Image* image, struct File* file);
	static bool ReadBMPHeader(struct BMPHeader& header, struct BMPInfo& info, File* file);
	static void SetBmpColorMasks(struct BMPInfo& info);
	static bool LoadPNG(Image* image, struct File* file);
	static bool LoadJPG(Image* image, struct File* file);
	static bool LoadTGA(Image* image, struct File* file);

	static void DestroyTexture(Texture* texture);

	static void DestroyRenderpass(Renderpass* renderpass);

	static void CreateMaterial(MaterialConfig& config, Material* material);
	static void DestroyMaterial(Material* material);

	static void LoadOBJ(Model* mesh, struct File* file);
	static void LoadKSM(Model* mesh, struct File* file);

	static void GetConfigType(const String& field, FieldType& type, U32& size);

	static Renderpass* LoadRenderpass(const String& name);
	static Shader* LoadShader(const String& name);

	static void DestroyMesh(Mesh* mesh);

	//Textures
	static HashMap<String, Texture*> textures;
	static Texture* defaultTexture;
	static Texture* defaultDiffuse;
	static Texture* defaultSpecular;
	static Texture* defaultNormal;

	//Renderpasses
	static Vector<Renderpass*> renderpasses;

	//Shaders
	static Vector<Shader*> shaders;
	static Shader* defaultMaterialShader;

	//Materials
	static Vector<Material*> materials;
	static Material* defaultMaterial;

	//Mesh
	static HashMap<String, Mesh*> meshes;
	static Mesh* cubeMesh;
	static Mesh* sphereMesh;
	static Mesh* capsuleMesh;
	static Mesh* quadMesh;

	//TODO: Model
	static HashMap<String, Model*> models;

	Resources() = delete;

	friend class Engine;
};