#pragma once

#include "ResourceDefines.hpp"

#include "Scene.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "Containers\Pool.hpp"
#include "Math\Math.hpp"

struct Pipeline;
struct KTXInfo;

class NH_API Resources
{
public:
	static Sampler* CreateSampler(const SamplerCreation& info);
	static Texture* CreateTexture(const TextureCreation& info);
	static Texture* CreateSwapchainTexture(VkImage image, VkFormat format, U8 index);
	static bool RecreateSwapchainTexture(Texture* texture, VkImage image);
	static Texture* LoadTexture(const String& name, bool generateMipMaps = false);
	static Buffer* CreateBuffer(const BufferCreation& info);
	static Buffer* LoadBuffer(const BufferCreation& info);
	static DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& info);
	static DescriptorSet* CreateDescriptorSet(DescriptorSetLayout* layout);
	static void UpdateDescriptorSet(DescriptorSet* descriptorSet, Texture** textures, Buffer** buffers);
	static void UpdateDescriptorSet(DescriptorSet* descriptorSet, Texture* texture, U32 binding);
	static void UpdateDescriptorSet(DescriptorSet* descriptorSet, Buffer* buffer, U32 binding);
	static Renderpass* CreateRenderPass(const RenderPassCreation& info);
	static Pipeline* CreatePipeline(const String& name, bool RenderToswapchain = false);
	static Program* CreateProgram(const ProgramCreation& info);
	static Material* CreateMaterial(const MaterialCreation& info);
	static Skybox* CreateSkybox(const SkyboxCreation& info);
	static void SaveSkybox(Skybox* skybox);
	static Skybox* LoadSkybox(const String& name);
	static Scene* LoadScene(const String& name);
	static void SaveScene(const Scene* scene);
	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);

	static Sampler* AccessDummySampler();
	static Texture* AccessDummyTexture();
	static Buffer* AccessDummyAttributeBuffer();
	static Sampler* AccessDefaultSampler();
	static Material* AccessDefaultMaterial(bool transparent = false);

	static Sampler* AccessSampler(const String& name);
	static Texture* AccessTexture(const String& name);
	static Renderpass* AccessRenderPass(const String& name);
	static Pipeline* AccessPipeline(const String& name);

	static Sampler* AccessSampler(HashHandle handle);
	static Texture* AccessTexture(HashHandle handle);
	static Renderpass* AccessRenderPass(HashHandle handle);
	static Pipeline* AccessPipeline(HashHandle handle);

	static void	DestroySampler(Sampler* sampler);
	static void	DestroyTexture(Texture* texture);
	static void	DestroyBuffer(Buffer* buffer);
	static void	DestroyDescriptorSetLayout(DescriptorSetLayout* layout);
	static void	DestroyDescriptorSet(DescriptorSet* set);
	static void	DestroyRenderPass(Renderpass* renderpass);

	static bool LoadBinary(const String& name, String& result);
	static U32 LoadBinary(const String& name, void** result);

private:
	static bool Initialize();
	static void CreateDefaults();
	static bool CreateBindless();
	static void Shutdown();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, NullPointer);

	static void Update();

	//Texture Loading
	static bool LoadBMP(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadPNG(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadJPG(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadPSD(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadTIFF(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadTGA(Texture* texture, File& file, bool generateMipMaps);
	static bool LoadKTX(Texture* texture, File& file, bool generateMipMaps);
	static void GetKTXInfo(U32 internalFormat, KTXInfo& info);

	static Sampler*								dummySampler;
	static Texture*								dummyTexture;
	static Buffer*								dummyAttributeBuffer;
	static Sampler*								defaultSampler;
	static Program*								skyboxProgram;
	static Program*								compositionProgram;
	static Material*							materialOpaque;
	static Material*							materialTransparent;

	static VkDescriptorPool						descriptorPool;

	static Hashmap<String, Sampler>				samplers;
	static Hashmap<String, Texture>				textures;
	static Hashmap<String, Buffer>				buffers;
	static Pool<DescriptorSet, 256>				descriptorSets;
	static Pool<DescriptorSetLayout, 256>		descriptorSetLayouts;
	static Hashmap<String, Renderpass>			renderPasses;
	static Hashmap<String, Pipeline>			pipelines;
	static Hashmap<String, Program>				programs;
	static Hashmap<String, Material>			materials;
	static Hashmap<String, Skybox>				skyboxes;
	static Hashmap<String, Scene>				scenes;

	static Queue<ResourceUpdate>				resourceDeletionQueue;
	static Queue<ResourceUpdate>				bindlessTexturesToUpdate;

	static VkDescriptorPool						bindlessDescriptorPool;
	static VkDescriptorSet						bindlessDescriptorSet;
	static VkDescriptorSetLayout				bindlessDescriptorSetLayout;
	static constexpr U32						maxBindlessResources{ 1024 };
	static constexpr U32						bindlessTextureBinding{ 10 };

	STATIC_CLASS(Resources);
	friend class Renderer;
	friend class Engine;
	friend struct CommandBuffer;
	friend struct Pipeline;
};

template<typename Type>
inline void Resources::CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy)
{
	typename Hashmap<String, Type>::Iterator end0 = hashmap.end();
	for (auto it = hashmap.begin(); it != end0; ++it)
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
	typename Hashmap<String, Type>::Iterator end0 = hashmap.end();
	for (auto it = hashmap.begin(); it != end0; ++it)
	{
		if (it.Valid() && !it->name.Blank())
		{
			if constexpr (IsDestroyable<Type>) { it->Destroy(); }
			hashmap.Remove(it->handle);
		}
	}
}