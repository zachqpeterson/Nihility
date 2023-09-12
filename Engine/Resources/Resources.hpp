#pragma once

#include "ResourceDefines.hpp"

#include "Scene.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "Containers\Pool.hpp"
#include "Math\Math.hpp"
#include "Rendering\Pipeline.hpp"

struct KTXInfo;
struct aiTexture;
struct aiMaterial;
struct aiMesh;
struct aiScene;

class NH_API Resources
{
public:
	static Sampler* CreateSampler(const SamplerInfo& info);
	static DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& info);
	static Renderpass* CreateRenderpass(const RenderpassInfo& info);
	static Shader* CreateShader(const String& name, U8 pushConstantCount = 0, VkPushConstantRange* pushConstants = nullptr);
	static Pipeline* CreatePipeline(const PipelineInfo& info, const SpecializationInfo& specializationInfo = {});
	static Model* LoadModel(const String& name);
	static Skybox* LoadSkybox(const String& name);
	static Scene* CreateScene(const String& name);
	static Scene* LoadScene(const String& name);
	static void SaveScene(const Scene* scene);

	static Sampler* AccessDummySampler();
	static Texture* AccessDummyTexture();
	static Sampler* AccessDefaultSampler();

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

	static bool LoadBinary(const String& name, String& result);
	static U32 LoadBinary(const String& name, void** result);
	static void SaveBinary(const String& name, const String& data);
	static void SaveBinary(const String& name, void* data, U64 length);

	static Texture* CreateTexture(const TextureInfo& info);
	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);
	static Texture* CreateSwapchainTexture(VkImage image, VkFormat format, U8 index);
	static bool RecreateSwapchainTexture(Texture* texture, VkImage image);
	static Texture* LoadTexture(const String& name, bool generateMipMaps = false);

private:
	static bool Initialize();
	static void CreateDefaults();
	static bool CreateBindless();
	static void Shutdown();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, NullPointer);

	static void Update();
	static void UpdatePipelines();

	//Texture Loading
	static bool LoadBMP(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadPNG(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadJPG(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadPSD(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadTIFF(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadTGA(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadKTX(Texture* texture, File& file, bool generateMipMaps);
	static void GetKTXInfo(U32 internalFormat, KTXInfo& info);

	static Texture* AssimpToNhimg(const String& name, const aiTexture* texture);
	static Texture* ConvertToNhimg(const String& name, const U8* data);

	//Model Loading
	static U32 UploadMaterial(const aiMaterial* materialInfo, const aiScene* scene);
	static DrawCall UploadMesh(const aiMesh* meshInfo);
	static void ParseModel(Model* model, DrawCall* drawCalls, U32* meshMaterials, const aiScene* scene);

	//Scene Loading
	static bool LoadNHSCN(Scene* scene, File& file);
	static bool LoadGLTF(Scene* scene, File& file);
	static bool LoadGLB(Scene* scene, File& file);

	static Sampler*								dummySampler;
	static Texture*								dummyTexture;
	static Sampler*								defaultSampler;
	static Shader*								meshProgram;
	static Pipeline*							renderPipeline;

	static Hashmap<String, Sampler>				samplers;
	static Hashmap<String, Texture>				textures;
	static Hashmap<String, Renderpass>			renderpasses;
	static Hashmap<String, Shader>				shaders;
	static Hashmap<String, Pipeline>			pipelines;
	static Hashmap<String, Model>				models;
	static Hashmap<String, Skybox>				skyboxes;
	static Hashmap<String, Scene>				scenes;

	static Queue<ResourceUpdate>				resourceDeletionQueue;
	static Queue<ResourceUpdate>				bindlessTexturesToUpdate;

	static Pool<DescriptorSetLayout, 256>		descriptorSetLayouts;
	static VkDescriptorPool						bindlessDescriptorPool;
	static VkDescriptorSet						bindlessDescriptorSet;
	static DescriptorSetLayout					bindlessDescriptorSetLayout;
	static constexpr U32						maxBindlessResources{ 1024 };
	static constexpr U32						bindlessTextureBinding{ 10 };

	STATIC_CLASS(Resources);
	friend class Renderer;
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