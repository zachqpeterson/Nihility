#pragma once

#include "RenderingDefines.hpp"

#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "CommandBuffer.hpp"
#include "Swapchain.hpp"

struct Timestamp;
struct TimestampManager;
struct Scene;

class NH_API Renderer
{
public:
	//TODO: Public only for temp testing
	static CommandBuffer*				GetCommandBuffer(QueueType type, bool begin);
	static void*						MapBuffer(const MapBufferParameters& parameters);
	static void							UnmapBuffer(const MapBufferParameters& parameters);

	static void							SetSkybox(Texture* skyboxTexture, Buffer* constantBuffer);

	static U32							GetFrameIndex();

	static void							LoadScene(const String& name);

private:
	static bool							Initialize(CSTR applicationName, U32 applicationVersion);
	static void							Shutdown();

	static bool							CreateInstance();
	static bool							SelectGPU();
	static bool							GetFamilyQueue(VkPhysicalDevice gpu);
	static bool							CreateDevice();
	static bool							CreateResources();
	static bool							CreatePostProcessing();

	static void							BeginFrame();
	static void							EndFrame();
	static void							Resize();

	static void*						DynamicAllocate(U64 size);

	static void							SetResourceName(VkObjectType type, U64 handle, CSTR name);
	static void							PushMarker(VkCommandBuffer commandBuffer, CSTR name);
	static void							PopMarker(VkCommandBuffer commandBuffer);
	static void                         SetGpuTimestampsEnable(bool value);
	static void							PushGpuTimestamp(CommandBuffer* commandBuffer, const String& name);
	static void							PopGpuTimestamp(CommandBuffer* commandBuffer);

	static void							FrameCountersAdvance();
	static void							QueueCommandBuffer(CommandBuffer* commandBuffer);
	static CommandBuffer*				GetInstantCommandBuffer();
	static void							AddImageBarrier(VkCommandBuffer commandBuffer, VkImage image, ResourceType oldState,
		ResourceType newState, U32 baseMipLevel, U32 mipCount, bool isDepth);

	static bool							CreateSampler(Sampler* sampler);
	static bool							CreateTexture(Texture* texture, void* data);
	static bool							CreateBuffer(Buffer* buffer, void* data);
	static bool							CreateDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
	static bool							CreateRenderPass(Renderpass* renderpass);

	static void							DestroySamplerInstant(Sampler* sampler);
	static void							DestroyTextureInstant(Texture* texture);
	static void							DestroyBufferInstant(Buffer* buffer);
	static void							DestroyRenderPassInstant(Renderpass* renderpass);

	static bool							IsDepthStencil(VkFormat value);
	static bool							IsDepthOnly(VkFormat value);
	static bool							IsStencilOnly(VkFormat value);
	static bool							HasDepth(VkFormat value);
	static bool							HasStencil(VkFormat value);
	static bool							HasDepthOrStencil(VkFormat value);

private:
	// INFO
	static CSTR									appName;
	static U32									appVersion;

	// CAPABILITIES
	static VkPhysicalDeviceProperties			physicalDeviceProperties;

	// DEVICE
	static VkInstance							instance;
	static VkPhysicalDevice						physicalDevice;
	static VkDevice								device;
	static VkQueue								deviceQueue;
	static Swapchain							swapchain;
	static U32									queueFamilyIndex;

	static VkAllocationCallbacks*				allocationCallbacks;
	static VkDescriptorPool						descriptorPool;
	static U64									uboAlignment;
	static U64									sboAlignemnt;

	static bool									bindlessSupported;

	// RAY TRACING
	static VkPhysicalDeviceRayTracingPipelineFeaturesKHR	rayTracingPipelineFeatures;
	static VkPhysicalDeviceRayTracingPipelinePropertiesKHR	rayTracingPipelineProperties;
	static VkPhysicalDeviceAccelerationStructureFeaturesKHR	accelerationStructureFeatures;
	static bool												rayTracingPresent;

	// WINDOW
	static RenderpassOutput						swapchainOutput;
	static Renderpass*							skyboxPass;
	static Renderpass*							offscreenPass;
	static Renderpass*							filterPass;
	static Renderpass*							bloomPasses[MAX_BLOOM_PASSES * 2 - 1];
	static U8									bloomPassCount;
	static Pipeline*							skyboxPipeline;
	static Program*								skybox;
	static Program*								offscreen;
	static Program*								postProcessing;
	static Program*								bloom;
	static U32									imageIndex;
	static U32									currentFrame;
	static U32									previousFrame;
	static U32									absoluteFrame;
	static bool									resized;

	// RESOURCES
	static Scene*								currentScene;
	static VmaAllocator_T*						allocator;
	static CommandBufferRing					commandBufferRing;
	static CommandBuffer**						queuedCommandBuffers;
	static U32									allocatedCommandBufferCount;
	static U32									queuedCommandBufferCount;
	static U64									dynamicMaxPerFrameSize;
	static Buffer*								dynamicBuffer;
	static U8*									dynamicMappedMemory;
	static U64									dynamicAllocatedSize;
	static U64									dynamicPerFrameSize;

	// TIMING
	static F32									timestampFrequency;
	static VkQueryPool							timestampQueryPool;
	static VkSemaphore							imageAcquired;
	static VkSemaphore							renderCompleted[MAX_SWAPCHAIN_IMAGES];
	static VkFence								commandBufferExecuted[MAX_SWAPCHAIN_IMAGES];
	static bool									timestampsEnabled;
	static bool									timestampReset;

	// DEBUG
	static VkDebugUtilsMessengerEXT				debugMessenger;
	static PFN_vkCreateDebugUtilsMessengerEXT	vkCreateDebugUtilsMessengerEXT;
	static PFN_vkDestroyDebugUtilsMessengerEXT	vkDestroyDebugUtilsMessengerEXT;
	static PFN_vkSetDebugUtilsObjectNameEXT		vkSetDebugUtilsObjectNameEXT;
	static PFN_vkCmdBeginDebugUtilsLabelEXT		vkCmdBeginDebugUtilsLabelEXT;
	static PFN_vkCmdEndDebugUtilsLabelEXT		vkCmdEndDebugUtilsLabelEXT;
	static bool									debugUtilsExtensionPresent;

	STATIC_CLASS(Renderer);
	friend class Engine;
	friend class Profiler;
	friend class Resources;
	friend struct CommandBufferRing;
	friend struct CommandBuffer;
	friend struct Swapchain;
	friend struct Pipeline;
	friend struct Scene; //TODO: temp
};