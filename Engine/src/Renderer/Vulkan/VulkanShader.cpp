#include "VulkanShader.hpp"

#include "VulkanRenderpass.hpp"
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanImage.hpp"
#include "VulkanSwapchain.hpp"

#include "Memory/Memory.hpp"
#include "Resources/Resources.hpp"

bool VulkanShader::Create(RendererState* rendererState, Shader* shader)
{
	pipeline = (VulkanPipeline*)Memory::Allocate(sizeof(VulkanPipeline), MEMORY_TAG_RESOURCE);
	uniformBuffer = (VulkanBuffer*)Memory::Allocate(sizeof(VulkanBuffer), MEMORY_TAG_RESOURCE);

	if (shader->stages.Size() >= VULKAN_SHADER_MAX_STAGES)
	{
		Logger::Error("VulkanShader::Create: Shaders may have a maximum of {} stages", VULKAN_SHADER_MAX_STAGES);
		Memory::Free(pipeline, sizeof(VulkanPipeline), MEMORY_TAG_RESOURCE);
		Memory::Free(uniformBuffer, sizeof(VulkanBuffer), MEMORY_TAG_RESOURCE);
		return false;
	}

	for (U32 i = 0; i < shader->stages.Size(); ++i)
	{
		VkShaderStageFlagBits stageFlag;
		switch (shader->stages[i])
		{
		case SHADER_STAGE_VERTEX:
			stageFlag = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case SHADER_STAGE_FRAGMENT:
			stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case SHADER_STAGE_GEOMETRY:
			Logger::Warn("VulkanShader::Create: VK_SHADER_STAGE_GEOMETRY_BIT isn't currently supported.");
			continue;
		case SHADER_STAGE_COMPUTE:
			Logger::Warn("VulkanShader::Create: SHADER_STAGE_COMPUTE isn't currently supported.");
			continue;
		default:
			Logger::Error("VulkanShader::Create: Unsupported shader stage at index: {}. Stage ignored.", i);
			continue;
		}

		ShaderStageConfig stageConfig;
		stageConfig.stage = stageFlag;
		stageConfig.fileName = shader->stageFilenames[i];
		config.stages.Push(stageConfig);
	}

	config.poolSizes.Push({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 });         // HACK: max number of ubo descriptor sets.
	config.poolSizes.Push({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 }); // HACK: max number of image sampler descriptor sets.

	for (Uniform& u : shader->uniforms[SHADER_SCOPE_GLOBAL])
	{
		if (u.bindingIndex >= config.descriptorSets[SHADER_SCOPE_GLOBAL].Size())
		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = u.bindingIndex;
			binding.descriptorCount = 1;
			if (u.type == FIELD_TYPE_SAMPLER)
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}
			else
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				++globalDescriptorUboCount;
			}
			binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; //TODO: dynamic
			config.descriptorSets[SHADER_SCOPE_GLOBAL].Push(binding);
		}
		else if (u.type == FIELD_TYPE_SAMPLER)
		{
			++config.descriptorSets[SHADER_SCOPE_GLOBAL][u.bindingIndex].descriptorCount;
		}
	}

	for (Uniform& u : shader->uniforms[SHADER_SCOPE_INSTANCE])
	{
		if (u.bindingIndex >= config.descriptorSets[SHADER_SCOPE_INSTANCE].Size())
		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = u.bindingIndex;
			binding.descriptorCount = 1;
			if (u.type == FIELD_TYPE_SAMPLER)
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}
			else
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				++instanceDescriptorUboCount;
			}
			binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; //TODO: dynamic
			config.descriptorSets[SHADER_SCOPE_INSTANCE].Push(binding);
		}
		else if (u.type == FIELD_TYPE_SAMPLER)
		{
			++config.descriptorSets[SHADER_SCOPE_INSTANCE][u.bindingIndex].descriptorCount;
		}
	}

	// TODO: Dynamic
	for (InstanceState& state : instanceStates)
	{
		state.id = INVALID_ID;
		state.offset = INVALID_ID;
	}

	return true;
}

void VulkanShader::Destroy(RendererState* rendererState)
{
	for (U32 i = 0; i < config.descriptorSets.Size(); ++i)
	{
		if (descriptorSetLayouts[i])
		{
			vkDestroyDescriptorSetLayout(rendererState->device->logicalDevice, descriptorSetLayouts[i], rendererState->allocator);
			descriptorSetLayouts[i] = VK_NULL_HANDLE;
		}
	}

	if (descriptorPool)
	{
		vkDestroyDescriptorPool(rendererState->device->logicalDevice, descriptorPool, rendererState->allocator);
	}

	if (mappedUniformBufferBlock)
	{
		uniformBuffer->UnlockMemory(rendererState);
		mappedUniformBufferBlock = nullptr;
		uniformBuffer->Destroy(rendererState);
	}

	pipeline->Destroy(rendererState);
	Memory::Free(pipeline, sizeof(VulkanPipeline), MEMORY_TAG_RENDERER);

	for (ShaderStage& s : stages)
	{
		vkDestroyShaderModule(rendererState->device->logicalDevice, s.handle, rendererState->allocator);
	}

	Memory::Zero(&config, sizeof(VulkanShaderConfig));
}

bool VulkanShader::Initialize(RendererState* rendererState, Shader* shader)
{
	stages.Resize(config.stages.Size());
	for (U32 i = 0; i < config.stages.Size(); ++i)
	{
		if (!CreateShaderModule(rendererState, config.stages[i], &stages[i]))
		{
			Logger::Error("VulkanShader::Initialize: Unable to create {} shader module for {}. Shader will be destroyed.", config.stages[i].fileName, shader->name);
			return false;
		}
	}

	static const VkFormat types[11]{
	    VK_FORMAT_R32_SFLOAT,
	    VK_FORMAT_R32G32_SFLOAT,
	    VK_FORMAT_R32G32B32_SFLOAT,
	    VK_FORMAT_R32G32B32A32_SFLOAT,
	    VK_FORMAT_R8_SINT,
	    VK_FORMAT_R8_UINT,
	    VK_FORMAT_R16_SINT,
	    VK_FORMAT_R16_UINT,
	    VK_FORMAT_R32_SINT,
	    VK_FORMAT_R32_UINT
	};

	U32 offset = 0;
	for (U32 i = 0; i < shader->attributes.Size(); ++i)
	{
		VkVertexInputAttributeDescription attribute;
		attribute.location = i;
		attribute.binding = 0;
		attribute.offset = offset;
		attribute.format = types[shader->attributes[i].type];

		config.attributes.Push(attribute);

		offset += shader->attributes[i].size;
	}

	VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = (U32)config.poolSizes.Size();
	poolInfo.pPoolSizes = config.poolSizes.Data();
	poolInfo.maxSets = (U32)config.descriptorSets.Size() * rendererState->swapchain->imageCount;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkCheck_ERROR(vkCreateDescriptorPool(rendererState->device->logicalDevice, &poolInfo, rendererState->allocator, &descriptorPool));

	for (U32 i = 0; i < config.descriptorSets.Size(); ++i)
	{
		descriptorSetLayouts.Push({});
		VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = (U32)config.descriptorSets[i].Size();
		layoutInfo.pBindings = config.descriptorSets[i].Data();
		VkCheck_ERROR(vkCreateDescriptorSetLayout(rendererState->device->logicalDevice, &layoutInfo, rendererState->allocator, &descriptorSetLayouts[i]));
	}

	Vector<VkPipelineShaderStageCreateInfo> stageInfos;
	for (U32 i = 0; i < stages.Size(); ++i)
	{
		stageInfos.Push(stages[i].shaderStageInfo);
	}

	pipeline->Create(
		rendererState,
		(VulkanRenderpass*)shader->renderpass->internalData,
		shader->attributeStride,
		(U32)config.attributes.Size(),
		config.attributes.Data(),
		(U32)config.descriptorSets.Size(),
		descriptorSetLayouts.Data(),
		(U32)config.stages.Size(),
		stageInfos.Data(),
		false, true,
		(U32)shader->pushConstantRanges.Size(),
		shader->pushConstantRanges.Data());

	shader->requiredUboAlignment = rendererState->device->properties.limits.minUniformBufferOffsetAlignment;

	shader->globalUboStride = AlignPow2(shader->globalUboSize, shader->requiredUboAlignment);
	shader->instanceUboStride = AlignPow2(shader->instanceUboSize, shader->requiredUboAlignment);

	U32 deviceLocalBits = rendererState->device->supportsDeviceLocalHostVisible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
	// TODO: max count should be configurable, or perhaps long term support of buffer resizing.
	U64 totalBufferSize = shader->globalUboStride + (shader->instanceUboStride * VULKAN_MAX_MATERIAL_COUNT);

	if (totalBufferSize)
	{
		if (!uniformBuffer->Create(rendererState, totalBufferSize,
			(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
			(VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | deviceLocalBits),
			true, true))
		{
			Logger::Error("Failed to create uniform buffer!");
			return false;
		}

		shader->globalUboOffset = uniformBuffer->Allocate(shader->globalUboStride);

		mappedUniformBufferBlock = uniformBuffer->LockMemory(rendererState, 0, U32_MAX, 0);
	}

	Vector<VkDescriptorSetLayout> layouts(rendererState->swapchain->imageCount, descriptorSetLayouts[SHADER_SCOPE_GLOBAL]);

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = (U32)layouts.Size();
	allocInfo.pSetLayouts = layouts.Data();
	globalDescriptorSets.Resize(layouts.Size());
	VkCheck_ERROR(vkAllocateDescriptorSets(rendererState->device->logicalDevice, &allocInfo, globalDescriptorSets.Data()));

	return true;
}

bool VulkanShader::Use(RendererState* rendererState)
{
	pipeline->Bind(rendererState->graphicsCommandBuffers[rendererState->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS);
	return true;
}

bool VulkanShader::ApplyGlobals(RendererState* rendererState, Shader* shader)
{
	U32 imageIndex = rendererState->imageIndex;
	VkCommandBuffer commandBuffer = rendererState->graphicsCommandBuffers[imageIndex].handle;
	VkDescriptorSet globalDescriptor = globalDescriptorSets[imageIndex];

	Vector<VkWriteDescriptorSet> descriptorWrites;

	U32 binding = 0;

	VkWriteDescriptorSet uboWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	VkDescriptorBufferInfo bufferInfo;

	if (globalDescriptorUboCount)
	{
		bufferInfo.buffer = uniformBuffer->handle;
		bufferInfo.offset = shader->globalUboOffset;
		bufferInfo.range = shader->globalUboStride;

		uboWrite.dstSet = globalDescriptorSets[imageIndex];
		uboWrite.dstBinding = binding;
		uboWrite.dstArrayElement = 0;
		uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboWrite.descriptorCount = 1;
		uboWrite.pBufferInfo = &bufferInfo;

		descriptorWrites.Push(uboWrite);

		++binding;
	}

	VkWriteDescriptorSet samplerWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

	Vector<VkDescriptorImageInfo> imageInfos;
	if (config.descriptorSets[SHADER_SCOPE_GLOBAL].Size() - globalDescriptorUboCount)
	{
		U32 index = 0;
		for (const VkDescriptorSetLayoutBinding& b : config.descriptorSets[SHADER_SCOPE_GLOBAL])
		{
			if (b.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				for (U32 i = 0; i < b.descriptorCount; ++i)
				{
					TextureMap* map = shader->globalTextureMaps[index];
					++index;
					Texture* texture = map->texture;
					VulkanImage* image = (VulkanImage*)texture->internalData;

					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = image->view;
					imageInfo.sampler = (VkSampler)map->internalData;

					imageInfos.Push(imageInfo);
				}
			}
		}

		samplerWrite.dstSet = globalDescriptor;
		samplerWrite.dstBinding = binding;
		samplerWrite.dstArrayElement = 0;
		samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerWrite.descriptorCount = (U32)imageInfos.Size();
		samplerWrite.pImageInfo = imageInfos.Data();

		descriptorWrites.Push(samplerWrite);

		++binding;
	}

	if (descriptorWrites.Size())
	{
		vkUpdateDescriptorSets(rendererState->device->logicalDevice, (U32)descriptorWrites.Size(), descriptorWrites.Data(), 0, nullptr);
	}

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1, &globalDescriptor, 0, nullptr);
	return true;
}

bool VulkanShader::ApplyInstance(RendererState* rendererState, Shader* shader, bool needsUpdate)
{
	if (!shader->useInstances)
	{
		Logger::Error("VulkanShader::ApplyInstance: This shader does not use instances.");
		return false;
	}

	U32 imageIndex = rendererState->imageIndex;
	VkCommandBuffer commandBuffer = rendererState->graphicsCommandBuffers[imageIndex].handle;

	InstanceState* objectState = &instanceStates[shader->boundInstanceId];
	VkDescriptorSet objectDescriptorSet = objectState->descriptorSets[imageIndex];

	U32 binding = 0;

	if (needsUpdate)
	{
		Vector<VkWriteDescriptorSet> descriptorWrites;

		VkWriteDescriptorSet uboDescriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		VkDescriptorBufferInfo bufferInfo;
		if (instanceDescriptorUboCount)
		{
			// TODO: determine if update is required.
			bufferInfo.buffer = uniformBuffer->handle;
			bufferInfo.offset = objectState->offset;
			bufferInfo.range = shader->instanceUboStride;

			uboDescriptor.dstSet = objectDescriptorSet;
			uboDescriptor.dstBinding = binding;
			uboDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptor.descriptorCount = 1;
			uboDescriptor.pBufferInfo = &bufferInfo;

			descriptorWrites.Push(uboDescriptor);
			++binding;
		}

		VkWriteDescriptorSet samplerDescriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		Vector<VkDescriptorImageInfo> imageInfos;
		if (config.descriptorSets[SHADER_SCOPE_INSTANCE].Size() - instanceDescriptorUboCount)
		{
			for (const VkDescriptorSetLayoutBinding& b : config.descriptorSets[SHADER_SCOPE_INSTANCE])
			{
				if (b.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					for (U32 i = 0; i < b.descriptorCount; ++i)
					{
						// TODO: only update in the list if actually needing an update
						TextureMap& map = instanceStates[shader->boundInstanceId].instanceTextureMaps[i];
						Texture* texture = map.texture;
						VulkanImage* image = (VulkanImage*)texture->internalData;

						VkDescriptorImageInfo imageInfo{};
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo.imageView = image->view;
						imageInfo.sampler = (VkSampler)map.internalData;

						imageInfos.Push(imageInfo);

						// TODO: change up descriptor state to handle this properly.
						// Sync frame generation if not using a default texture.
						// if (t->generation != INVALID_ID) {
						//     *descriptor_generation = t->generation;
						//     *descriptor_id = t->id;
						// }
					}
				}
			}

			samplerDescriptor.dstSet = objectDescriptorSet;
			samplerDescriptor.dstBinding = binding;
			samplerDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerDescriptor.descriptorCount = (U32)config.descriptorSets[SHADER_SCOPE_INSTANCE].Size() - instanceDescriptorUboCount;
			samplerDescriptor.pImageInfo = imageInfos.Data();

			descriptorWrites.Push(samplerDescriptor);

			++binding;
		}

		if (descriptorWrites.Size())
		{
			vkUpdateDescriptorSets(rendererState->device->logicalDevice, (U32)descriptorWrites.Size(), descriptorWrites.Data(), 0, nullptr);
		}
	}

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 1, 1, &objectDescriptorSet, 0, nullptr);
	return true;
}

U32 VulkanShader::AcquireInstanceResources(RendererState* rendererState, Shader* shader, Vector<TextureMap>& maps)
{
	// TODO: dynamic
	U32 outInstanceId = INVALID_ID;
	for (U32 i = 0; i < VULKAN_MAX_MATERIAL_COUNT; ++i)
	{
		if (instanceStates[i].id == INVALID_ID)
		{
			instanceStates[i].id = i;
			outInstanceId = i;
			break;
		}
	}

	if (outInstanceId == INVALID_ID)
	{
		Logger::Error("VulkanShader::AcquireInstanceResources: failed to acquire new id");
		return INVALID_ID;
	}

	InstanceState& instanceState = instanceStates[outInstanceId];

	instanceState.instanceTextureMaps = maps;

	if (shader->instanceUboStride)
	{
		U64 size = shader->instanceUboStride;

		instanceState.offset = (U32)uniformBuffer->Allocate(size);
		if (instanceState.offset == U32_MAX)
		{
			Logger::Error("VulkanShader::AcquireInstanceResources: failed to acquire ubo space");
			return INVALID_ID;
		}
	}
	else
	{
		instanceState.offset = 0;
	}

	Vector<VkDescriptorSetLayout> layouts(rendererState->swapchain->imageCount, descriptorSetLayouts[SHADER_SCOPE_INSTANCE]);

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = (U32)layouts.Size();
	allocInfo.pSetLayouts = layouts.Data();
	instanceState.descriptorSets.Resize(layouts.Size());
	VkCheck_ERROR(vkAllocateDescriptorSets(rendererState->device->logicalDevice, &allocInfo, instanceState.descriptorSets.Data()));

	return outInstanceId;
}

bool VulkanShader::ReleaseInstanceResources(RendererState* rendererState, Shader* shader, U32 instanceId)
{
	InstanceState& instanceState = instanceStates[instanceId];

	vkDeviceWaitIdle(rendererState->device->logicalDevice);

	VkCheck_ERROR(vkFreeDescriptorSets(rendererState->device->logicalDevice, descriptorPool,
		(U32)instanceState.descriptorSets.Size(), instanceState.descriptorSets.Data()));

	instanceState.instanceTextureMaps.Clear();

	uniformBuffer->Free(shader->instanceUboStride, instanceState.offset);
	instanceState.offset = INVALID_ID;
	instanceState.id = INVALID_ID;

	return true;
}

bool VulkanShader::CreateShaderModule(RendererState* rendererState, ShaderStageConfig config, ShaderStage* shaderStage)
{
	Binary* binary = Resources::LoadBinary(config.fileName);

	if (!binary)
	{
		Logger::Error("VulkanShader::CreateShaderModule: Unable to read shader module: {}", config.fileName);
		return false;
	}

	shaderStage->info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderStage->info.codeSize = binary->data.Size();
	shaderStage->info.pCode = (U32*)binary->data.Data();
	shaderStage->info.pNext = nullptr;
	shaderStage->info.flags = 0;

	VkCheck_ERROR(vkCreateShaderModule(rendererState->device->logicalDevice, &shaderStage->info, rendererState->allocator, &shaderStage->handle));

	Resources::UnloadBinary(binary);

	shaderStage->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage->shaderStageInfo.stage = config.stage;
	shaderStage->shaderStageInfo.module = shaderStage->handle;
	shaderStage->shaderStageInfo.pName = "main";
	shaderStage->shaderStageInfo.pNext = nullptr;
	shaderStage->shaderStageInfo.pSpecializationInfo = nullptr;
	shaderStage->shaderStageInfo.flags = 0;

	return true;
}

void VulkanShader::SetUniform(RendererState* rendererState, Shader* shader, Uniform& uniform, const void* value)
{
	if (uniform.type == FIELD_TYPE_SAMPLER)
	{
		if (uniform.setIndex == SHADER_SCOPE_GLOBAL) { shader->globalTextureMaps[uniform.location] = (TextureMap*)value; }
		else { instanceStates[shader->boundInstanceId].instanceTextureMaps[uniform.location] = *(TextureMap*)value; }
	}
	else
	{
		U64 offset = uniform.setIndex == SHADER_SCOPE_GLOBAL ? shader->globalUboOffset : instanceStates[shader->boundInstanceId].offset;
		Memory::Copy((U8*)mappedUniformBufferBlock + offset + uniform.offset, value, uniform.size);
	}
}

void VulkanShader::SetPushConstant(RendererState* rendererState, Shader* shader, PushConstant& pushConstant, const void* value)
{
	VkCommandBuffer commandBuffer = rendererState->graphicsCommandBuffers[rendererState->imageIndex].handle;
	vkCmdPushConstants(commandBuffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, pushConstant.offset, pushConstant.size, value);
}