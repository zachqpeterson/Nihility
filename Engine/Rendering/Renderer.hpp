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
#include "Resources/Scene.hpp"

struct VmaAllocator_T;
struct VmaAllocation_T;

class Renderer
{
public:
	static void SetScene(Scene* scene);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();
	static void Synchronize();
	static void SubmitTransfer();
	static void Submit();

	static bool InitializeVma();
	static bool GetQueues();
	static bool CreateDepthTextures();
	static bool CreateDescriptorPool();
	static bool CreateRenderpasses();
	static bool CreateSynchronization();
	static bool CreateStagingBuffers();

	static bool RecreateSwapchain();

	static bool UploadTexture(Resource<Texture>& texture, U8* data, const Sampler& sampler);
	static void DestroyTexture(Resource<Texture>& texture);

	//Resources
	static VmaAllocator_T* vmaAllocator;
	static VkAllocationCallbacks* allocationCallbacks;
	static VkDescriptorPool vkDescriptorPool;
	static VkDescriptorPool vkBindlessDescriptorPool;
	static DescriptorSet descriptorSet;
	static Texture depthTextures[MaxSwapchainImages];
	static Buffer stagingBuffers[MaxSwapchainImages];

	//Vulkan Objects
	static Instance instance;
	static Device device;
	static VkQueue graphicsQueue;
	static VkQueue presentQueue;
	static Swapchain swapchain;
	static Renderpass renderpass;
	static FrameBuffer frameBuffer;

	//Recording
	static Vector<VkCommandBuffer> commandBuffers[MaxSwapchainImages];
	static GlobalPushConstant globalPushConstant;
	static Scene* scene;

	//Synchronization
	static U32 frameIndex;
	static U32 previousFrame;
	static U32 absoluteFrame;
	static VkSemaphore imageAcquired[MaxSwapchainImages];
	static VkSemaphore transferFinished[MaxSwapchainImages];
	static VkSemaphore renderFinished[MaxSwapchainImages];
	static VkSemaphore presentReady[MaxSwapchainImages];
	static U64 renderWaitValues[MaxSwapchainImages];
	static U64 transferWaitValues[MaxSwapchainImages];

	friend class Engine;
	friend class Resources;
	friend class CommandBufferRing;
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
	friend struct Material;
	friend struct Scene;

	STATIC_CLASS(Renderer);
};