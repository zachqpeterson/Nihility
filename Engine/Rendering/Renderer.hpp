#pragma once

#include "RenderingDefines.hpp"

#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "CommandBuffer.hpp"

struct Timestamp;
struct TimestampManager;

class NH_API Renderer
{
public:
	//TODO: Public only for temp testing
	static const RenderPassOutput& GetSwapchainOutput();
	static RenderPass* GetSwapchainPass();
	static CommandBuffer* GetCommandBuffer(QueueType type, bool begin);
	static void* MapBuffer(const MapBufferParameters& parameters);
	static void							UnmapBuffer(const MapBufferParameters& parameters);

private:
	static bool							Initialize(CSTR applicationName, U32 applicationVersion);
	static void							Shutdown();

	static bool							CreateInstance();
	static bool							CreateSurface();
	static bool							SelectGPU();
	static bool							GetFamilyQueue(VkPhysicalDevice gpu);
	static bool							CreateDevice();
	static bool							SetFormats();
	static bool							CreateSwapchain();
	static bool							CreateResources();
	static bool							CreatePrimitiveResources();

	static void							BeginFrame();
	static void							EndFrame();
	static void							Resize();
	static void							ResizeSwapchain();
	static void							DestroySwapchain();

	
	static void* DynamicAllocate(U32 size);

	static void							SetResourceName(VkObjectType type, U64 handle, CSTR name);
	static void							PushMarker(VkCommandBuffer commandBuffer, CSTR name);
	static void							PopMarker(VkCommandBuffer commandBuffer);
	static void                         SetGpuTimestampsEnable(bool value);
	static void							PushGpuTimestamp(CommandBuffer* commandBuffer, const String& name);
	static void							PopGpuTimestamp(CommandBuffer* commandBuffer);

	static void							FrameCountersAdvance();
	static void							QueueCommandBuffer(CommandBuffer* commandBuffer);
	static CommandBuffer* GetInstantCommandBuffer();
	static void							TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth);
	static void							FillWriteDescriptorSets(const DescriptorSetLayout* descriptorSetLayout, VkDescriptorSet vkDescriptorSet,
		VkWriteDescriptorSet* descriptorWrite, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo,
		VkSampler vkDefaultSampler, U32& numResources, const void** resources, const Sampler** samplers, const U16* bindings);
	static VkShaderModuleCreateInfo		CompileShader(CSTR path, VkShaderStageFlagBits stage, CSTR name);
	static VkRenderPass					CreateVulkanRenderPass(const RenderPassOutput& output, CSTR name);
	static VkRenderPass					GetRenderPass(const RenderPassOutput& output, CSTR name);
	static void							CreateSwapchainPass(RenderPass* renderPass);
	static void							CreateFramebuffer(RenderPass* renderPass);

	static bool							CreateSampler(Sampler* sampler);
	static bool							CreateTexture(Texture* texture, void* data);
	static bool							CreateBuffer(Buffer* buffer, void* data);
	static bool							CreateDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
	static bool							CreateDescriptorSet(DescriptorSet* descriptorSet);
	static bool							CreateShaderState(ShaderState* shaderState, const ShaderStateCreation& info);
	static bool							CreateRenderPass(RenderPass* renderPass, RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);
	static bool							CreatePipeline(Pipeline* pipeline, const RenderPassOutput& renderPass, const VertexInputCreation& vertexInput);

	static void							DestroySamplerInstant(Sampler* sampler);
	static void							DestroyTextureInstant(Texture* texture);
	static void							DestroyBufferInstant(Buffer* buffer);
	static void							DestroyDescriptorSetLayoutInstant(DescriptorSetLayout* layout);
	static void							DestroyDescriptorSetInstant(DescriptorSet* set);
	static void							DestroyShaderStateInstant(ShaderState* shader);
	static void							DestroyRenderPassInstant(RenderPass* renderPass);
	static void							DestroyPipelineInstant(Pipeline* pipeline);

	static void							UpdateDescriptorSet(DescriptorSet* descriptorSet);
	static void							UpdateDescriptorSetInstant(const DescriptorSetUpdate& update);
	static void							ResizeOutputTextures(RenderPass* renderPass, U32 width, U32 height);

	static bool							IsDepthStencil(VkFormat value);
	static bool							IsDepthOnly(VkFormat value);
	static bool							IsStencilOnly(VkFormat value);
	static bool							HasDepth(VkFormat value);
	static bool							HasStencil(VkFormat value);
	static bool							HasDepthOrStencil(VkFormat value);

private:
	// INFO
	NH_HEADER_STATIC CSTR									appName;
	NH_HEADER_STATIC U32									appVersion;

	// DEVICE
	NH_HEADER_STATIC VkAllocationCallbacks*					allocationCallbacks;
	NH_HEADER_STATIC VkInstance								instance;
	NH_HEADER_STATIC VkPhysicalDevice						physicalDevice;
	NH_HEADER_STATIC VkPhysicalDeviceProperties				physicalDeviceProperties;
	NH_HEADER_STATIC VkDevice								device;
	NH_HEADER_STATIC VkQueue								deviceQueue;
	NH_HEADER_STATIC U32									queueFamilyIndex;
	NH_HEADER_STATIC VkDescriptorPool						descriptorPool;
	NH_HEADER_STATIC U64									uboAlignment;
	NH_HEADER_STATIC U64									ssboAlignemnt;

	// WINDOW
	NH_HEADER_STATIC VkSurfaceKHR							surface;
	NH_HEADER_STATIC RenderPassOutput						swapchainOutput;
	NH_HEADER_STATIC VkSurfaceFormatKHR						surfaceFormat;
	NH_HEADER_STATIC VkPresentModeKHR						presentMode;
	NH_HEADER_STATIC VkSwapchainKHR							swapchain;
	NH_HEADER_STATIC U32									swapchainImageCount;
	NH_HEADER_STATIC U16									swapchainWidth;
	NH_HEADER_STATIC U16									swapchainHeight;
	NH_HEADER_STATIC VkImage								swapchainImages[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC VkImageView							swapchainImageViews[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC VkFramebuffer							swapchainFramebuffers[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC Texture*								depthTexture;
	NH_HEADER_STATIC U32									imageIndex{ 0 };
	NH_HEADER_STATIC U32									currentFrame{ 1 };
	NH_HEADER_STATIC U32									previousFrame{ 0 };
	NH_HEADER_STATIC U32									absoluteFrame{ 0 };
	NH_HEADER_STATIC bool									resized{ false };
	NH_HEADER_STATIC bool									verticalSync{ false };

	// RESOURCES
	NH_HEADER_STATIC VmaAllocator_T*						allocator;
	NH_HEADER_STATIC Queue<DescriptorSetUpdate>				descriptorSetUpdates;
	NH_HEADER_STATIC Hashmap<U64, VkRenderPass>				renderPassCache{ 16 };
	NH_HEADER_STATIC CommandBufferRing						commandBufferRing;
	NH_HEADER_STATIC CommandBuffer**						queuedCommandBuffers;
	NH_HEADER_STATIC U32									allocatedCommandBufferCount{ 0 };
	NH_HEADER_STATIC U32									queuedCommandBufferCount{ 0 };
	NH_HEADER_STATIC U32									dynamicMaxPerFrameSize;
	NH_HEADER_STATIC Buffer*								dynamicBuffer;
	NH_HEADER_STATIC U8*									dynamicMappedMemory;
	NH_HEADER_STATIC U32									dynamicAllocatedSize;
	NH_HEADER_STATIC U32									dynamicPerFrameSize;
	NH_HEADER_STATIC C8										binariesPath[512];
	NH_HEADER_STATIC bool									bindlessSupported{ false };
	// PRIMITIVE
	NH_HEADER_STATIC Buffer*								fullscreenVertexBuffer;
	NH_HEADER_STATIC RenderPass*							swapchainPass;
	NH_HEADER_STATIC Sampler*								defaultSampler;  //TODO: Move to Resources
	// DUMMY
	NH_HEADER_STATIC Buffer*								dummyConstantBuffer; //TODO: Move to Resources

	// TIMING
	NH_HEADER_STATIC F32									timestampFrequency;
	NH_HEADER_STATIC VkQueryPool							timestampQueryPool;
	NH_HEADER_STATIC VkSemaphore							imageAcquired;
	NH_HEADER_STATIC VkSemaphore							renderCompleted[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC VkFence								commandBufferExecuted[MAX_SWAPCHAIN_IMAGES];
	NH_HEADER_STATIC bool									timestampsEnabled{ false };
	NH_HEADER_STATIC bool									timestampReset{ true };

	// DEBUG
	NH_HEADER_STATIC VkDebugUtilsMessengerEXT				debugMessenger;
	NH_HEADER_STATIC PFN_vkCreateDebugUtilsMessengerEXT		vkCreateDebugUtilsMessengerEXT;
	NH_HEADER_STATIC PFN_vkDestroyDebugUtilsMessengerEXT	vkDestroyDebugUtilsMessengerEXT;
	NH_HEADER_STATIC PFN_vkSetDebugUtilsObjectNameEXT		vkSetDebugUtilsObjectNameEXT;
	NH_HEADER_STATIC PFN_vkCmdBeginDebugUtilsLabelEXT		vkCmdBeginDebugUtilsLabelEXT;
	NH_HEADER_STATIC PFN_vkCmdEndDebugUtilsLabelEXT			vkCmdEndDebugUtilsLabelEXT;
	NH_HEADER_STATIC bool									debugUtilsExtensionPresent{ false };

	STATIC_CLASS(Renderer);
	friend class Engine;
	friend class Profiler;
	friend class Resources;
	friend struct CommandBufferRing;
	friend struct CommandBuffer;
};