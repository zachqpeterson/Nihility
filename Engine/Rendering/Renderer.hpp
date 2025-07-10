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
#include "Resources/World.hpp"
#include "Containers/String.hpp"

struct VmaAllocator_T;
struct VmaAllocation_T;
struct VkDescriptorPool_T;
struct VkCommandBuffer_T;
struct VkSemaphore_T;
struct VkAllocationCallbacks;

class NH_API Renderer
{
public:
	static U32 FrameIndex();
	static U32 PreviousFrame();
	static U32 AbsoluteFrame();

	static Vector4Int RenderSize();

	static const GlobalPushConstant* GetGlobalPushConstant();
	static VkSemaphore_T* RenderFinished();
	static const Device& GetDevice();

private:
	static bool Initialize(const StringView& name, U32 version);
	static void Shutdown();

	static void Update();
	static bool Synchronize();
	static void SubmitTransfer();
	static void Submit();

	static bool InitializeVma();
	static bool CreateColorTextures();
	static bool CreateDepthTextures();
	static bool CreateDescriptorPool();
	static bool CreateRenderpasses();
	static bool CreateSynchronization();
	static bool CreateStagingBuffers();

	static bool RecreateSwapchain();

	static bool UploadTexture(Resource<Texture>& texture, void* data, const Sampler& sampler);
	static void DestroyTexture(Resource<Texture>& texture);

	//Resources
	static VmaAllocator_T* vmaAllocator;
	static VkAllocationCallbacks* allocationCallbacks;
	static VkDescriptorPool_T* vkDescriptorPool;
	static VkDescriptorPool_T* vkBindlessDescriptorPool;
	static DescriptorSet descriptorSet;
	static Texture colorTextures[MaxSwapchainImages];
	static Texture depthTextures[MaxSwapchainImages];
	static Buffer stagingBuffers[MaxSwapchainImages];

	//Vulkan Objects
	static Instance instance;
	static Device device;
	static Swapchain swapchain;
	static Renderpass renderpass;
	static FrameBuffer frameBuffer;

	//Recording
	static Vector<VkCommandBuffer_T*> commandBuffers[MaxSwapchainImages];
	static GlobalPushConstant globalPushConstant;

	//Synchronization
	static bool resize;
	static U32 frameIndex;
	static U32 previousFrame;
	static U32 absoluteFrame;
	static VkSemaphore_T* imageAcquired[MaxSwapchainImages];
	static VkSemaphore_T* transferFinished[MaxSwapchainImages];
	static VkSemaphore_T* renderFinished[MaxSwapchainImages];
	static VkSemaphore_T* presentReady[MaxSwapchainImages];
	static U64 renderWaitValues[MaxSwapchainImages];
	static U64 transferWaitValues[MaxSwapchainImages];

	friend class Engine;
	friend class Resources;
	friend class CommandBufferRing;
	friend class World;
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

	STATIC_CLASS(Renderer);
};