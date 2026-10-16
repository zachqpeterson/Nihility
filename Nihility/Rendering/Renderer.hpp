#pragma once

#include "RenderingDefines.hpp"

#include "RenderTarget.hpp"
#include "FrameArena.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"

#include "Core/Containers.hpp"
#include "Core/Function.hpp"
#include "Resources/Texture.hpp"
#include "Components/Registry.hpp"

struct SpriteData;

struct RenderPacket
{
	RenderPacket(U64 arenaSize, U64 maxSpriteCapacity);

	void PushSprite(const SpriteData& instance);
	void Clear();
	std::span<const SpriteData> GetActiveSprites() const;

	FrameArena arena;

	SpriteData* sprites;
	std::atomic<U64> spriteCount;
	U64 maxSprites;
};

struct RenderExtractor
{
public:
	RenderExtractor();

	void BeginExtraction();
	void SubmitSprite(const SpriteData& sprite);
	void FinishExtraction();

	RenderPacket& GetWritePacket();
	const RenderPacket& GetReadPacket() const;

private:
	Array<RenderPacket, 2> bufferedPackets;

	U32 writeIndex = 0;
	U32 readIndex = 1;
};

struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkAllocationCallbacks;

struct WorkerCommandContext
{
	VkCommandPool_T* graphicsPools[MaxFramesInFlight];
	Vector<VkCommandBuffer_T*> commandBuffers[MaxFramesInFlight];

	VkCommandPool_T* transferPool;

	U32 currentBufferIndex = 0;

	void Create(Device& device, const VkAllocationCallbacks* allocator, U32 graphicsQueueIndex, U32 transferQueueIndex, U32 preallocateCount);
	void Destroy(Device& device, const VkAllocationCallbacks* allocator);

	void ResetFrame(Device& device, U32 frameIndex);

	VkCommandBuffer_T* GetNextCommandBuffer(U32 frameIndex);
};

struct PointLightGPU
{
	glm::vec2 position;
	F32 radius;
	F32 intensity;
	glm::vec4 color;
};

enum VkFormat;
enum VkImageLayout;
enum VmaMemoryUsage;
struct VmaAllocation_T;
struct VmaAllocator_T;
struct VkImage_T;
struct VkBuffer_T;
struct VkPipeline_T;
struct VkSemaphore_T;
struct VkDescriptorSet_T;
struct VkPipelineLayout_T;
struct VkDescriptorPool_T;
struct VkDescriptorSetLayout_T;

class NH_API Renderer
{
public:
	static Buffer CreateBuffer(U64 size, U64 usage, VmaMemoryUsage memoryUsage);
	static void DestroyBuffer(Buffer& buffer);
	static void UploadToBuffer(Buffer& dstBuffer, const void* data, U64 size, U32 threadId = 0);

	static VkDescriptorSet_T* GlobalBindlessSet();
	static U32 FrameIndex();

	static void SubmitSprite(const SpriteData& sprite);

	static void SetActiveCamera(Entity camera);
	static glm::mat4 GetViewProjectionMatrix();
	static glm::mat4 GetGIOverscanMatrix();
	static glm::vec4 RenderArea();

	static void NameAllocation(VmaAllocation_T* allocation, const WString& name);
	static void NameAllocation(VmaAllocation_T* allocation, const String& name);

	static constexpr U32 TargetRenderWidth = 1920;
	static constexpr U32 TargetRenderHeight = 1080;
	static constexpr U32 Overscan = 512;
	static constexpr U32 OverscanWidth = TargetRenderWidth + Overscan * 2;
	static constexpr U32 OverscanHeight = TargetRenderHeight + Overscan * 2;
	static constexpr U32 RadianceCascadeWidth = OverscanWidth / 2;
	static constexpr U32 RadianceCascadeHeight = OverscanHeight / 2;
	static constexpr U32 BaseRayCount = 16;

private:
	static bool Initialize(const StringView& name, U32 version);
	static void Stop();
	static void Shutdown();

	static bool BeginFrame();
	static void EndFrame();
	static void UpdateLightBuffer();
	static void RunHRC(VkCommandBuffer_T* cmd);

	static bool InitializeVma();
	static bool CreateSurfaceInfo();
	static bool CreateDescriptorSet();
	static bool CreateShaders();
	static bool CreateStorageBuffers();
	static bool CreateSynchronization();
	static bool CreateCommandBuffers();

	static void DestroyTexture(Texture& texture);

	static void DeferDestruction(Function<void()>&& function);
	static void FlushDeletionQueue();

	static void RecreateSwapchain();
	static void ComputeMemoryBarrier(VkCommandBuffer_T* cmd);
	static void TransitionImageLayout(VkCommandBuffer_T* cmd, VkImage_T* image, VkImageLayout oldLayout, VkImageLayout newLayout,
		U64 srcStageMask, U64 srcAccessMask, U64 dstStageMask, U64 dstAccessMask);
	static VkCommandBuffer_T* RecordSecondaryCommandBuffer(U32 threadId, U32 frameIndex, std::span<const SpriteData> sprites);
	static VkCommandBuffer_T* BeginSingleTimeCommands(VkCommandPool_T* commandPool);
	static void EndSingleTimeCommands(VkCommandPool_T* commandPool, VkCommandBuffer_T* commandBuffer);
	static bool UploadTexture(std::shared_ptr<Texture> texture, void* data, U32 threadId);

	static Instance instance;
	static Device device;
	static Swapchain swapchain;
	static Shader lightInjectionShader;
	static Shader spriteShader;
	static Shader jfaInitShader;
	static Shader jfaStepShader;
	static Shader radianceCascadeShader;
	static Shader compositeShader;
	static Shader vBlurShader;
	static Shader hBlurShader;

	static VkDescriptorSetLayout_T* globalBindlessSetLayout;
	static VkDescriptorPool_T* globalDescriptorPool;
	static VkDescriptorSet_T* globalBindlessSet;

	static VkDescriptorSetLayout_T* ssboSetLayout;
	static VkDescriptorSet_T* ssboDescriptorSets[MaxFramesInFlight];

	static VkBuffer_T* spriteBuffers[MaxFramesInFlight];
	static VmaAllocation_T* spriteAllocations[MaxFramesInFlight];
	static void* spriteMappedData[MaxFramesInFlight];

	static Buffer lightStorageBuffer;
	static Vector<PointLightGPU> lightDataCache;

	static VkCommandPool_T* primaryPools[MaxFramesInFlight];
	static VkCommandBuffer_T* primaryBuffers[MaxFramesInFlight];

	static RenderTarget collisionMaskTarget;
	static RenderTarget emissiveTarget;
	static RenderTarget jfaBufferA;
	static RenderTarget jfaBufferB;
	static RenderTarget rcAtlas;
	static RenderTarget radianceCascadeTarget;
	static RenderTarget radianceCascadeBlurTarget;
	static RenderTarget albedoTarget;
	static RenderTarget* finalSDF;
	static Sampler nearSampler;
	static Sampler linearSampler;

	static U32 surfaceFormat;
	static U32 surfaceColorSpace;
	static U32 imageCount;
	static U32 presentMode;
	static U32 surfaceWidth;
	static U32 surfaceHeight;

	static U32 imageIndex;
	static U64 currentFrameNumber;
	static VkSemaphore_T* frameTimelineSemaphore;
	static VkSemaphore_T* imageAcquiredSemaphores[MaxFramesInFlight];
	static VkSemaphore_T* renderFinishedSemaphores[MaxSwapchainImages];

	static U32 workerThreadCount;
	static Vector<WorkerCommandContext> workerContexts;
	static RenderExtractor extractor;

	static VmaAllocator_T* vmaAllocator;
	static VkAllocationCallbacks* allocationCallbacks;

	static Vector<Function<void()>> pendingDeletions;
	static Vector<Function<void()>> deletionQueues[MaxFramesInFlight];

	static Entity activeCamera;

#ifdef NH_DEBUG
	static RenderTarget viewportTarget;
#endif

	friend class Nihility;
	friend class Resources;
	friend class Tilemap;
	friend class Editor;
	friend struct RenderTarget;
	friend struct Swapchain;
	friend struct Instance;
	friend struct Sampler;
	friend struct Device;
	friend struct Shader;

	STATIC_CLASS(Renderer);
};