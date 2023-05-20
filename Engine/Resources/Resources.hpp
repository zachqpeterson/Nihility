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
	static Texture* LoadTexture(const String& name);
	static Buffer* CreateBuffer(const BufferCreation& info);
	static DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& info);
	static DescriptorSet* CreateDescriptorSet(const DescriptorSetCreation& info);
	static ShaderState* CreateShaderState(const ShaderStateCreation& info);
	static RenderPass* CreateRenderPass(const RenderPassCreation& info);
	static Pipeline* CreatePipeline(const PipelineCreation& info);
	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);

	static glTF* LoadScene(const String& name);

	static Sampler* AccessDummySampler();
	static Texture* AccessDummyTexture();
	static Buffer* AccessDummyAttributeBuffer();

	static Sampler* AccessSampler(const String& name);
	static Texture* AccessTexture(const String& name);
	static DescriptorSetLayout* AccessDescriptorSetLayout(const String& name);
	static DescriptorSet* AccessDescriptorSet(const String& name);
	static ShaderState* AccessShaderState(const String& name);
	static RenderPass* AccessRenderPass(const String& name);
	static Pipeline* AccessPipeline(const String& name);

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

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	//Texture Loading
	static void* LoadBMP(Texture* texture, File& file);
	static void* LoadPNG(Texture* texture, File& file);
	static void* LoadJPG(Texture* texture, File& file);
	static void* LoadPSD(Texture* texture, File& file);
	static void* LoadTIFF(Texture* texture, File& file);
	static void* LoadTGA(Texture* texture, File& file);

	NH_HEADER_STATIC Sampler dummySampler;
	NH_HEADER_STATIC Texture dummyTexture;
	NH_HEADER_STATIC Buffer dummyAttributeBuffer;

	NH_HEADER_STATIC Hashmap<String, Texture>				textures{ 512, {} };
	NH_HEADER_STATIC Hashmap<String, Buffer>				buffers{ 4096, {} };
	NH_HEADER_STATIC Hashmap<String, Pipeline>				pipelines{ 128, {} };
	NH_HEADER_STATIC Hashmap<String, Sampler>				samplers{ 32, {} };
	NH_HEADER_STATIC Hashmap<String, DescriptorSetLayout>	descriptorSetLayouts{ 128, {} };
	NH_HEADER_STATIC Hashmap<String, DescriptorSet>			descriptorSets{ 256, {} };
	NH_HEADER_STATIC Hashmap<String, RenderPass>			renderPasses{ 256, {} };
	NH_HEADER_STATIC Hashmap<String, ShaderState>			shaders{ 128, {} };
	NH_HEADER_STATIC Hashmap<String, glTF>					scenes{ 128, {} };

	NH_HEADER_STATIC Queue<ResourceDeletion>				resourceDeletionQueue{};

	STATIC_CLASS(Resources);
	friend class Renderer;
};