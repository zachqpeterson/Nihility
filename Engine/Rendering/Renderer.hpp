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

class NH_API Renderer
{
public:
	static void							LoadScene(const String& name);
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
	static void							Render(CommandBuffer* commandBuffer, Pipeline* pipeline, U32 drawCount, bool late);
	static void							Cull(CommandBuffer* commandBuffer, Pipeline* pipeline, U32 drawCount, bool late);
	static void							GenerateDepthPyramid(CommandBuffer* commandBuffer);
	static void							EndFrame();
	static void							Resize();

	static void							SetResourceName(VkObjectType type, U64 handle, CSTR name);
	static void							PushMarker(VkCommandBuffer commandBuffer, CSTR name);
	static void							PopMarker(VkCommandBuffer commandBuffer);
	static void							FrameCountersAdvance();
	
	static CommandBuffer*				GetCommandBuffer(bool begin);
	
	static VkImageMemoryBarrier2		ImageBarrier(VkImage image, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkImageLayout oldLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkImageLayout newLayout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, U32 baseMipLevel = 0, U32 levelCount = VK_REMAINING_MIP_LEVELS);
	static VkBufferMemoryBarrier2		BufferBarrier(VkBuffer buffer, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask);
	static void							TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	static Buffer						CreateBuffer(U64 size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
	static void							FillBuffer(Buffer& buffer, const void* data, U64 size, U64 offset);
	static U64							UploadToBuffer(Buffer& buffer, const void* data, U64 size);
	static void							MapBuffer(Buffer& buffer);
	static void							UnmapBuffer(Buffer& buffer);
	static void							DestroyBuffer(Buffer& buffer);

	static bool							CreateDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
	static bool							CreateDescriptorUpdateTemplate(DescriptorSetLayout* descriptorSetLayout, Shader* shader);
	static void							PushDescriptors(Shader* shader, const Descriptor* descriptors);

	static bool							CreateSampler(Sampler* sampler);
	static bool							CreateTexture(Texture* texture, void* data);
	static bool							CreateCubeMap(Texture* texture, void* data, U32* layerSize);
	static bool							CreateRenderPass(Renderpass* renderpass, bool swapchain = false);

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
	static VkPhysicalDeviceProperties			physicalDeviceProperties;
	static VkPhysicalDeviceMemoryProperties		physicalDeviceMemoryProperties;

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
	static bool									pushDescriptorsSupported;
	static bool									meshShadingSupported;

	// WINDOW
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
	static Buffer								vertexBuffer;
	static Buffer								indexBuffer;
	static Buffer								meshBuffer;
	static Buffer								drawsBuffer;
	static Buffer								drawCommandsBuffer;
	static Buffer								drawCountsBuffer;
	static Buffer								drawVisibilityBuffer;
	static Texture*								depthPyramid;

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
	friend struct CommandBufferRing;
	friend struct CommandBuffer;
	friend struct Swapchain;
	friend struct Renderpass;
	friend struct Shader;
	friend struct Pipeline;
	friend struct Scene; //TODO: temp
};