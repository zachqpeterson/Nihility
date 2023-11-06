#pragma once

#include "Defines.hpp"

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
struct VmaAllocator_T;
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

struct CameraData
{
	Matrix4 vp;
	Vector4 eye;
};

struct PostProcessData
{
	F32 contrast{ 1.0f };
	F32 brightness{ 0.0f };
	F32 saturation{ 1.0f };
	F32 gammaCorrection{ 1.0f };
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
	static void							LoadScene(Scene* scene);
	static void							SetRenderGraph(PipelineGraph* graph);
	static CameraData*					GetCameraData();

	static const Vector4&				RenderArea();
	static U32							FrameIndex();
	static U32							CurrentFrame();
	static U32							AbsoluteFrame();

	static const VkPhysicalDeviceFeatures&			GetDeviceFeatures();
	static const VkPhysicalDeviceProperties&		GetDeviceProperties();
	static const VkPhysicalDeviceMemoryProperties&	GetDeviceMemoryProperties();

	static VkImageMemoryBarrier2		ImageBarrier(VkImage_T* image, U64 srcStageMask, U64 srcAccessMask,
		VkImageLayout oldLayout, U64 dstStageMask, U64 dstAccessMask,
		VkImageLayout newLayout, U32 aspectMask = 1, U32 baseMipLevel = 0, U32 levelCount = ~0, U32 layerCount = ~0);
	static VkBufferMemoryBarrier2		BufferBarrier(VkBuffer_T* buffer, U64 srcStageMask, U64 srcAccessMask,
		U64 dstStageMask, U64 dstAccessMask);

private:
	static bool							Initialize(CSTR applicationName, U32 applicationVersion);
	static void							Shutdown();

	static bool							CreateInstance();
	static bool							SelectGPU();
	static bool							GetFamilyQueue(VkPhysicalDevice_T* gpu);
	static bool							CreateDevice();
	static bool							CreateResources();

	static bool							BeginFrame();
	static void							EndFrame();
	static void							Resize();
	static void							SetRenderArea();

	static void							SetResourceName(VkObjectType type, U64 handle, CSTR name);
	static void							FrameCountersAdvance();
	
	static CommandBuffer*				GetCommandBuffer();

	static Buffer						CreateBuffer(U32 size, U32 usageFlags, U32 memoryFlags);
	static void							FillBuffer(Buffer& buffer, U32 size, const void* data, U32 regionCount, VkBufferCopy* regions);
	static U32							UploadToBuffer(Buffer& buffer, U32 size, const void* data);
	static void							MapBuffer(Buffer& buffer);
	static void							UnmapBuffer(Buffer& buffer);
	static void							DestroyBuffer(Buffer& buffer);

	static void							PushConstants(CommandBuffer* commandBuffer, Shader* shader);

	static bool							CreateTexture(Texture* texture, void* data);
	static bool							CreateCubemap(Texture* texture, void* data, U32* layerSize);
	static bool							CreateRenderpass(Renderpass* renderpass);
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
	static VkQueue_T*							deviceQueue;
	static Swapchain							swapchain;
	static U32									queueFamilyIndex;

	static VkAllocationCallbacks*				allocationCallbacks;
	static VkDescriptorPool_T*					descriptorPool;
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
	static PipelineGraph*						pipelineGraph;

	// SYNCRONIZATION
	static VkSemaphore_T*						imageAcquired;
	static VkSemaphore_T*						queueSubmitted;

	// DEBUG
	static VkDebugUtilsMessengerEXT_T*			debugMessenger;

	static bool									debugUtilsExtensionPresent;

#ifdef NH_DEBUG
	static FlyCamera							flyCamera;
#endif

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