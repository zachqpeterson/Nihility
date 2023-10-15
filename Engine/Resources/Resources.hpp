#pragma once

#include "ResourceDefines.hpp"

#include "Scene.hpp"

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

class NH_API Resources
{
public:
	static Texture* CreateTexture(const TextureInfo& info);
	static Texture* CreateSwapchainTexture(VkImage image, VkFormat format, U8 index);
	static Sampler* CreateSampler(const SamplerInfo& info);
	static DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& info);
	static Renderpass* CreateRenderpass(const RenderpassInfo& info);
	static Shader* CreateShader(const String& name, U8 pushConstantCount = 0, VkPushConstantRange* pushConstants = nullptr);
	static Pipeline* CreatePipeline(const PipelineInfo& info, const SpecializationInfo& specializationInfo = {});
	static Scene* CreateScene(const String& name);

	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);
	static bool RecreateSwapchainTexture(Texture* texture, VkImage image);

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
	static Sampler* AccessDefaultSampler(SamplerType type);

	static Sampler* AccessSampler(const String& name);
	static Texture* AccessTexture(const String& name);
	static Renderpass* AccessRenderpass(const String& name);
	static Pipeline* AccessPipeline(const String& name);

	static Sampler* AccessSampler(HashHandle handle);
	static Texture* AccessTexture(HashHandle handle);
	static Renderpass* AccessRenderpass(HashHandle handle);
	static Pipeline* AccessPipeline(HashHandle handle);

	static void	DestroySampler(Sampler* sampler);
	static void	DestroyTexture(Texture* texture);
	static void	DestroyDescriptorSetLayout(DescriptorSetLayout* layout);
	static void	DestroyDescriptorSet(DescriptorSet* set);
	static void	DestroyRenderpass(Renderpass* renderpass);
	static void DestroyBinary(Binary& binary);

	static U8 MipmapCount(U16 width, U16 height);

	//Convert 3rd party asset formats to nh formats
	static String UploadFont(const String& path);
	static String UploadAudio(const String& path);
	static String UploadTexture(const String& path);
	static String UploadTexture(const aiTexture* textureInfo);
	static String UploadSkybox(const String& path);
	static String UploadModel(const String& path);

private:
	static bool Initialize();
	static void CreateDefaults();
	static bool CreateBindless();
	static void Shutdown();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, NullPointer);

	static void Update();
	static void Resize();
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
	static Sampler*							defaultPointSampler;
	static Sampler*							defaultLinearSampler;
	static Pipeline*						meshPipeline;
	static Pipeline*						skyboxPipeline;
	static Pipeline*						postProcessPipeline;
	static Renderpass*						geometryRenderpass;
	static Renderpass*						postProcessRenderpass;
	static Renderpass*						uiRenderpass;
	static Texture*							geometryBuffer;
	static Texture*							geometryDepth;
	static RenderGraph						defaultRenderGraph;

	static Hashmap<String, Sampler>			samplers;
	static Hashmap<String, Texture>			textures;
	static Hashmap<String, Font>			fonts;
	static Hashmap<String, AudioClip>		audioClips;
	static Hashmap<String, Renderpass>		renderpasses;
	static Hashmap<String, Shader>			shaders;
	static Hashmap<String, Pipeline>		pipelines;
	static Hashmap<String, Model>			models;
	static Hashmap<String, Skybox>			skyboxes;
	static Hashmap<String, Scene>			scenes;

	static Queue<ResourceUpdate>			resourceDeletionQueue;
	static Queue<ResourceUpdate>			bindlessTexturesToUpdate;

	static Pool<DescriptorSetLayout, 256>	descriptorSetLayouts;
	static VkDescriptorPool					bindlessDescriptorPool;
	static VkDescriptorSet					bindlessDescriptorSet;
	static DescriptorSetLayout				bindlessDescriptorSetLayout;
	static constexpr U32					maxBindlessResources{ 1024 };
	static constexpr U32					bindlessTextureBinding{ 10 };

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
			destroy(&hashmap.Obtain(it->handle));
			if constexpr (IsDestroyable<Type>) { it->Destroy(); }
			hashmap.Remove(it->handle);
		}
	}
}

template<typename Type>
inline void Resources::CleanupHashmap(Hashmap<String, Type>& hashmap, NullPointer)
{
	typename Hashmap<String, Type>::Iterator end = hashmap.end();
	for (auto it = hashmap.begin(); it != end; ++it)
	{
		if (it.Valid() && !it->name.Blank())
		{
			if constexpr (IsDestroyable<Type>) { it->Destroy(); }
			hashmap.Remove(it->handle);
		}
	}
}