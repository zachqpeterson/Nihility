#pragma once

#include "ResourceDefines.hpp"

#include "Scene.hpp"

#include "ResourceDefines.hpp"
#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "Containers\Pool.hpp"
#include "Math\Math.hpp"
#include "Rendering\Pipeline.hpp"

struct Font;
struct AudioClip;
struct aiTexture;
struct aiMaterial;
struct aiMesh;
struct aiScene;
struct DataReader;
struct KTXInfo;
struct VkImage_T;
enum KTXType;
enum KTXFormat;
enum VkFormat;

class NH_API Resources
{
public:
	static Texture* CreateTexture(const TextureInfo& info, const SamplerInfo& samplerInfo = {});
	static Texture* CreateSwapchainTexture(VkImage_T* image, VkFormat format, U8 index);
	static Renderpass* CreateRenderpass(const RenderpassInfo& info, const Vector<PipelineInfo>& pipelines);
	static Shader* CreateShader(const String& name, U8 pushConstantCount = 0, PushConstant* pushConstants = nullptr);
	static Pipeline* CreatePipeline(const PipelineInfo& info, Renderpass* renderpass);
	static Scene* CreateScene(const String& name, CameraType cameraType);

	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);
	static bool RecreateSwapchainTexture(Texture* texture, VkImage_T* image);

	static void RecreateRenderpass(Renderpass* renderpass);

	static Font* LoadFont(const String& path);
	static AudioClip* LoadAudio(const String& path);
	static Texture* LoadTexture(const String& path);
	static Model* LoadModel(const String& path);
	static Skybox* LoadSkybox(const String& path);
	static Scene* LoadScene(const String& path);
	static Binary LoadBinary(const String& path);
	static String LoadBinaryString(const String& path);

	static void SaveScene(const Scene* scene);
	static void SaveBinary(const String& path, U32 size, void* data);

	static Texture* AccessDummyTexture();

	static Texture* AccessTexture(const String& name);
	static Renderpass* AccessRenderpass(const String& name);
	static Pipeline* AccessPipeline(const String& name);

	static Texture* AccessTexture(HashHandle handle);
	static Renderpass* AccessRenderpass(HashHandle handle);
	static Pipeline* AccessPipeline(HashHandle handle);

	static void	DestroyTexture(Texture* texture);
	static void	DestroyRenderpass(Renderpass* renderpass);
	static void DestroyBinary(Binary& binary);

	static U8 MipmapCount(U16 width, U16 height);

	//Convert 3rd party asset formats to nh formats
	static String UploadFont(const String& path);
	static String UploadAudio(const String& path);
	static String UploadTexture(const String& path, const TextureUpload& upload = {});
	static String UploadTexture(const aiTexture* textureInfo, TextureUsage usage);
	static String UploadSkybox(const String& path);
	static String UploadModel(const String& path);

private:
	static bool Initialize();
	static void Shutdown();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy);

	static void Update();
	static void UseSkybox(Skybox* skybox);

	//Texture Loading
	static U8* LoadKTX(DataReader& reader, U32& faceCount, U32& faceSize, U32& resolution, VkFormat& format);
	static void GetKTXInfo(U32 internalFormat, KTXInfo& info);
	static VkFormat GetKTXFormat(KTXType type, KTXFormat format);
	static U8* LoadHDRToCube(DataReader& reader, U32& faceSize, U32& resolution, VkFormat& format);

	//Assimp Utilities
	static Material ParseAssimpMaterial(ModelUpload& model, const aiMaterial* materialInfo, const aiScene* scene);
	static MeshUpload ParseAssimpMesh(const aiMesh* meshInfo);
	static void ParseAssimpModel(ModelUpload& model, const aiScene* scene);

	static Texture*							dummyTexture;
	static PipelineGraph					defaultPipelineGraph;
	static Pipeline*						meshPipeline;
	static Pipeline*						postProcessPipeline;
	static Pipeline*						skyboxPipeline;

	static Hashmap<String, Texture>			textures;
	static Hashmap<String, Font>			fonts;
	static Hashmap<String, AudioClip>		audioClips;
	static Hashmap<String, Renderpass>		renderpasses;
	static Hashmap<String, Shader>			shaders;
	static Hashmap<String, Pipeline>		pipelines;
	static Hashmap<String, Model>			models;
	static Hashmap<String, Skybox>			skyboxes;
	static Hashmap<String, Scene>			scenes;

	static Queue<ResourceUpdate>			bindlessTexturesToUpdate;

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