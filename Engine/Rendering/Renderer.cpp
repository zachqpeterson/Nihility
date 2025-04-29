#include "Renderer.hpp"

#include "Platform/Platform.hpp"
#include "Core/Time.hpp"
#include "Math/Math.hpp"
#include "Resources/Resources.hpp"

#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

VmaAllocator Renderer::vmaAllocator;
VkAllocationCallbacks* Renderer::allocationCallbacks = VK_NULL_HANDLE;
Instance Renderer::instance;
Device Renderer::device;
VkQueue Renderer::graphicsQueue;
VkQueue Renderer::presentQueue;
Swapchain Renderer::swapchain;
VkCommandPool Renderer::commandPool = VK_NULL_HANDLE;
CommandBuffer Renderer::commandBuffer;
Buffer Renderer::perspectiveViewMatrixUBO;
Buffer Renderer::worldPosBuffer;
Vector<Matrix4> Renderer::worldPosMatrices;
Buffer Renderer::boneMatrixBuffer;
Vector<Matrix4> Renderer::modelBoneMatrices;
VkDescriptorPool Renderer::vkDescriptorPool = VK_NULL_HANDLE;
VkDescriptorSetLayout Renderer::assimpDescriptorLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout Renderer::assimpTextureDescriptorLayout = VK_NULL_HANDLE;
VkDescriptorSet Renderer::assimpDescriptorSet = VK_NULL_HANDLE;
VkDescriptorSet Renderer::assimpSkinningDescriptorSet = VK_NULL_HANDLE;
Renderpass Renderer::renderpass;
PipelineLayout Renderer::assimpPipelineLayout;
PipelineLayout Renderer::assimpSkinningPipelineLayout;
Pipeline Renderer::assimpPipeline;
Pipeline Renderer::assimpSkinningPipeline;
Shader Renderer::assimpVertShader;
Shader Renderer::assimpFragShader;
Shader Renderer::assimpSkinningVertShader;
Shader Renderer::assimpSkinningFragShader;
FrameBuffer Renderer::frameBuffer;
VkSemaphore Renderer::presentSemaphore = VK_NULL_HANDLE;
VkSemaphore Renderer::renderSemaphore = VK_NULL_HANDLE;
VkFence Renderer::renderFence = VK_NULL_HANDLE;

VkFormat Renderer::depthFormat = VK_FORMAT_D32_SFLOAT;
VkImage Renderer::depthBuffer;
VkImageView Renderer::depthBufferView;
VmaAllocation_T* Renderer::depthBufferAllocation;

Camera Renderer::camera;

Vector<Vector<ModelInstance>> Renderer::instances;

struct VkPushConstants
{
	I32 modelStride;
	I32 worldPosOffset;
} modelData;

bool Renderer::Initialize()
{
	Logger::Trace("Initializing Renderer...");

	if (!instance.Create()) { Logger::Fatal("Failed To Create Vulkan Instance!"); return false; }
	if (!device.Create()) { Logger::Fatal("Failed To Create Vulkan Device!"); return false; }
	if (!InitializeVma()) { Logger::Fatal("Failed To Initialize Vma!"); return false; }
	if (!GetQueues()) { return false; }
	if (!swapchain.Create(false)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateDepthBuffer()) { Logger::Fatal("Failed To Create Depth Buffer!"); return false; }
	if (!(commandPool = CreateCommandPool(QueueType::Graphics))) { Logger::Fatal("Failed To Create Command Pool!"); return false; }
	if (!commandBuffer.Create(commandPool)) { Logger::Fatal("Failed To Create Command Buffer!"); return false; }
	if (!perspectiveViewMatrixUBO.Create(BufferType::Uniform)) { Logger::Fatal("Failed To Create Uniform Buffer!"); return false; }
	if (!worldPosBuffer.Create(BufferType::Shader)) { Logger::Fatal("Failed To Create Shader Storage Buffer!"); return false; }
	if (!boneMatrixBuffer.Create(BufferType::Shader)) { Logger::Fatal("Failed To Create Storage Buffer!"); return false; }
	if (!CreateDescriptorPool()) { Logger::Fatal("Failed To Create Descriptor Pool!"); return false; }
	if (!CreateDescriptorSetLayouts()) { Logger::Fatal("Failed To Create Descriptor Set Layouts!"); return false; }
	if (!CreateDescriptorSets()) { Logger::Fatal("Failed To Create Descriptor Sets!"); return false; }
	if (!CreateRenderpasses()) { Logger::Fatal("Failed To Create Renderpasses!"); return false; }
	if (!CreatePipelineLayouts()) { Logger::Fatal("Failed To Create Pipeline Layouts!"); return false; }
	if (!CreatePipelines()) { Logger::Fatal("Failed To Create Pipelines!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }
	if (!CreateSynchronization()) { Logger::Fatal("Failed To Create Synchronization Objects!"); return false; }

	camera.Create(CameraType::Perspective);

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Renderer...");

	vkDeviceWaitIdle(device);

	vkDestroyFence(device, renderFence, allocationCallbacks);
	vkDestroySemaphore(device, renderSemaphore, allocationCallbacks);
	vkDestroySemaphore(device, presentSemaphore, allocationCallbacks);

	frameBuffer.Destroy();

	assimpSkinningPipeline.Destroy();
	assimpPipeline.Destroy();

	assimpVertShader.Destroy();
	assimpFragShader.Destroy();
	assimpSkinningVertShader.Destroy();
	assimpSkinningFragShader.Destroy();

	assimpSkinningPipelineLayout.Destroy();
	assimpPipelineLayout.Destroy();

	renderpass.Destroy();

	vkFreeDescriptorSets(device, vkDescriptorPool, 1, &assimpSkinningDescriptorSet);
	vkFreeDescriptorSets(device, vkDescriptorPool, 1, &assimpDescriptorSet);

	vkDestroyDescriptorSetLayout(device, assimpDescriptorLayout, allocationCallbacks);
	vkDestroyDescriptorSetLayout(device, assimpTextureDescriptorLayout, allocationCallbacks);

	vkDestroyDescriptorPool(device, vkDescriptorPool, allocationCallbacks);

	boneMatrixBuffer.Destroy();

	worldPosBuffer.Destroy();

	perspectiveViewMatrixUBO.Destroy();

	commandBuffer.Destroy();

	DestroyCommandPool(commandPool);

	vkDestroyImageView(device, depthBufferView, allocationCallbacks);
	vmaDestroyImage(vmaAllocator, depthBuffer, depthBufferAllocation);

	swapchain.Destroy();

	vmaDestroyAllocator(vmaAllocator);

	device.Destroy();

	instance.Destroy();
}

void Renderer::Update()
{
	if (Math::IsZero(Time::DeltaTime())) { return; }

	VkValidateFExit(vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX));

	U32 imageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) { RecreateSwapchain(); }
	else { VkValidateFExit(res); }

	VkValidateFExit(vkResetFences(device, 1, &renderFence));

	camera.Update();

	MatrixData upload = {
		camera.View(),
		camera.Projection()
	};

	perspectiveViewMatrixUBO.UploadUniformData(upload);

	worldPosMatrices.Clear();
	modelBoneMatrices.Clear();

	for (Vector<ModelInstance>& modelInstances : instances)
	{
		U32 numberOfInstances = (U32)modelInstances.Size();
		if (numberOfInstances)
		{
			ResourceRef<Model> model = modelInstances[0].ModelRef();

			if (model->Animated() && !modelInstances[0].BoneMatrices().Empty())
			{
				for (U32 i = 0; i < numberOfInstances; ++i)
				{
					modelInstances[i].UpdateAnimation();
					Vector<Matrix4> instanceBoneMatrices = modelInstances[i].BoneMatrices();
					modelBoneMatrices.Insert(modelBoneMatrices.Size(), instanceBoneMatrices);
				}
			}
			else
			{
				for (const ModelInstance& instance : modelInstances)
				{
					worldPosMatrices.Emplace(instance.WorldTransformMatrix());
				}
			}
		}
	}

	U64 bufferSize = worldPosMatrices.Size() * sizeof(Matrix4) + modelBoneMatrices.Size() * sizeof(Matrix4);

	bool resize = false;
	resize = boneMatrixBuffer.UploadShaderData(modelBoneMatrices);
	resize |= worldPosBuffer.UploadShaderData(worldPosMatrices);

	if (resize) { UpdateDescriptorSets(); }

	commandBuffer.Reset();
	commandBuffer.BeginSingleShot();

	VkClearValue colorClearValue;
	colorClearValue.color = { { 0.25f, 0.25f, 0.25f, 1.0f } };

	VkClearValue depthValue;
	depthValue.depthStencil.depth = 1.0f;

	Vector<VkClearValue> clearValues = { colorClearValue, depthValue };

	VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderpass;
	renderPassBeginInfo.framebuffer = frameBuffer.vkFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = swapchain.extent;
	renderPassBeginInfo.clearValueCount = (U32)clearValues.Size();
	renderPassBeginInfo.pClearValues = clearValues.Data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//Flip viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (F32)swapchain.extent.width;
	viewport.height = (F32)swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.extent;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	U32 worldPosOffset = 0;
	U32 worldPosOffsetSkinned = 0;
	for (Vector<ModelInstance>& modelInstances : instances)
	{
		U32 numberOfInstances = (U32)modelInstances.Size();
		if (numberOfInstances > 0)
		{
			ResourceRef<Model> model = modelInstances[0].ModelRef();

			if (model->Animated() && !modelInstances[0].BoneMatrices().Empty())
			{
				U32 numberOfBones = (U32)model->BoneList().Size();

				modelData.modelStride = (I32)numberOfBones;
				modelData.worldPosOffset = worldPosOffsetSkinned;
				vkCmdPushConstants(commandBuffer, assimpSkinningPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT, 0, (U32)sizeof(VkPushConstants), &modelData);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, assimpSkinningPipeline);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					assimpSkinningPipelineLayout, 1, 1, &assimpSkinningDescriptorSet, 0, nullptr);
				DrawInstanced(model, numberOfInstances);
				worldPosOffsetSkinned += numberOfInstances * numberOfBones;
			}
			else
			{
				modelData.worldPosOffset = worldPosOffset;
				vkCmdPushConstants(commandBuffer, assimpPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT, 0, (U32)sizeof(VkPushConstants), &modelData);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, assimpPipeline);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					assimpPipelineLayout, 1, 1, &assimpDescriptorSet, 0, nullptr);
				DrawInstanced(model, numberOfInstances);
				worldPosOffset += numberOfInstances;
			}
		}
	}


	//TODO: Draw UI


	vkCmdEndRenderPass(commandBuffer);

	commandBuffer.End();

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderSemaphore;

	VkValidateFExit(vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFence));

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	res = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) { RecreateSwapchain(); }
	else { VkValidateFExit(res); }
}

void Renderer::Draw(ResourceRef<Model> model)
{
	for (U32 i = 0; i < model->modelMeshes.Size(); ++i)
	{
		MeshData& mesh = model->modelMeshes[i];

		// find diffuse texture by name
		ResourceRef<Texture> diffuseTex;
		String* diffuseTexName = mesh.textures.Get(TextureType::Diffuse);
		if (diffuseTexName)
		{
			diffuseTex = *model->textures.Get(*diffuseTexName);
		}

		/* switch between animated and non-animated pipeline layout */
		VkPipelineLayout renderLayout;
		if (model->Animated()) { renderLayout = assimpSkinningPipelineLayout; }
		else { renderLayout = assimpPipelineLayout; }

		if (diffuseTex)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderLayout, 0, 1, &diffuseTex->descriptorSet, 0, nullptr);
		}
		else
		{
			if (mesh.usesPBRColors)
			{
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					renderLayout, 0, 1, &Resources::WhiteTexture()->descriptorSet, 0, nullptr);
			}
			else
			{
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					renderLayout, 0, 1, &Resources::PlaceholderTexture()->descriptorSet, 0, nullptr);
			}
		}

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertexBuffers[i].vkBuffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, model->indexBuffers[i].vkBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, (U32)mesh.indices.Size(), 1, 0, 0, 0);
	}
}

void Renderer::DrawInstanced(ResourceRef<Model> model, U32 instanceCount)
{
	for (U32 i = 0; i < model->modelMeshes.Size(); ++i)
	{
		MeshData& mesh = model->modelMeshes[i];

		// find diffuse texture by name
		ResourceRef<Texture> diffuseTex;
		String* diffuseTexName = mesh.textures.Get(TextureType::Diffuse);
		if (diffuseTexName)
		{
			diffuseTex = *model->textures.Get(*diffuseTexName);
		}

		/* switch between animated and non-animated pipeline layout */
		VkPipelineLayout renderLayout;
		if (model->Animated()) { renderLayout = assimpSkinningPipelineLayout; }
		else { renderLayout = assimpPipelineLayout; }

		if (diffuseTex)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderLayout, 0, 1, &diffuseTex->descriptorSet, 0, nullptr);
		}
		else
		{
			if (mesh.usesPBRColors)
			{
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					renderLayout, 0, 1, &Resources::WhiteTexture()->descriptorSet, 0, nullptr);
			}
			else
			{
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					renderLayout, 0, 1, &Resources::PlaceholderTexture()->descriptorSet, 0, nullptr);
			}
		}

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertexBuffers[i].vkBuffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, model->indexBuffers[i].vkBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, (U32)mesh.indices.Size(), instanceCount, 0, 0, 0);
	}
}

bool Renderer::InitializeVma()
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = device.physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

	VkValidateFR(vmaCreateAllocator(&allocatorInfo, &vmaAllocator));

	return true;
}

bool Renderer::GetQueues()
{
	graphicsQueue = device.GetQueue(QueueType::Graphics);
	if (graphicsQueue == nullptr)
	{
		Logger::Fatal("Failed To Get Graphics Queue!");
		return false;
	}

	presentQueue = device.GetQueue(QueueType::Present);
	if (presentQueue == nullptr)
	{
		Logger::Fatal("Failed To Get Present Queue!");
		return false;
	}

	return true;
}

bool Renderer::CreateDepthBuffer()
{
	VkExtent3D depthImageExtent = {
		swapchain.extent.width,
		swapchain.extent.height,
		1
	};
	
	VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = depthFormat;
	imageCreateInfo.extent = depthImageExtent;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.flags = 0;
	allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	allocationInfo.preferredFlags = 0;
	allocationInfo.memoryTypeBits = 0;
	allocationInfo.pool = nullptr;
	allocationInfo.pUserData = nullptr;
	allocationInfo.priority = 0;

	VkValidateFR(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocationInfo, &depthBuffer, &depthBufferAllocation, nullptr));

	VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = depthBuffer;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.components = {};
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkValidateFR(vkCreateImageView(device, &imageViewCreateInfo, allocationCallbacks, &depthBufferView));

	return true;
}

bool Renderer::CreateDescriptorPool()
{
	Vector<VkDescriptorPoolSize> poolSizes =
	{
	  { VK_DESCRIPTOR_TYPE_SAMPLER, 10000 },
	  { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000 },
	  { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10000 },
	  { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
	  { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
	  { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 10000;
	descriptorPoolCreateInfo.poolSizeCount = (U32)poolSizes.Size();
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.Data();

	VkValidateFR(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, allocationCallbacks, &vkDescriptorPool));

	return true;
}

//TODO: Automate
bool Renderer::CreateDescriptorSetLayouts()
{
	/* texture */
	VkDescriptorSetLayoutBinding assimpTextureBind{};
	assimpTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	assimpTextureBind.binding = 0;
	assimpTextureBind.descriptorCount = 1;
	assimpTextureBind.pImmutableSamplers = nullptr;
	assimpTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	Vector<VkDescriptorSetLayoutBinding> assimpTexBindings = { assimpTextureBind };

	VkDescriptorSetLayoutCreateInfo assimpTextureCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	assimpTextureCreateInfo.pNext = nullptr;
	assimpTextureCreateInfo.flags = 0;
	assimpTextureCreateInfo.bindingCount = (U32)assimpTexBindings.Size();
	assimpTextureCreateInfo.pBindings = assimpTexBindings.Data();

	VkValidateFR(vkCreateDescriptorSetLayout(device, &assimpTextureCreateInfo, allocationCallbacks, &assimpTextureDescriptorLayout));

	/* UBO/SSBO in shader */
	VkDescriptorSetLayoutBinding assimpUboBind{};
	assimpUboBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	assimpUboBind.binding = 0;
	assimpUboBind.descriptorCount = 1;
	assimpUboBind.pImmutableSamplers = nullptr;
	assimpUboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding assimpSsboBind{};
	assimpSsboBind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	assimpSsboBind.binding = 1;
	assimpSsboBind.descriptorCount = 1;
	assimpSsboBind.pImmutableSamplers = nullptr;
	assimpSsboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	Vector<VkDescriptorSetLayoutBinding> assimpBindings = { assimpUboBind, assimpSsboBind };

	VkDescriptorSetLayoutCreateInfo assimpCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	assimpTextureCreateInfo.pNext = nullptr;
	assimpTextureCreateInfo.flags = 0;
	assimpCreateInfo.bindingCount = (U32)assimpBindings.Size();
	assimpCreateInfo.pBindings = assimpBindings.Data();

	VkValidateFR(vkCreateDescriptorSetLayout(device, &assimpCreateInfo, allocationCallbacks, &assimpDescriptorLayout));

	return true;
}

bool Renderer::CreateDescriptorSets()
{
	/* non-animated models */
	VkDescriptorSetAllocateInfo descriptorAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	descriptorAllocateInfo.pNext = nullptr;
	descriptorAllocateInfo.descriptorPool = vkDescriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pSetLayouts = &assimpDescriptorLayout;

	VkValidateFR(vkAllocateDescriptorSets(device, &descriptorAllocateInfo, &assimpDescriptorSet));

	/* animated models */
	VkDescriptorSetAllocateInfo skinningDescriptorAllocateInfo{};
	skinningDescriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	skinningDescriptorAllocateInfo.descriptorPool = vkDescriptorPool;
	skinningDescriptorAllocateInfo.descriptorSetCount = 1;
	skinningDescriptorAllocateInfo.pSetLayouts = &assimpDescriptorLayout;

	VkValidateFR(vkAllocateDescriptorSets(device, &skinningDescriptorAllocateInfo, &assimpSkinningDescriptorSet));

	UpdateDescriptorSets();

	return true;
}

bool Renderer::CreateRenderpasses()
{
	return renderpass.Create();
}

bool Renderer::CreatePipelineLayouts()
{
	Vector<VkDescriptorSetLayout> layouts = { assimpTextureDescriptorLayout, assimpDescriptorLayout };
	Vector<VkPushConstantRange> pushConstants = { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkPushConstants) } };

	if (!assimpPipelineLayout.Create(layouts, pushConstants)) { return false; }
	if (!assimpSkinningPipelineLayout.Create(layouts, pushConstants)) { return false; }

	return true;
}

bool Renderer::CreatePipelines()
{
	assimpVertShader.Create("shaders/assimp.vert.spv", ShaderType::Vertex);
	assimpFragShader.Create("shaders/assimp.frag.spv", ShaderType::Fragment);
	assimpSkinningVertShader.Create("shaders/assimp_skinning.vert.spv", ShaderType::Vertex);
	assimpSkinningFragShader.Create("shaders/assimp_skinning.frag.spv", ShaderType::Fragment);

	if (!assimpPipeline.Create(assimpPipelineLayout, { assimpVertShader, assimpFragShader })) { return false; }
	if (!assimpSkinningPipeline.Create(assimpSkinningPipelineLayout, { assimpSkinningVertShader, assimpSkinningFragShader })) { return false; }

	return true;
}

bool Renderer::CreateSynchronization()
{
	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;

	VkValidateFR(vkCreateSemaphore(device, &semaphoreInfo, Renderer::allocationCallbacks, &presentSemaphore));
	VkValidateFR(vkCreateSemaphore(device, &semaphoreInfo, Renderer::allocationCallbacks, &renderSemaphore));
	VkValidateFR(vkCreateFence(device, &fenceInfo, Renderer::allocationCallbacks, &renderFence));

	return true;
}

bool Renderer::RecreateSwapchain()
{
	vkDeviceWaitIdle(device);

	frameBuffer.Destroy();
	vkDestroyImageView(device, depthBufferView, allocationCallbacks);
	vmaDestroyImage(vmaAllocator, depthBuffer, depthBufferAllocation);

	if (!swapchain.Create(true)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateDepthBuffer()) { Logger::Fatal("Failed To Create Depth Buffer!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }

	return true;
}

bool Renderer::UpdateDescriptorSets()
{
	/* non-animated shader */
	VkDescriptorBufferInfo matrixInfo{};
	matrixInfo.buffer = perspectiveViewMatrixUBO.vkBuffer;
	matrixInfo.offset = 0;
	matrixInfo.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo worldPosInfo{};
	worldPosInfo.buffer = worldPosBuffer.vkBuffer;
	worldPosInfo.offset = 0;
	worldPosInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet matrixWriteDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	matrixWriteDescriptorSet.pNext = nullptr;
	matrixWriteDescriptorSet.dstSet = assimpDescriptorSet;
	matrixWriteDescriptorSet.dstBinding = 0;
	matrixWriteDescriptorSet.dstArrayElement = 0;
	matrixWriteDescriptorSet.descriptorCount = 1;
	matrixWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	matrixWriteDescriptorSet.pImageInfo = nullptr;
	matrixWriteDescriptorSet.pBufferInfo = &matrixInfo;
	matrixWriteDescriptorSet.pTexelBufferView = nullptr;

	VkWriteDescriptorSet posWriteDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	posWriteDescriptorSet.pNext = nullptr;
	posWriteDescriptorSet.dstSet = assimpDescriptorSet;
	posWriteDescriptorSet.dstBinding = 1;
	posWriteDescriptorSet.dstArrayElement = 0;
	posWriteDescriptorSet.descriptorCount = 1;
	posWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	posWriteDescriptorSet.pImageInfo = nullptr;
	posWriteDescriptorSet.pBufferInfo = &worldPosInfo;
	posWriteDescriptorSet.pTexelBufferView = nullptr;

	Vector<VkWriteDescriptorSet> writeDescriptorSets = { matrixWriteDescriptorSet, posWriteDescriptorSet };

	vkUpdateDescriptorSets(device, (U32)writeDescriptorSets.Size(), writeDescriptorSets.Data(), 0, nullptr);

	/* animated shader */
	VkDescriptorBufferInfo boneMatrixInfo{};
	boneMatrixInfo.buffer = boneMatrixBuffer.vkBuffer;
	boneMatrixInfo.offset = 0;
	boneMatrixInfo.range = VK_WHOLE_SIZE;

	matrixWriteDescriptorSet.dstSet = assimpSkinningDescriptorSet;

	VkWriteDescriptorSet boneMatrixWriteDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	boneMatrixWriteDescriptorSet.pNext = nullptr;
	boneMatrixWriteDescriptorSet.dstSet = assimpSkinningDescriptorSet;
	boneMatrixWriteDescriptorSet.dstBinding = 1;
	boneMatrixWriteDescriptorSet.dstArrayElement = 0;
	boneMatrixWriteDescriptorSet.descriptorCount = 1;
	boneMatrixWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	boneMatrixWriteDescriptorSet.pImageInfo = nullptr;
	boneMatrixWriteDescriptorSet.pBufferInfo = &boneMatrixInfo;
	boneMatrixWriteDescriptorSet.pTexelBufferView = nullptr;

	Vector<VkWriteDescriptorSet> skinningWriteDescriptorSets = { matrixWriteDescriptorSet, boneMatrixWriteDescriptorSet };

	vkUpdateDescriptorSets(device, (U32)skinningWriteDescriptorSets.Size(), skinningWriteDescriptorSets.Data(), 0, nullptr);

	return true;
}

VkCommandPool Renderer::CreateCommandPool(QueueType queueType)
{
	VkCommandPool pool;

	VkCommandPoolCreateInfo poolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolCreateInfo.queueFamilyIndex = device.GetQueueIndex(queueType);
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkValidateF(vkCreateCommandPool(device, &poolCreateInfo, allocationCallbacks, &pool));

	return pool;
}

void Renderer::DestroyCommandPool(VkCommandPool pool)
{
	vkDestroyCommandPool(device, pool, allocationCallbacks);
}

bool Renderer::UploadTexture(Resource<Texture>& texture, U8* data)
{
	VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stagingBufferInfo.pNext = nullptr;
	stagingBufferInfo.flags = 0;
	stagingBufferInfo.size = texture->size;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingBufferInfo.queueFamilyIndexCount = 0;
	stagingBufferInfo.pQueueFamilyIndices = nullptr;

	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferAlloc;

	VmaAllocationCreateInfo stagingAllocInfo{};
	stagingAllocInfo.flags = 0;
	stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	stagingAllocInfo.requiredFlags = 0;
	stagingAllocInfo.preferredFlags = 0;
	stagingAllocInfo.memoryTypeBits = 0;
	stagingAllocInfo.pool = nullptr;
	stagingAllocInfo.pUserData = nullptr;
	stagingAllocInfo.priority = 0.0f;

	VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingBufferAlloc, nullptr));

	void* uploadData;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAlloc, &uploadData));

	memcpy(uploadData, data, texture->size);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAlloc);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAlloc, 0, texture->size);

	CommandBuffer uploadCommandBuffer;
	uploadCommandBuffer.CreateSingleShotBuffer(commandPool);

	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.pNext = nullptr;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.extent.width = texture->width;
	imageInfo.extent.height = texture->height;
	imageInfo.extent.depth = texture->depth;
	imageInfo.mipLevels = texture->mipmapLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo imageAllocInfo{};
	imageAllocInfo.flags = 0;
	imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imageAllocInfo.requiredFlags = 0;
	imageAllocInfo.preferredFlags = 0;
	imageAllocInfo.memoryTypeBits = 0;
	imageAllocInfo.pool = nullptr;
	imageAllocInfo.pUserData = nullptr;
	imageAllocInfo.priority = 0.0f;

	VkValidateR(vmaCreateImage(vmaAllocator, &imageInfo, &imageAllocInfo, &texture->image, &texture->allocation, nullptr));

	VkImageSubresourceRange stagingBufferRange{};
	stagingBufferRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingBufferRange.baseMipLevel = 0;
	stagingBufferRange.levelCount = texture->mipmapLevels;
	stagingBufferRange.baseArrayLayer = 0;
	stagingBufferRange.layerCount = 1;

	VkImageMemoryBarrier stagingBufferTransferBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	stagingBufferTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	stagingBufferTransferBarrier.pNext = nullptr;
	stagingBufferTransferBarrier.srcAccessMask = 0;
	stagingBufferTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	stagingBufferTransferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	stagingBufferTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingBufferTransferBarrier.srcQueueFamilyIndex = 0;
	stagingBufferTransferBarrier.dstQueueFamilyIndex = 0;
	stagingBufferTransferBarrier.image = texture->image;
	stagingBufferTransferBarrier.subresourceRange = stagingBufferRange;

	VkOffset3D textureOffset{};
	textureOffset.x = 0;
	textureOffset.y = 0;
	textureOffset.z = 0;

	VkExtent3D textureExtent{};
	textureExtent.width = texture->width;
	textureExtent.height = texture->height;
	textureExtent.depth = texture->depth;

	VkBufferImageCopy stagingBufferCopy{};
	stagingBufferCopy.bufferOffset = 0;
	stagingBufferCopy.bufferRowLength = 0;
	stagingBufferCopy.bufferImageHeight = 0;
	stagingBufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingBufferCopy.imageSubresource.mipLevel = 0;
	stagingBufferCopy.imageSubresource.baseArrayLayer = 0;
	stagingBufferCopy.imageSubresource.layerCount = 1;
	stagingBufferCopy.imageOffset = textureOffset;
	stagingBufferCopy.imageExtent = textureExtent;

	VkImageMemoryBarrier stagingBufferShaderBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	stagingBufferShaderBarrier.pNext = nullptr;
	stagingBufferShaderBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	stagingBufferShaderBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	stagingBufferShaderBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingBufferShaderBarrier.newLayout = texture->mipmapLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	stagingBufferShaderBarrier.srcQueueFamilyIndex = 0;
	stagingBufferShaderBarrier.dstQueueFamilyIndex = 0;
	stagingBufferShaderBarrier.image = texture->image;
	stagingBufferShaderBarrier.subresourceRange = stagingBufferRange;

	vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingBufferTransferBarrier);
	vkCmdCopyBufferToImage(uploadCommandBuffer, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &stagingBufferCopy);
	vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingBufferShaderBarrier);

	if (texture->mipmapLevels > 1)
	{
		VkImageSubresourceRange blitRange{};
		blitRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRange.baseMipLevel = 0;
		blitRange.levelCount = 1;
		blitRange.baseArrayLayer = 0;
		blitRange.layerCount = 1;

		VkImageMemoryBarrier firstBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		firstBarrier.pNext = nullptr;
		firstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		firstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		firstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		firstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		firstBarrier.srcQueueFamilyIndex = 0;
		firstBarrier.dstQueueFamilyIndex = 0;
		firstBarrier.image = texture->image;
		firstBarrier.subresourceRange = blitRange;

		VkImageMemoryBarrier secondBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		secondBarrier.pNext = nullptr;
		secondBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		secondBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		secondBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		secondBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		secondBarrier.srcQueueFamilyIndex = 0;
		secondBarrier.dstQueueFamilyIndex = 0;
		secondBarrier.image = texture->image;
		secondBarrier.subresourceRange = blitRange;

		VkImageBlit mipBlit{};
		mipBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipBlit.srcSubresource.mipLevel = 0;
		mipBlit.srcSubresource.baseArrayLayer = 0;
		mipBlit.srcSubresource.layerCount = 1;
		mipBlit.srcOffsets[0] = { 0, 0, 0 };
		mipBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipBlit.dstSubresource.mipLevel = 0;
		mipBlit.dstSubresource.baseArrayLayer = 0;
		mipBlit.dstSubresource.layerCount = 1;
		mipBlit.dstOffsets[0] = { 0, 0, 0 };

		I32 mipWidth = texture->width;
		I32 mipHeight = texture->height;

		for (I32 i = 1; i < texture->mipmapLevels; ++i)
		{
			mipBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			mipBlit.srcSubresource.mipLevel = i - 1;

			mipBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			mipBlit.dstSubresource.mipLevel = i;

			firstBarrier.subresourceRange.baseMipLevel = i - 1;
			vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &firstBarrier);

			vkCmdBlitImage(uploadCommandBuffer,
				texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &mipBlit, VK_FILTER_LINEAR);

			secondBarrier.subresourceRange.baseMipLevel = i - 1;
			vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &secondBarrier);

			if (mipWidth > 1) { mipWidth /= 2; }
			if (mipHeight > 1) { mipHeight /= 2; }
		}

		VkImageMemoryBarrier lastBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		lastBarrier.pNext = nullptr;
		lastBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		lastBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		lastBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		lastBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		lastBarrier.srcQueueFamilyIndex = 0;
		lastBarrier.dstQueueFamilyIndex = 0;
		lastBarrier.image = texture->image;
		lastBarrier.subresourceRange = blitRange;
		lastBarrier.subresourceRange.baseMipLevel = texture->mipmapLevels - 1;

		vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &lastBarrier);
	}

	bool commandResult = uploadCommandBuffer.SubmitSingleShotBuffer(graphicsQueue);
	vmaDestroyBuffer(vmaAllocator, stagingBuffer, stagingBufferAlloc);

	if (!commandResult) { return false; }

	VkImageViewCreateInfo texViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	texViewInfo.pNext = nullptr;
	texViewInfo.flags = 0;
	texViewInfo.image = texture->image;
	texViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	texViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	texViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	texViewInfo.subresourceRange.baseMipLevel = 0;
	texViewInfo.subresourceRange.levelCount = texture->mipmapLevels;
	texViewInfo.subresourceRange.baseArrayLayer = 0;
	texViewInfo.subresourceRange.layerCount = 1;

	VkValidateR(vkCreateImageView(device, &texViewInfo, allocationCallbacks, &texture->imageView));

	const VkBool32 anisotropyAvailable = device.physicalDevice.features.samplerAnisotropy;
	const F32 maxAnisotropy = device.physicalDevice.properties.limits.maxSamplerAnisotropy;

	VkSamplerCreateInfo texSamplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	texSamplerInfo.pNext = nullptr;
	texSamplerInfo.flags = 0;
	texSamplerInfo.magFilter = VK_FILTER_LINEAR;
	texSamplerInfo.minFilter = VK_FILTER_LINEAR;
	texSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	texSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	texSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	texSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	texSamplerInfo.mipLodBias = 0.0f;
	texSamplerInfo.anisotropyEnable = anisotropyAvailable;
	texSamplerInfo.maxAnisotropy = maxAnisotropy;
	texSamplerInfo.compareEnable = VK_FALSE;
	texSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	texSamplerInfo.minLod = 0.0f;
	texSamplerInfo.maxLod = (F32)texture->mipmapLevels;
	texSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	texSamplerInfo.unnormalizedCoordinates = VK_FALSE;

	VkValidateR(vkCreateSampler(device, &texSamplerInfo, allocationCallbacks, &texture->sampler));

	VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorAllocateInfo.pNext;
	descriptorAllocateInfo.descriptorPool = vkDescriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pSetLayouts = &assimpTextureDescriptorLayout;

	VkValidateR(vkAllocateDescriptorSets(device, &descriptorAllocateInfo, &texture->descriptorSet));

	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.imageView = texture->imageView;
	descriptorImageInfo.sampler = texture->sampler;

	VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = texture->descriptorSet;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.pBufferInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

	return true;
}

void Renderer::DestroyTexture(Resource<Texture>& texture)
{
	vkFreeDescriptorSets(device, vkDescriptorPool, 1, &texture->descriptorSet);
	vkDestroySampler(device, texture->sampler, allocationCallbacks);
	vkDestroyImageView(device, texture->imageView, allocationCallbacks);
	vmaDestroyImage(vmaAllocator, texture->image, texture->allocation);
}



void Renderer::AddModelInstance(ModelInstance& instance)
{
	Vector<ModelInstance>* instanceList = instances.Find([instance](Vector<ModelInstance>* instanceList) { return instanceList->Get(0).ModelRef() == instance.ModelRef(); });

	if (instanceList) { instanceList->Emplace(instance); }
	else { instances.Emplace(Vector<ModelInstance>{ instance }); }
}