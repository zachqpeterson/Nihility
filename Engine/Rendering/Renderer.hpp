#pragma once

#include "Defines.hpp"

import Containers;

#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"
#include "CommandBuffer.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"

struct Scene;
struct VkImage_T;
struct VkQueue_T;
struct VkBuffer_T;
struct VkDevice_T;
struct VkInstance_T;
struct VkSemaphore_T;
struct VkRenderPass_T;
struct VmaAllocator_T;
struct VkCommandBuffer_T;
struct VkPhysicalDevice_T;
struct VkDescriptorPool_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkBufferCopy;
struct VkAllocationCallbacks;
struct VkImageMemoryBarrier2;
struct VkBufferMemoryBarrier2;
struct VkPhysicalDeviceFeatures;
struct VkPhysicalDeviceProperties;
struct VkPhysicalDeviceMemoryProperties;
enum VkFormat;
enum VkObjectType;
enum VkImageLayout;

struct CommandBufferRing
{
	void							Create();
	void							Destroy();

	void							ResetDrawPool();
	void							ResetDraw(U32 frameIndex);
	void							ResetPool(U32 frameIndex);

	CommandBuffer*					GetDrawCommandBuffer(U32 frameIndex);
	CommandBuffer*					GetWriteCommandBuffer(U32 frameIndex);

	static constexpr U16			maxPools = MAX_SWAPCHAIN_IMAGES;
	static constexpr U16			buffersPerPool = 128;
	static constexpr U16			maxBuffers = buffersPerPool * maxPools;

	VkCommandPool_T*				drawCommandPool;
	VkCommandPool_T*				commandPools[maxPools];
	CommandBuffer					drawCommandBuffers[maxPools];
	CommandBuffer					commandBuffers[maxBuffers];
	Freelist						freeCommandBuffers[maxPools];
};

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
	static const VkPhysicalDeviceFeatures&			GetDeviceFeatures();
	static const VkPhysicalDeviceProperties&		GetDeviceProperties();
	static const VkPhysicalDeviceMemoryProperties&	GetDeviceMemoryProperties();

	static void							LoadScene(Scene* scene);
	static ShadowData*					GetShadowData();
	static GlobalData*					GetGlobalData();
	static SkyboxData*					GetSkyboxData();
	static PostProcessData*				GetPostProcessData();

	static const Vector4&				RenderArea();
	static U32							FrameIndex();
	static U32							AbsoluteFrame();

	static VkImageMemoryBarrier2		ImageBarrier(VkImage_T* image, U64 srcStageMask, U64 srcAccessMask,
		VkImageLayout oldLayout, U64 dstStageMask, U64 dstAccessMask,
		VkImageLayout newLayout, U32 aspectMask = 1, U32 baseMipLevel = 0, U32 levelCount = ~0, U32 layerCount = ~0);
	static VkBufferMemoryBarrier2		BufferBarrier(VkBuffer_T* buffer, U64 srcStageMask, U64 srcAccessMask,
		U64 dstStageMask, U64 dstAccessMask);

	static Buffer						CreateBuffer(U64 size, BufferUsageBits bufferUsage, BufferMemoryTypeBits memoryType, const String& name = "buffer");
	static void							DestroyBuffer(Buffer& buffer);
	static void							FillBuffer(Buffer& buffer, U64 size, const void* data, U32 regionCount, VkBufferCopy* regions);
	static void							FillBuffer(Buffer& buffer, const Buffer& stagingBuffer, U32 regionCount, VkBufferCopy* regions);
	static U64							UploadToBuffer(Buffer& buffer, U64 size, const void* data);
	static void							MapBuffer(Buffer& buffer);
	static void							UnmapBuffer(Buffer& buffer);

private:
	static bool							Initialize(CSTR applicationName, U32 applicationVersion);
	static void							Shutdown();

	static bool							CreateInstance();
	static bool							SelectGPU();
	static bool							GetFamilyQueue(VkPhysicalDevice_T* gpu);
	static bool							CreateDevice();
	static bool							CreateResources();

	static void							InitialSubmit();
	static bool							BeginFrame();
	static void							EndFrame();
	static void							SubmitTransfer();
	static VkCommandBuffer_T*			Record();
	static void							Resize();
	static void							SetRenderArea();

	static void							SetResourceName(VkObjectType type, U64 handle, CSTR name);

	static bool							CreateTexture(Texture* texture, void* data);
	static bool							CreateCubemap(Texture* texture, void* data, U32* layerSize);
	static bool							CreateRenderpass(Renderpass* renderpass, const RenderpassInfo& info);
	static bool							RecreateRenderpass(Renderpass* renderpass);

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

	// DEVICE
	static VkInstance_T*						instance;
	static VkPhysicalDevice_T*					physicalDevice;
	static VkDevice_T*							device;
	static Swapchain							swapchain;
	static VkQueue_T*							renderQueue;
	static VkQueue_T*							transferQueue;
	static U32									renderQueueIndex;
	static U32									transferQueueIndex;

	static VkAllocationCallbacks*				allocationCallbacks;
	static VkDescriptorPool_T*					descriptorPools[MAX_SWAPCHAIN_IMAGES];
	static U64									uboAlignment;
	static U64									sboAlignemnt;

	static bool									bindlessSupported;
	static bool									pushDescriptorsSupported;
	static bool									meshShadingSupported;

	// WINDOW
	static Vector4								renderArea;
	static U32									frameIndex;
	static U32									previousFrame;
	static U32									absoluteFrame;
	static bool									resized;

	// RESOURCES
	static Scene*								currentScene;
	static VmaAllocator_T*						allocator;
	static CommandBufferRing					commandBufferRing;
	static Vector<VkCommandBuffer_T*>			commandBuffers[MAX_SWAPCHAIN_IMAGES];
	static Buffer								stagingBuffer;
	static Buffer								materialBuffer;
	static Buffer								globalsBuffer;
	static ShadowData							shadowData;
	static GlobalData							globalData;
	static SkyboxData							skyboxData;
	static PostProcessData						postProcessData;
	static ResourceRef<Texture>					defaultRenderTarget;
	static ResourceRef<Texture>					defaultDepthTarget;

	// SYNCRONIZATION
	static VkSemaphore_T*						imageAcquired;
	static VkSemaphore_T*						presentReady[MAX_SWAPCHAIN_IMAGES];
	static VkSemaphore_T*						renderCompleted[MAX_SWAPCHAIN_IMAGES];
	static VkSemaphore_T*						transferCompleted[MAX_SWAPCHAIN_IMAGES];
	static U64									renderWaitValues[MAX_SWAPCHAIN_IMAGES];
	static U64									transferWaitValues[MAX_SWAPCHAIN_IMAGES];

	// DEBUG
	static VkDebugUtilsMessengerEXT_T*			debugMessenger;

	static bool									debugUtilsExtensionPresent;

	STATIC_CLASS(Renderer);
	friend class Engine;
	friend class Resources;
	friend class UI;
	friend struct CommandBufferRing;
	friend struct CommandBuffer;
	friend struct Swapchain;
	friend struct Rendergraph;
	friend struct Shader;
	friend struct Pipeline;
	friend struct Scene;
};