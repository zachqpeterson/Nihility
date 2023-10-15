#pragma once

#include "RenderingDefines.hpp"

#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "CommandBuffer.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"

struct Timestamp;
struct TimestampManager;
struct Scene;

//Post Processing		
//TODO: Bloom			
//TODO: Fog				
//TODO: Exposure 		
//TODO: White Balancing	
//TODO: Contrast		✓
//TODO: Brightness		✓
//TODO: Color Filtering	
//TODO: Saturation		✓
//TODO: Tonemapping		✓
//TODO: Gamma			✓
class NH_API Renderer
{
public:
	static void							LoadScene(const String& name);
	static void							SetRenderGraph(RenderGraph* graph);

	static const Vector4&				RenderArea();
	static U32							FrameIndex();
	static U32							CurrentFrame();

private:
	static bool							Initialize(CSTR applicationName, U32 applicationVersion);
	static void							Shutdown();

	static bool							CreateInstance();
	static bool							SelectGPU();
	static bool							GetFamilyQueue(VkPhysicalDevice gpu);
	static bool							CreateDevice();
	static bool							CreateResources();

	static bool							BeginFrame();
	static void							EndFrame();
	static void							Resize();
	static void							SetRenderArea();

	static void							SetResourceName(VkObjectType type, U64 handle, CSTR name);
	static void							PushMarker(VkCommandBuffer commandBuffer, CSTR name);
	static void							PopMarker(VkCommandBuffer commandBuffer);
	static void							FrameCountersAdvance();
	
	static CommandBuffer*				GetCommandBuffer();
	
	static VkImageMemoryBarrier2		ImageBarrier(VkImage image, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, 
		VkImageLayout oldLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, 
		VkImageLayout newLayout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, U32 baseMipLevel = 0, U32 levelCount = VK_REMAINING_MIP_LEVELS, U32 layerCount = VK_REMAINING_ARRAY_LAYERS);
	static VkBufferMemoryBarrier2		BufferBarrier(VkBuffer buffer, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
		VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask);

	static Buffer						CreateBuffer(U32 size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
	static void							FillBuffer(Buffer& buffer, U32 size, const void* data, U32 regionCount, VkBufferCopy* regions);
	static U32							UploadToBuffer(Buffer& buffer, U32 size, const void* data);
	static void							MapBuffer(Buffer& buffer);
	static void							UnmapBuffer(Buffer& buffer);
	static void							DestroyBuffer(Buffer& buffer);

	static bool							CreateDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
	static bool							CreateDescriptorUpdateTemplate(DescriptorSetLayout* descriptorSetLayout, Shader* shader);
	static void							PushDescriptors(CommandBuffer* commandBuffer, Shader* shader);
	static void							PushConstants(CommandBuffer* commandBuffer, Shader* shader);

	static bool							CreateSampler(Sampler* sampler);
	static bool							CreateTexture(Texture* texture, void* data);
	static bool							CreateCubemap(Texture* texture, void* data, U32* layerSize);
	static bool							CreateRenderpass(Renderpass* renderpass);

	static void							DestroySamplerInstant(Sampler* sampler);
	static void							DestroyTextureInstant(Texture* texture);
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
	static VkPhysicalDeviceFeatures				physicalDeviceFeatures;
	static VkPhysicalDeviceProperties			physicalDeviceProperties;
	static VkPhysicalDeviceMemoryProperties		physicalDeviceMemoryProperties;

	// DEVICE
	static VkInstance							instance;
	static VkPhysicalDevice						physicalDevice;
	static VkDevice								device;
	static VkQueue								deviceQueue;
	static Swapchain							swapchain;
	static U32									queueFamilyIndex;
	static PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR;

	static VkAllocationCallbacks*				allocationCallbacks;
	static VkDescriptorPool						descriptorPool;
	static U64									uboAlignment;
	static U64									sboAlignemnt;

	static bool									bindlessSupported;
	static bool									pushDescriptorsSupported;
	static bool									meshShadingSupported;

	// WINDOW
	static Vector4								renderArea;
	static U32									frameIndex;
	static U32									currentFrame;
	static U32									previousFrame;
	static U32									absoluteFrame;
	static bool									resized;

	// RESOURCES
	static Scene*								currentScene;
	static VmaAllocator_T*						allocator;
	static CommandBufferRing					commandBufferRing;
	static Buffer								stagingBuffer;
	static Buffer								materialBuffer;
	static CameraData							cameraData;
	static PostProcessData						postProcessData;
	static RenderGraph*							renderGraph;

	// SYNCRONIZATION
	static VkSemaphore							imageAcquired;
	static VkSemaphore							queueSubmitted;

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
	friend class UI;
	friend struct CommandBufferRing;
	friend struct CommandBuffer;
	friend struct Swapchain;
	friend struct Renderpass;
	friend struct Shader;
	friend struct Pipeline;
	friend struct Scene;
};