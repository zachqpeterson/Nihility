#pragma once

#include "Defines.hpp"

#include "Instance.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "CommandBuffer.hpp"
#include "Buffer.hpp"
#include "Renderpass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "DescriptorSet.hpp"
#include "Shader.hpp"
#include "FrameBuffer.hpp"
#include "Camera.hpp"

#include "Resources/ResourceDefines.hpp"
#include "Resources/Texture.hpp"

struct VmaAllocator_T;
struct VmaAllocation_T;

class Renderer
{
public:

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static bool InitializeVma();
	static bool GetQueues();
	static bool CreateDepthBuffer();
	static bool CreateDescriptorPool();
	static bool CreateRenderpasses();
	static bool CreateSynchronization();

	static bool RecreateSwapchain();

	static VkCommandPool CreateCommandPool(QueueType queueType);
	static void DestroyCommandPool(VkCommandPool pool);

	static bool UploadTexture(Resource<Texture>& texture, U8* data, const Sampler& sampler);
	static void DestroyTexture(Resource<Texture>& texture);

	static VmaAllocator_T* vmaAllocator;
	static VkAllocationCallbacks* allocationCallbacks;
	static Instance instance;
	static Device device;
	static VkQueue graphicsQueue;
	static VkQueue presentQueue;
	static Swapchain swapchain;
	static VkCommandPool commandPool;
	static CommandBuffer commandBuffer;
	static VkDescriptorPool vkDescriptorPool;
	static VkDescriptorPool vkBindlessDescriptorPool;
	static DescriptorSet descriptorSet;
	static Renderpass renderpass;
	static FrameBuffer frameBuffer;
	static VkSemaphore presentSemaphore;
	static VkSemaphore renderSemaphore;
	static VkFence renderFence;

	static VkFormat depthFormat;
	static VkImage depthBuffer;
	static VkImageView depthBufferView;
	static VmaAllocation_T* depthBufferAllocation;

	static Camera camera;

	friend class Engine;
	friend class Resources;
	friend struct Instance;
	friend struct PhysicalDevice;
	friend struct Device;
	friend struct Swapchain;
	friend struct CommandBuffer;
	friend struct Buffer;
	friend struct Renderpass;
	friend struct PipelineLayout;
	friend struct Pipeline;
	friend struct Shader;
	friend struct FrameBuffer;
	friend struct DescriptorSet;

	STATIC_CLASS(Renderer);
};