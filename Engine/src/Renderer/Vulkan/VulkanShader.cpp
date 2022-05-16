#include "VulkanShader.hpp"

#include "VulkanRenderpass.hpp"
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"

#include "Memory/Memory.hpp"

bool VulkanShader::Create(RendererState* rendererState, U8 renderpassId, U8 stageCount, Vector<String> stageFilenames, Vector<ShaderStageType> stages)
{
    // TODO: Dynamic renderpasses
    renderpass = renderpassId == 1 ? rendererState->mainRenderpass : rendererState->uiRenderpass;

    VkShaderStageFlags vkStages[VULKAN_SHADER_MAX_STAGES];
    for (U8 i = 0; i < stageCount; ++i)
    {
        switch (stages[i])
        {
        case SHADER_STAGE_FRAGMENT:
            vkStages[i] = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case SHADER_STAGE_VERTEX:
            vkStages[i] = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case SHADER_STAGE_GEOMETRY:
            LOG_WARN("vulkan_renderer_shader_create: VK_SHADER_STAGE_GEOMETRY_BIT is set but not yet supported.");
            vkStages[i] = VK_SHADER_STAGE_GEOMETRY_BIT;
            break;
        case SHADER_STAGE_COMPUTE:
            LOG_WARN("vulkan_renderer_shader_create: SHADER_STAGE_COMPUTE is set but not yet supported.");
            vkStages[i] = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        default:
            LOG_ERROR("Unsupported stage type: %d", stages[i]);
            break;
        }
    }

    // TODO: Configurable max descriptor allocate count.
    U32 maxDescriptorAllocateCount = 1024;

    config.maxDescriptorSetCount = maxDescriptorAllocateCount;

    Memory::Zero(config.stages, sizeof(ShaderStageConfig) * VULKAN_SHADER_MAX_STAGES);
    config.stageCount = 0;

    for (U32 i = 0; i < stageCount; ++i)
    {
        if (config.stageCount + 1 > VULKAN_SHADER_MAX_STAGES)
        {
            LOG_ERROR("Shaders may have a maximum of %d stages", VULKAN_SHADER_MAX_STAGES);
            return false;
        }

        VkShaderStageFlagBits stageFlag;
        switch (stages[i])
        {
        case SHADER_STAGE_VERTEX:
            stageFlag = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case SHADER_STAGE_FRAGMENT:
            stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        default:
            LOG_ERROR("vulkan_shader_create: Unsupported shader stage flagged: %d. Stage ignored.", stages[i]);
            continue;
        }

        config.stages[config.stageCount].stage = stageFlag;
        config.stages[config.stageCount].fileName = stageFilenames[i];
        ++config.stageCount;
    }

    Memory::Zero(config.descriptorSets, sizeof(DescriptorSetConfig) * 2);
    Memory::Zero(config.attributes, sizeof(VkVertexInputAttributeDescription) * VULKAN_SHADER_MAX_ATTRIBUTES);

    config.poolSizes[0] = (VkDescriptorPoolSize){ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 };          // HACK: max number of ubo descriptor sets.
    config.poolSizes[1] = (VkDescriptorPoolSize){ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096 };  // HACK: max number of image sampler descriptor sets.

    DescriptorSetConfig globalDescriptorSetConfig = {};

    globalDescriptorSetConfig.bindings[0].binding = 0;
    globalDescriptorSetConfig.bindings[0].descriptorCount = 1;
    globalDescriptorSetConfig.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalDescriptorSetConfig.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ++globalDescriptorSetConfig.bindingCount;

    config.descriptorSets[0] = globalDescriptorSetConfig;
    ++config.descriptorSetCount;
    if (useInstances)
    {
        DescriptorSetConfig instanceDescriptorSetConfig = {};

        // NOTE: Might be a good idea to only add this if it is going to be used...
        instanceDescriptorSetConfig.bindings[0].binding = 0;
        instanceDescriptorSetConfig.bindings[0].descriptorCount = 1;
        instanceDescriptorSetConfig.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        instanceDescriptorSetConfig.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        ++instanceDescriptorSetConfig.bindingCount;

        config.descriptorSets[1] = instanceDescriptorSetConfig;
        config.descriptorSetCount++;
    }

    // TODO: Dynamic
    for (U32 i = 0; i < 1024; ++i)
    {
        instanceStates[i].id = INVALID_ID;
    }

    return true;
}

void VulkanShader::Destroy(RendererState* rendererState)
{
    for (U32 i = 0; i < config.descriptorSetCount; ++i)
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

    uniformBuffer->UnlockMemory(rendererState);
    mappedUniformBufferBlock = 0; //TODO: What?
    uniformBuffer->Destroy(rendererState);

    //pipeline->Destroy(rendererState);

    for (U32 i = 0; i < config.stageCount; ++i)
    {
        vkDestroyShaderModule(rendererState->device->logicalDevice, stages[i].handle, rendererState->allocator);
    }

    Memory::Zero(&config, sizeof(ShaderConfig));
}

bool VulkanShader::Initialize()
{
    
}

bool VulkanShader::Use(RendererState* rendererState)
{
    //pipeline.Bind(&rendererState->graphicsCommandBuffers[rendererState->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS);
    return true;
}

bool VulkanShader::BindGlobals()
{
    boundUboOffset = globalUboOffset;
    return true;
}

bool VulkanShader::ApplyGlobals(RendererState* rendererState)
{
    U32 imageIndex = rendererState->imageIndex;
    VkCommandBuffer commandBuffer = rendererState->graphicsCommandBuffers[imageIndex].handle;
    VkDescriptorSet globalDescriptor = globalDescriptorSets[imageIndex];

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer->handle;
    bufferInfo.offset = globalUboOffset;
    bufferInfo.range = globalUboStride;

    VkWriteDescriptorSet uboWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    uboWrite.dstSet = globalDescriptorSets[imageIndex];
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0] = uboWrite;

    U32 globalSetBindingCount = config.descriptorSets[0].bindingCount;
    if (globalSetBindingCount > 1) {
        // TODO: There are samplers to be written. Support this.
        globalSetBindingCount = 1;
        LOG_ERROR("Global image samplers are not yet supported.");

        // VkWriteDescriptorSet sampler_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        // descriptorWrites[1] = ...
    }

    vkUpdateDescriptorSets(rendererState->device->logicalDevice, globalSetBindingCount, descriptorWrites, 0, 0);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, 1, &globalDescriptor, 0, 0);
    return true;
}

bool VulkanShader::BindInstance(U32 instanceId)
{
    boundInstanceId = instanceId;
    boundUboOffset = instanceStates[instanceId].offset;
    return true;
}

bool VulkanShader::ApplyInstance(RendererState* rendererState, bool needsUpdate)
{
    if (!useInstances)
    {
        LOG_ERROR("This shader does not use instances.");
        return false;
    }

    U32 imageIndex = rendererState->imageIndex;
    VkCommandBuffer command_buffer = rendererState->graphicsCommandBuffers[imageIndex].handle;

    InstanceState* objectState = &instanceStates[boundInstanceId];
    VkDescriptorSet objectDescriptorSet = objectState->descriptorSetState.descriptorSets[imageIndex];

    if (needsUpdate)
    {
        VkWriteDescriptorSet descriptorWrites[2];
        Memory::Zero(descriptorWrites, sizeof(VkWriteDescriptorSet) * 2);
        U32 descriptorCount = 0;
        U32 descriptorIndex = 0;

        U8* instanceUboGeneration = &(objectState->descriptorSetState.descriptorStates[descriptorIndex].generations[imageIndex]);
        // TODO: determine if update is required.
        if (*instanceUboGeneration == INVALID_ID_U8 /*|| *global_ubo_generation != material->generation*/)
        {
            VkDescriptorBufferInfo buffer_info;
            buffer_info.buffer = uniformBuffer->handle;
            buffer_info.offset = objectState->offset;
            buffer_info.range = uboStride;

            VkWriteDescriptorSet uboDescriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            uboDescriptor.dstSet = objectDescriptorSet;
            uboDescriptor.dstBinding = descriptorIndex;
            uboDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboDescriptor.descriptorCount = 1;
            uboDescriptor.pBufferInfo = &buffer_info;

            descriptorWrites[descriptorCount] = uboDescriptor;
            ++descriptorCount;

            *instanceUboGeneration = 1;  // material->generation; TODO: some generation from... somewhere
        }
        ++descriptorIndex;

        if (config.descriptorSets[1].bindingCount > 1)
        {
            U32 totalSamplerCount = config.descriptorSets[1].bindings[1].descriptorCount;
            U32 updateSamplerCount = 0;
            VkDescriptorImageInfo imageInfos[VULKAN_SHADER_MAX_GLOBAL_TEXTURES];
            for (U32 i = 0; i < totalSamplerCount; ++i)
            {
                // TODO: only update in the list if actually needing an update.
                //Texture* t = instanceStates[boundInstanceId].instanceTextures[i];
                //vulkan_texture_data* internal_data = (vulkan_texture_data*)t->internal_data;
                //imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                //imageInfos[i].imageView = internal_data->image.view;
                //imageInfos[i].sampler = internal_data->sampler;

                // TODO: change up descriptor state to handle this properly.
                // Sync frame generation if not using a default texture.
                // if (t->generation != INVALID_ID) {
                //     *descriptor_generation = t->generation;
                //     *descriptor_id = t->id;
                // }

                updateSamplerCount++;
            }

            VkWriteDescriptorSet sampler_descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            sampler_descriptor.dstSet = objectDescriptorSet;
            sampler_descriptor.dstBinding = descriptorIndex;
            sampler_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            sampler_descriptor.descriptorCount = updateSamplerCount;
            sampler_descriptor.pImageInfo = imageInfos;

            descriptorWrites[descriptorCount] = sampler_descriptor;
            descriptorCount++;
        }

        if (descriptorCount > 0)
        {
            vkUpdateDescriptorSets(rendererState->device->logicalDevice, descriptorCount, descriptorWrites, 0, 0);
        }
    }

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 1, 1, &objectDescriptorSet, 0, 0);
    return true;
}

bool VulkanShader::AcquireInstanceResources(RendererState* rendererState, U32* outInstanceId)
{
    // TODO: dynamic
    *outInstanceId = INVALID_ID;
    for (U32 i = 0; i < 1024; ++i)
    {
        if (instanceStates[i].id == INVALID_ID)
        {
            instanceStates[i].id = i;
            *outInstanceId = i;
            break;
        }
    }

    if (*outInstanceId == INVALID_ID)
    {
        LOG_ERROR("vulkan_shader_acquire_instance_resources failed to acquire new id");
        return false;
    }

    InstanceState* instanceState = &instanceStates[*outInstanceId];
    U32 instanceTextureCount = config.descriptorSets[1].bindings[1].descriptorCount;
    
    instanceState->instanceTextures.Resize(instanceTextureCount);
    //Texture* defaultTexture = texture_system_get_default_texture();
    
    for (U32 i = 0; i < instanceTextureCount; ++i)
    {
        //instanceState->instanceTextures[i] = defaultTexture;
    }

    U64 size = uboStride;
    

    if (!uniformBuffer->Allocate(size, &instanceState->offset))
    {
        LOG_ERROR("vulkan_material_shader_acquire_resources failed to acquire ubo space");
        return false;
    }

    DescriptorSetState* setState = &instanceState->descriptorSetState;

    U32 bindingCount = config.descriptorSets[1].bindingCount;
    Memory::Zero(setState->descriptorStates, sizeof(DescriptorState) * VULKAN_SHADER_MAX_BINDINGS);
    for (U32 i = 0; i < bindingCount; ++i)
    {
        for (U32 j = 0; j < 3; ++j)
        {
            setState->descriptorStates[i].generations[j] = INVALID_ID_U8;
            setState->descriptorStates[i].ids[j] = INVALID_ID;
        }
    }

    VkDescriptorSetLayout layouts[3] = {
        descriptorSetLayouts[1],
        descriptorSetLayouts[1],
        descriptorSetLayouts[1] };

    VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 3;
    allocInfo.pSetLayouts = layouts;
    VkCheck_ERROR(vkAllocateDescriptorSets(rendererState->device->logicalDevice, &allocInfo, instanceState->descriptorSetState.descriptorSets));

    return true;
}

bool VulkanShader::ReleaseInstanceResources(RendererState* rendererState, U32 instanceId)
{
    InstanceState* instanceState = &instanceStates[instanceId];

    vkDeviceWaitIdle(rendererState->device->logicalDevice);

    VkCheck_ERROR(vkFreeDescriptorSets(rendererState->device->logicalDevice, descriptorPool,
        3, instanceState->descriptorSetState.descriptorSets));

    Memory::Zero(instanceState->descriptorSetState.descriptorStates, sizeof(DescriptorSetState) * VULKAN_SHADER_MAX_BINDINGS);

    instanceState->instanceTextures.Clear();

    uniformBuffer->Free(uboStride, instanceState->offset);
    instanceState->offset = INVALID_ID;
    instanceState->id = INVALID_ID;

    return true;
}

bool VulkanShader::SetUniform(RendererState* rendererState, ShaderUniform* uniform, const void* value)
{
    if (uniform->type == SHADER_UNIFORM_TYPE_SAMPLER)
    {
        if (uniform->scope == SHADER_SCOPE_GLOBAL)
        {
            globalTextures[uniform->location] = (Texture*)value;
        }
        else
        {
            instanceStates[boundInstanceId].instanceTextures[uniform->location] = (Texture*)value;
        }
    }
    else
    {
        if (uniform->scope == SHADER_SCOPE_LOCAL)
        {
            VkCommandBuffer command_buffer = rendererState->graphicsCommandBuffers[rendererState->imageIndex].handle;
            vkCmdPushConstants(command_buffer, pipeline->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, uniform->offset, uniform->size, value);
        }
        else
        {
            U64 addr = (U64)mappedUniformBufferBlock;
            addr += boundUboOffset + uniform->offset;
            kcopy_memory((void*)addr, value, uniform->size);
            if (addr)
            {
                //TODO:
            }
        }
    }
    return true;
}

bool VulkanShader::CreateModule(ShaderStageConfig config, ShaderStage* shaderStage)
{

}
