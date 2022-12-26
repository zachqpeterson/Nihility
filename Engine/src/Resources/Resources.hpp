#pragma once

#include "Defines.hpp"

#include "Renderer/Shader.hpp"
#include "Renderer/Material.hpp"

#include "Containers/String.hpp"
#include <Containers/Vector.hpp>
#include <Containers/HashTable.hpp>
#include "Math/Math.hpp"

#include <xaudio2.h>

#undef LoadImage

#define AUDIO_CHUNK_LENGTH 96000

struct PhysicsObject2D;
struct PhysicsObject3D;

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

enum RenderpassBufferFlag
{
	RENDERPASS_BUFFER_FLAG_NONE = 0x0,
	RENDERPASS_BUFFER_FLAG_COLOR = 0x1,
	RENDERPASS_BUFFER_FLAG_DEPTH = 0x2,
	RENDERPASS_BUFFER_FLAG_STENCIL = 0x4
};

enum RenderPassState
{
	RENDERPASS_STATE_READY,
	RENDERPASS_STATE_RECORDING,
	RENDERPASS_STATE_IN_RENDER_PASS,
	RENDERPASS_STATE_RECORDING_ENDED,
	RENDERPASS_STATE_SUBMITTED,
	RENDERPASS_STATE_NOT_ALLOCATED
};

struct NH_API Binary
{
	String name;
	Vector<U8> data;
};

struct NH_API Image
{
	String name;
	U32 width{ 0 };
	U32 height{ 0 };
	U8 channelCount{ 4 };
	ImageLayout layout{ IMAGE_LAYOUT_RGBA32 };
	Vector<U8> pixels;
};

struct NH_API Texture
{
	String name;
	U32 generation{ 0 };
	U32 width{ 0 };
	U32 height{ 0 };
	U8 flags{ 0 };
	U8 channelCount{ 4 };
	U8 sampleCount{ 1 };
	ImageLayout layout{ IMAGE_LAYOUT_RGBA32 };
	void* internalData{ nullptr };
};

struct NH_API TextureMap
{
	Texture* texture{ nullptr };
	TextureFilter filterMinify{ TEXTURE_FILTER_MODE_NEAREST };
	TextureFilter filterMagnify{ TEXTURE_FILTER_MODE_NEAREST };
	TextureRepeat repeatU{ TEXTURE_REPEAT_REPEAT };
	TextureRepeat repeatV{ TEXTURE_REPEAT_REPEAT };
	TextureRepeat repeatW{ TEXTURE_REPEAT_REPEAT };
	void* internalData{ nullptr };
};

struct RenderTarget
{
	bool syncToWindowSize;
	Vector<Texture*> attachments;
	void* internalFramebuffer;
};

struct Character
{
	F32 x, y;
	F32 uvWidth, uvHeight;
	F32 width, height;
	F32 xOffset, yOffset;
	F32 xAdvance;
};

struct Font
{
	String name;
	Character characters[256];
	Texture* texture;
};

struct AudioData
{
	String name;
	U8* data;
	U32 size;
	WAVEFORMATEX format;
};

struct VkRenderPass_T;

struct Renderpass
{
	String name;

	Vector4 renderArea;

	Vector4 clearColor; //TODO: color struct
	Vector<RenderTarget> targets;

	U8 clearFlags;
	F32 depth;
	U32 stencil;

	RenderPassState state;

	bool hasPrevPass;
	bool hasNextPass;

	VkRenderPass_T* handle;
};

struct MaterialConfig
{
	String shaderName;
	F32 shininess = 0.0f;
	Vector4 diffuseColor; //TODO: Color struct
	Vector<String> textureMapNames;
};

struct MeshConfig
{
	String name;
	String MaterialName;

	Vector<Texture*> instanceTextures;

	void* vertices;
	U32 vertexCount;
	U32 vertexSize;
	Vector<U32> indices;

	Vector3 center;
	Vector3 minExtents;
	Vector3 maxExtents;
};

struct NH_API Mesh
{
	String name;
	Material material;
	void* vertices{ nullptr };
	U32 vertexCount{ 0 };
	U32 vertexSize{ 0 };
	Vector<U32> indices;
	void* pushConstant{ nullptr };
	void* internalData{ nullptr };
};

struct NH_API Model
{
	String name;
	Vector<Mesh*> meshes;
};

struct NH_API GameObject2DConfig
{
	String name;
	Transform2D* transform{ nullptr };
	PhysicsObject2D* physics{ nullptr };
	Model* model{ nullptr };
};

struct NH_API GameObject2D
{
	U64 id{ U64_MAX };
	String name;
	Transform2D* transform{ nullptr };
	PhysicsObject2D* physics{ nullptr };
	Model* model{ nullptr };
	bool enabled{ true };
};

struct NH_API GameObject3DConfig
{
	String name;
	Transform3D* transform{ nullptr };
	PhysicsObject3D* physics{ nullptr };
	Model* model{ nullptr };
};

struct NH_API GameObject3D
{
	U64 id{ U64_MAX };
	String name;
	Transform3D* transform{ nullptr };
	PhysicsObject3D* physics{ nullptr };
	Model* model{ nullptr };
};

struct File;
struct RiffIterator;

class NH_API Resources
{
public:
	static bool Initialize();
	static void Shutdown();

	static Binary* LoadBinary(const String& name);
	static void UnloadBinary(Binary* binary);
	static Texture* LoadTexture(const String& name);
	static AudioData* LoadAudio(const String& name);
	static Font* LoadFont(const String& name);
	static void GetMaterialInstance(const String& name, Vector<Texture*>& instanceTextures, Material& instance);
	static void ChangeInstanceTextures(Material& material, const Vector<Texture*>& instanceTextures);
	static Mesh* LoadMesh(const String& name);
	static void DestroyMesh(Mesh* mesh);
	static void DestroyFreeMesh(Mesh* mesh);
	static Model* LoadModel(const String& name);
	static void DestroyModel(Model* model);

	static Texture* CreateWritableTexture(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency);
	static Texture* CreateTextureFromInternal(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency, bool isWriteable, bool registerTexture, void* internalData);
	static bool SetTextureInternal(Texture* texture, void* internalData);
	static bool ResizeTexture(Texture* texture, U32 width, U32 height, bool regenerateInternalData);

	static Texture* CreateFontCharacter(const String& fontName, I32 c, F32 heightPixels, const Vector3& color, I32& xOff, I32& yOff);

	static Vector<Renderpass*>& GetRenderpasses() { return renderpasses; }
	static Vector<Material*>& GetMaterials() { return materials; }

	static Mesh* CreateMesh(MeshConfig& config);
	static Mesh* CreateFreeMesh(MeshConfig& config);
	static void BatchCreateFreeMeshes(Vector<MeshConfig>& configs, Vector<Mesh*>& meshes);
	static Model* CreateModel(const String& name, const Vector<Mesh*>& meshes);
	static GameObject2D* CreateGameObject2D(const GameObject2DConfig& config);
	static void DestroyGameObject2D(GameObject2D* gameObject);

	static Texture* DefaultTexture() { return defaultTexture; }
	static Texture* DefaultDiffuse() { return defaultDiffuse; }
	static Texture* DefaultSpecular() { return defaultSpecular; }
	static Texture* DefaultNormal() { return defaultNormal; }

	static Shader* DefaultMaterialShader() { return defaultMaterialShader; }

	//TODO: Get copies of default meshes

private:
	static void CreateShaders();
	static void LoadSettings();
	static void WriteSettings();

	static Image* LoadImage(const String& name);
	static void UnloadImage(Image* image);
	static bool LoadBMP(Image* image, File& file);
	static bool ReadBMPHeader(struct BMPHeader& header, struct BMPInfo& info, File& file);
	static void SetBmpColorMasks(struct BMPInfo& info);
	static bool LoadPNG(Image* image, File& file);
	static bool LoadJPG(Image* image, File& file);
	static bool LoadTGA(Image* image, File& file);

	static void LoadWAV(AudioData& audio);
	static void DestroyAudio(AudioData& audio);
	static RiffIterator ParseChunkAt(void* at, void* stop);
	static RiffIterator NextChunk(RiffIterator& it);
	static void* GetChunkData(const RiffIterator& it);
	static bool IsValid(const RiffIterator& it);
	static U32 GetType(const RiffIterator& it);
	static U32 GetChunkSize(const RiffIterator& it);

	static void DestroyTexture(Texture* texture);

	static void DestroyRenderpass(Renderpass* renderpass);

	static Material* LoadMaterial(const String& name);
	static void CreateMaterial(MaterialConfig& config, Material* material);
	static void DestroyMaterial(Material* material);
	static void DestroyMaterialInstance(Material& material);

	static void LoadOBJ(Model* mesh, File& file);
	static void LoadKSM(Model* mesh, File& file);

	static void GetConfigType(const String& field, FieldType& type, U32& size);

	static Renderpass* LoadRenderpass(const String& name);
	static Shader* LoadShader(const String& name);

	static void DestroyFont(Font* font);

	//Textures
	static HashTable<String, Texture*> textures;
	static Texture* defaultTexture;
	static Texture* defaultDiffuse;
	static Texture* defaultSpecular;
	static Texture* defaultNormal;

	//Audio
	static HashTable<String, AudioData> audio;

	//Fonts
	static HashTable<String, Font*> fonts;

	//Renderpasses
	static Vector<Renderpass*> renderpasses;

	//Shaders
	static Vector<Shader*> shaders;
	static Shader* defaultMaterialShader;

	//Materials
	static Vector<Material*> materials;
	static Material* defaultMaterial;

	//Meshes
	static HashTable<String, Mesh*> meshes;
	static Mesh* cubeMesh;
	static Mesh* sphereMesh;
	static Mesh* capsuleMesh;
	static Mesh* quadMesh;

	//Models
	static HashTable<String, Model*> models;

	//GameObjects
	static HashTable<U64, GameObject2D*> gameObjects2D;
	static U64 gameObjectId;

	Resources() = delete;

	friend class Engine;
};