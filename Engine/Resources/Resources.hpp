#pragma once

#include "ResourceDefines.hpp"

#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "Material.hpp"

struct Font;
struct AudioClip;
struct aiTexture;
struct aiMaterial;
struct aiMesh;
struct aiScene;
struct DataReader;
struct KTXInfo;
struct VkImage_T;
struct Rendergraph;
struct RendergraphInfo;
struct MeshUpload;
struct ModelUpload;
struct Model;
struct Mesh;
struct Scene;
struct Shader;
struct Pipeline;
struct Material;
struct MaterialData;
struct MaterialInfo;
enum KTXType;
enum KTXFormat;
enum VkFormat;

class NH_API Resources
{
public:
	static ResourceRef<Texture> CreateTexture(const TextureInfo& info, const SamplerInfo& samplerInfo = {});
	static ResourceRef<Texture> CreateSwapchainTexture(VkImage_T* image, VkFormat format, U8 index);
	static ResourceRef<Shader> CreateShader(const String& name); //TODO: Load instead of create
	static ResourceRef<MaterialEffect> CreateMaterialEffect(const String& name, Vector<ResourceRef<Pipeline>>&& pipelines);
	static ResourceRef<MaterialEffect> CreateMaterialEffect(const String& name, const Vector<ResourceRef<Pipeline>>& pipelines);
	static ResourceRef<Material> CreateMaterial(MaterialInfo& info);
	static ResourceRef<Mesh> CreateMesh(const String& name);
	static Scene* CreateScene(const String& name, CameraType cameraType);

	static bool RecreateTexture(ResourceRef<Texture>& texture, U16 width, U16 height, U16 depth);
	static bool RecreateSwapchainTexture(ResourceRef<Texture>& texture, VkImage_T* image);

	static ResourceRef<Font> LoadFont(const String& path);
	static ResourceRef<AudioClip> LoadAudio(const String& path);
	static ResourceRef<Texture> LoadTexture(const String& path);
	static ResourceRef<Skybox> LoadSkybox(const String& path);
	static ResourceRef<Shader> LoadShader(const String& path, ShaderStageType stage);
	static ResourceRef<Pipeline> LoadPipeline(const String& path, U8 pushConstantCount = 0, PushConstant* pushConstants = nullptr);
	static ResourceRef<Material> LoadMaterial(const String& path);
	static ResourceRef<Mesh> LoadMesh(const String& path);
	static ResourceRef<Model> LoadModel(const String& path);
	static Scene* LoadScene(const String& path);
	static Binary LoadBinary(const String& path);
	static String LoadBinaryString(const String& path);

	static void SaveMaterial(const ResourceRef<Material>& material);
	static void SaveScene(Scene* scene);
	static void SaveBinary(const String& path, U32 size, void* data);

	static ResourceRef<Texture> GetTexture(const String& name);
	static ResourceRef<Texture> GetTexture(HashHandle handle);
	static ResourceRef<MaterialEffect> GetMaterialEffect(const String& name);
	static ResourceRef<MaterialEffect> GetMaterialEffect(HashHandle handle);

	static void DestroyBinary(Binary& binary);

	static U8 MipmapCount(U16 width, U16 height);

	//Convert 3rd party asset formats to nh formats
	static String UploadFile(const String& path);
	static String UploadFont(const String& path);
	static String UploadAudio(const String& path);
	static String UploadSkybox(const String& path);
	static String UploadTexture(const String& path, const TextureUpload& upload = {});
	static String UploadTexture(const String& name, U32 index, const aiTexture* textureInfo, TextureUsage usage);
	static String UploadTextures(const String& name, U32 index, const aiTexture* textureInfo0, const aiTexture* textureInfo1, const aiTexture* textureInfo2, U8 def0, U8 def1, U8 def2, TextureUsage usage);
	static String UploadModel(const String& path);

private:
	static bool Initialize();
	static void Shutdown();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy);

	static void Update();

	//Texture Loading
	static U8* LoadKTX(DataReader& reader, U32& faceCount, U32& faceSize, U32& resolution, VkFormat& format);
	static void GetKTXInfo(U32 internalFormat, KTXInfo& info);
	static VkFormat GetKTXFormat(KTXType type, KTXFormat format);

	//Assimp Utilities
	static String ParseAssimpMaterial(const String& name, const aiMaterial* materialInfo, const aiScene* scene);
	static String ParseAssimpMesh(const String& name, const aiMesh* meshInfo);
	static void ParseAssimpModel(ModelUpload& model, const aiScene* scene);

	static Hashmap<String, Texture>			textures;
	static Hashmap<String, Skybox>			skyboxes;
	static Hashmap<String, Font>			fonts;
	static Hashmap<String, AudioClip>		audioClips;
	static Hashmap<String, Shader>			shaders;
	static Hashmap<String, Pipeline>		pipelines;
	static Hashmap<String, MaterialEffect>	materialEffects;
	static Hashmap<String, Material>		materials;
	static Hashmap<String, Mesh>			meshes;
	static Hashmap<String, Model>			models;
	static Hashmap<String, Scene>			scenes;

	static Queue<ResourceUpdate>			bindlessTexturesToUpdate;

	static MaterialEffect					pbrOpaque;
	static MaterialEffect					pbrTransparent;

	STATIC_CLASS(Resources);
	friend class Renderer;
	friend class UI;
	friend class Engine;
	friend struct CommandBuffer;
	friend struct Shader;
	friend struct Pipeline;
	friend struct Scene;
};

template<typename Type>
inline void Resources::CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy)
{
	typename Hashmap<String, Type>::Iterator end = hashmap.end();
	for (auto it = hashmap.begin(); it != end; ++it)
	{
		if (it.Valid() && !it->name.Blank())
		{
			destroy(hashmap.Obtain(it->handle));
		}
	}
}