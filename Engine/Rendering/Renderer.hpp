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
#include "Shader.hpp"
#include "FrameBuffer.hpp"
#include "Camera.hpp"

#include "Resources/ResourceDefines.hpp"
#include "Resources/Model.hpp"
#include "Resources/ModelInstance.hpp"

struct VmaAllocator_T;
struct VmaAllocation_T;

class Renderer
{
public:
	//TODO: Scene Object
	static void AddModelInstance(ModelInstance& instance);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();
	static void Draw(ResourceRef<Model> model);
	static void DrawInstanced(ResourceRef<Model> model, U32 instanceCount);

	static bool InitializeVma();
	static bool GetQueues();
	static bool CreateDepthBuffer();
	static bool CreateDescriptorPool();
	static bool CreateDescriptorSetLayouts();
	static bool CreateDescriptorSets();
	static bool CreateRenderpasses();
	static bool CreatePipelineLayouts();
	static bool CreatePipelines();
	static bool CreateSynchronization();

	static bool RecreateSwapchain();
	static bool UpdateDescriptorSets();

	static VkCommandPool CreateCommandPool(QueueType queueType);
	static void DestroyCommandPool(VkCommandPool pool);

	static bool UploadTexture(Resource<Texture>& texture, U8* data);
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
	static Buffer perspectiveViewMatrixUBO;
	static Buffer worldPosBuffer;
	static Vector<Matrix4> worldPosMatrices;
	static Buffer boneMatrixBuffer;
	static Vector<Matrix4> modelBoneMatrices;
	static VkDescriptorPool vkDescriptorPool;
	static VkDescriptorSetLayout assimpDescriptorLayout;
	static VkDescriptorSetLayout assimpTextureDescriptorLayout;
	static VkDescriptorSet assimpDescriptorSet;
	static VkDescriptorSet assimpSkinningDescriptorSet;
	static Renderpass renderpass;
	static PipelineLayout assimpPipelineLayout;
	static PipelineLayout assimpSkinningPipelineLayout;
	static Pipeline assimpPipeline;
	static Pipeline assimpSkinningPipeline;
	static Shader assimpVertShader;
	static Shader assimpFragShader;
	static Shader assimpSkinningVertShader;
	static Shader assimpSkinningFragShader;
	static FrameBuffer frameBuffer;
	static VkSemaphore presentSemaphore;
	static VkSemaphore renderSemaphore;
	static VkFence renderFence;

	static VkFormat depthFormat;
	static VkImage depthBuffer;
	static VkImageView depthBufferView;
	static VmaAllocation_T* depthBufferAllocation;

	static Camera camera;

	static Vector<Vector<ModelInstance>> instances;

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

	STATIC_CLASS(Renderer);
};