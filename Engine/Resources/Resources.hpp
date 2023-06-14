#pragma once

#include "ResourceDefines.hpp"

#include "Scene.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "Math\Math.hpp"

class NH_API Resources
{
public:
	static Sampler* CreateSampler(const SamplerCreation& info);
	static Texture* CreateTexture(const TextureCreation& info);
	static Texture* LoadTexture(const String& name, bool generateMipMaps = false);
	static Buffer* CreateBuffer(const BufferCreation& info);
	static Buffer* LoadBuffer(const BufferCreation& info);
	static DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& info);
	static DescriptorSet* CreateDescriptorSet(const DescriptorSetCreation& info);
	static ShaderState* CreateShaderState(const ShaderStateCreation& info);
	static RenderPass* CreateRenderPass(const RenderPassCreation& info);
	static Pipeline* CreatePipeline(const PipelineCreation& info);
	static Program* CreateProgram(const ProgramCreation& info);
	static Material* CreateMaterial(const MaterialCreation& info);
	static Scene* LoadScene(const String& name);
	static void SaveScene(const Scene* scene);
	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);

	static Sampler* AccessDummySampler();
	static Texture* AccessDummyTexture();
	static Buffer* AccessDummyAttributeBuffer();
	static Sampler* AccessDefaultSampler();
	static Material* AccessDefaultMaterial(bool transparent = false, bool culling = true);

	static Sampler* AccessSampler(const String& name);
	static Texture* AccessTexture(const String& name);
	static DescriptorSetLayout* AccessDescriptorSetLayout(const String& name);
	static DescriptorSet* AccessDescriptorSet(const String& name);
	static ShaderState* AccessShaderState(const String& name);
	static RenderPass* AccessRenderPass(const String& name);
	static Pipeline* AccessPipeline(const String& name);

	static Sampler* AccessSampler(HashHandle handle);
	static Texture* AccessTexture(HashHandle handle);
	static DescriptorSetLayout* AccessDescriptorSetLayout(HashHandle handle);
	static DescriptorSet* AccessDescriptorSet(HashHandle handle);
	static ShaderState* AccessShaderState(HashHandle handle);
	static RenderPass* AccessRenderPass(HashHandle handle);
	static Pipeline* AccessPipeline(HashHandle handle);

	static void	DestroySampler(Sampler* sampler);
	static void	DestroyTexture(Texture* texture);
	static void	DestroyBuffer(Buffer* buffer);
	static void	DestroyDescriptorSetLayout(DescriptorSetLayout* layout);
	static void	DestroyDescriptorSet(DescriptorSet* set);
	static void	DestroyShaderState(ShaderState* shader);
	static void	DestroyRenderPass(RenderPass* renderPass);
	static void	DestroyPipeline(Pipeline* pipeline);

	static bool LoadBinary(const String& name, String& result);
	static U32 LoadBinary(const String& name, void** result);
	static void ParseSPIRV(VkShaderModuleCreateInfo& shaderInfo, ShaderState* shaderState);

private:
	static bool Initialize();
	static void CreateDefaults();
	static void Shutdown();

	static void Update();

	//Texture Loading
	static void* LoadBMP(Texture* texture, File& file);
	static void* LoadPNG(Texture* texture, File& file);
	static void* LoadJPG(Texture* texture, File& file);
	static void* LoadPSD(Texture* texture, File& file);
	static void* LoadTIFF(Texture* texture, File& file);
	static void* LoadTGA(Texture* texture, File& file);
	static void* LoadKTX(Texture* texture, File& file);

	static Sampler*								dummySampler;
	static Texture*								dummyTexture;
	static Buffer*								dummyAttributeBuffer;
	static Sampler*								defaultSampler;
	static Material*							materialNoCullOpaque;
	static Material*							materialCullOpaque;
	static Material*							materialNoCullTransparent;
	static Material*							materialCullTransparent;

	static Hashmap<String, Sampler>				samplers;
	static Hashmap<String, Texture>				textures;
	static Hashmap<String, Buffer>				buffers;
	static Hashmap<String, DescriptorSetLayout>	descriptorSetLayouts;
	static Hashmap<String, DescriptorSet>		descriptorSets;
	static Hashmap<String, ShaderState>			shaders;
	static Hashmap<String, RenderPass>			renderPasses;
	static Hashmap<String, Pipeline>			pipelines;
	static Hashmap<String, Program>				programs;
	static Hashmap<String, Material>			materials;
	static Hashmap<String, Scene>				scenes;

	static Queue<ResourceUpdate>				resourceDeletionQueue;

	STATIC_CLASS(Resources);
	friend class Renderer;
	friend class Engine;
};