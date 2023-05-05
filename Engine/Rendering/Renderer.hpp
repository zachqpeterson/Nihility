#pragma once

#include "RenderingDefines.hpp"

#include "Containers\Hashmap.hpp"
#include "CommandBuffer.hpp"

class NH_API Renderer
{
public:


private:
	static bool Initialize(CSTR applicationName, U32 applicationVersion);
	static void Shutdown();

	static bool CreateInstance();
	static bool CreateSurface();
	static bool SelectGPU();
	static bool GetFamilyQueue(VkPhysicalDevice gpu);
	static bool CreateDevice();
	static bool SetFormats();
	static bool CreateSwapchain();
	static bool CreatePools();
	static bool CreatePrimitiveResources();

	static void Update();

	static void* MapBuffer(const MapBufferParameters& parameters);
	static void UnmapBuffer(const MapBufferParameters& parameters);
	static void* DynamicAllocate(U32 size);

	static void SetResourceName(VkObjectType type, U64 handle, CSTR name);


	static void FrameCountersAdvance();
	static void QueueCommandBuffer(CommandBuffer* commandBuffer);
	static CommandBuffer* GetCommandBuffer(QueueType type, bool begin);
	static CommandBuffer* GetInstantCommandBuffer();
	static void CreateTexture(const TextureCreation& creation, TextureHandle handle, Texture* texture);
	static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth);
	static void FillWriteDescriptorSets(const DesciptorSetLayout* descriptorSetLayout, VkDescriptorSet vkDescriptorSet,
		VkWriteDescriptorSet* descriptorWrite, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo,
		VkSampler vkDefaultSampler, U32& numResources, const ResourceHandle* resources, const SamplerHandle* samplers, const U16* bindings);
	static VkShaderModuleCreateInfo CompileShader(CSTR code, U32 codeSize, VkShaderStageFlagBits stage, CSTR name);
	static VkRenderPass CreateVulkanRenderPass(const RenderPassOutput& output, CSTR name);
	static VkRenderPass GetRenderPass(const RenderPassOutput& output, CSTR name);
	static void CreateSwapchainPass(const RenderPassCreation& creation, RenderPass* renderPass);
	static void CreateFramebuffer(RenderPass* renderPass, const TextureHandle* outputTextures, U32 numRenderTargets, TextureHandle depthStencilTexture);
	static RenderPassOutput FillRenderPassOutput(const RenderPassCreation& creation);

	static BufferHandle					CreateBuffer(const BufferCreation& creation);
	static TextureHandle				CreateTexture(const TextureCreation& creation);
	static PipelineHandle				CreatePipeline(const PipelineCreation& creation);
	static SamplerHandle				CreateSampler(const SamplerCreation& creation);
	static DescriptorSetLayoutHandle	CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& creation);
	static DescriptorSetHandle			CreateDescriptorSet(const DescriptorSetCreation& creation);
	static RenderPassHandle				CreateRenderPass(const RenderPassCreation& creation);
	static ShaderStateHandle			CreateShaderState(const ShaderStateCreation& creation);
	static void							DestroyBuffer(BufferHandle buffer);
	static void							DestroyTexture(TextureHandle texture);
	static void							DestroyPipeline(PipelineHandle pipeline);
	static void							DestroySampler(SamplerHandle sampler);
	static void							DestroyDescriptorSetLayout(DescriptorSetLayoutHandle layout);
	static void							DestroyDescriptorSet(DescriptorSetHandle set);
	static void							DestroyRenderPass(RenderPassHandle renderPass);
	static void							DestroyShaderState(ShaderStateHandle shader);
	static void							DestroyBufferInstant(ResourceHandle buffer);
	static void							DestroyTextureInstant(ResourceHandle texture);
	static void							DestroyPipelineInstant(ResourceHandle pipeline);
	static void							DestroySamplerInstant(ResourceHandle sampler);
	static void							DestroyDescriptorSetLayoutInstant(ResourceHandle layout);
	static void							DestroyDescriptorSetInstant(ResourceHandle set);
	static void							DestroyRenderPassInstant(ResourceHandle renderPass);
	static void							DestroyShaderStateInstant(ResourceHandle shader);
	static ShaderState*					AccessShaderState(ShaderStateHandle shader);
	static Texture*						AccessTexture(TextureHandle texture);
	static Buffer*						AccessBuffer(BufferHandle buffer);
	static Pipeline*					AccessPipeline(PipelineHandle pipeline);
	static Sampler*						AccessSampler(SamplerHandle sampler);
	static DesciptorSetLayout*			AccessDescriptorSetLayout(DescriptorSetLayoutHandle layout);
	static DesciptorSet*				AccessDescriptorSet(DescriptorSetHandle set);
	static RenderPass*					AccessRenderPass(RenderPassHandle renderPass);

	static bool							IsDepthStencil(VkFormat value);
	static bool							IsDepthOnly(VkFormat value);
	static bool							IsStencilOnly(VkFormat value);
	static bool							HasDepth(VkFormat value);
	static bool							HasStencil(VkFormat value);
	static bool							HasDepthOrStencil(VkFormat value);

private:
	// INFO
	NH_HEADER_STATIC CSTR	appName;
	NH_HEADER_STATIC U32	appVersion;

	// DEVICE
	NH_HEADER_STATIC VkAllocationCallbacks*		allocationCallbacks;
	NH_HEADER_STATIC VkInstance					instance;
	NH_HEADER_STATIC VkPhysicalDevice			physicalDevice;
	NH_HEADER_STATIC VkPhysicalDeviceProperties	physicalDeviceProperties;
	NH_HEADER_STATIC VkDevice					device;
	NH_HEADER_STATIC VkQueue					deviceQueue;
	NH_HEADER_STATIC U32						queueFamilyIndex;
	NH_HEADER_STATIC VkDescriptorPool			descriptorPool;
	NH_HEADER_STATIC U64						uboAlignment;
	NH_HEADER_STATIC U64						ssboAlignemnt;

	// WINDOW
	NH_HEADER_STATIC VkSurfaceKHR		surface;
	NH_HEADER_STATIC RenderPassOutput	swapchainOutput;
	NH_HEADER_STATIC VkSurfaceFormatKHR	surfaceFormat;
	NH_HEADER_STATIC VkPresentModeKHR	presentMode;
	NH_HEADER_STATIC VkSwapchainKHR		swapchain;
	NH_HEADER_STATIC U32				swapchainImageCount;
	NH_HEADER_STATIC U16				swapchainWidth;
	NH_HEADER_STATIC U16				swapchainHeight;
	NH_HEADER_STATIC VkImage			swapchainImages[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC VkImageView		swapchainImageViews[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC VkFramebuffer		swapchainFramebuffers[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC TextureHandle      depthTexture;
	NH_HEADER_STATIC U32				imageIndex;
	NH_HEADER_STATIC U32				currentFrame;
	NH_HEADER_STATIC U32				previousFrame;
	NH_HEADER_STATIC U32				absoluteFrame;
	NH_HEADER_STATIC bool				resized{ false };
	NH_HEADER_STATIC bool				verticalSync{ false };

	// RESOURCES
	NH_HEADER_STATIC VmaAllocator_T*						allocator;
	NH_HEADER_STATIC ResourcePool<Buffer, 4096>				buffers;
	NH_HEADER_STATIC ResourcePool<Texture, 512>				textures;
	NH_HEADER_STATIC ResourcePool<Pipeline, 128>			pipelines;
	NH_HEADER_STATIC ResourcePool<Sampler, 32>				samplers;
	NH_HEADER_STATIC ResourcePool<DesciptorSetLayout, 128>	descriptorSetLayouts;
	NH_HEADER_STATIC ResourcePool<DesciptorSet, 256>		descriptorSets;
	NH_HEADER_STATIC ResourcePool<RenderPass, 256>			renderPasses;
	NH_HEADER_STATIC ResourcePool<ShaderState, 128>			shaders;
	NH_HEADER_STATIC Vector<ResourceUpdate>					resourceDeletionQueue;
	NH_HEADER_STATIC Vector<DescriptorSetUpdate>			descriptorSetUpdates;
	NH_HEADER_STATIC Hashmap<U64, VkRenderPass>				renderPassCache{ 16 };
	NH_HEADER_STATIC CommandBuffer**						queuedCommandBuffers;
	NH_HEADER_STATIC CommandBufferRing						commandBufferRing;
	NH_HEADER_STATIC U32									dynamicMaxPerFrameSize;
	NH_HEADER_STATIC BufferHandle							dynamicBuffer;
	NH_HEADER_STATIC U8*									dynamicMappedMemory;
	NH_HEADER_STATIC U32									dynamicAllocatedSize;
	NH_HEADER_STATIC U32									dynamicPerFrameSize;
	NH_HEADER_STATIC U32									numAllocatedCommandBuffers;
	NH_HEADER_STATIC U32									numQueuedCommandBuffers;
	NH_HEADER_STATIC C8										binariesPath[512];
	NH_HEADER_STATIC bool									bindlessSupported{ false };
	// PRIMITIVE
	NH_HEADER_STATIC BufferHandle							fullscreenVertexBuffer;
	NH_HEADER_STATIC RenderPassHandle						swapchainPass;
	NH_HEADER_STATIC SamplerHandle							defaultSampler;
	// DUMMY
	NH_HEADER_STATIC TextureHandle							dummyTexture;
	NH_HEADER_STATIC BufferHandle							dummyConstantBuffer;

	// TIMING
	NH_HEADER_STATIC F32					timestampFrequency;
	NH_HEADER_STATIC GPUTimestampManager*	timestampManager;
	NH_HEADER_STATIC VkQueryPool			timestampQueryPool;
	NH_HEADER_STATIC VkSemaphore			imageAcquired;
	NH_HEADER_STATIC VkSemaphore			renderCompleted[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC VkFence				commandBufferExecuted[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC bool					timestampsEnabled;
	NH_HEADER_STATIC bool					timestampReset = true;

	// DEBUG
	NH_HEADER_STATIC VkDebugUtilsMessengerEXT				debugMessenger;
	NH_HEADER_STATIC PFN_vkCreateDebugUtilsMessengerEXT		vkCreateDebugUtilsMessengerEXT;
	NH_HEADER_STATIC PFN_vkDestroyDebugUtilsMessengerEXT	vkDestroyDebugUtilsMessengerEXT;
	NH_HEADER_STATIC PFN_vkSetDebugUtilsObjectNameEXT		vkSetDebugUtilsObjectNameEXT;
	NH_HEADER_STATIC PFN_vkCmdBeginDebugUtilsLabelEXT		vkCmdBeginDebugUtilsLabelEXT;
	NH_HEADER_STATIC PFN_vkCmdEndDebugUtilsLabelEXT			vkCmdEndDebugUtilsLabelEXT;
	NH_HEADER_STATIC bool									debugUtilsExtensionPresent = false;

	STATIC_CLASS(Renderer);
	friend class Engine;
	friend struct CommandBufferRing;
	friend struct CommandBuffer;
};