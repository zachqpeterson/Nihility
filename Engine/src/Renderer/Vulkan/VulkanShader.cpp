#include "VulkanShader.hpp"

#include "VulkanRenderpass.hpp"
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanImage.hpp"

#include "Memory/Memory.hpp"
#include "Resources/Resources.hpp"

bool VulkanShader::Create(RendererState* rendererState, U8 renderpassId, U8 stageCount, const Vector<String>& stageFilenames, const Vector<ShaderStageType>& stages)
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
        ++config.descriptorSetCount;
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
    mappedUniformBufferBlock = nullptr; //TODO: What?
    uniformBuffer->Destroy(rendererState);

    pipeline->Destroy(rendererState);
    Memory::Free(pipeline, sizeof(VulkanPipeline), MEMORY_TAG_RENDERER);

    for (U32 i = 0; i < config.stageCount; ++i)
    {
        vkDestroyShaderModule(rendererState->device->logicalDevice, stages[i].handle, rendererState->allocator);
    }

    Memory::Zero(&config, sizeof(ShaderConfig));
}

bool VulkanShader::Initialize(RendererState* rendererState, Shader& shader)
{
    Memory::Zero(stages, sizeof(ShaderStage) * VULKAN_SHADER_MAX_STAGES);
    for (U32 i = 0; i < config.stageCount; ++i)
    {
        if (!CreateShaderModule(rendererState, config.stages[i], &stages[i]))
        {
            LOG_ERROR("Unable to create %s shader module for '%s'. Shader will be destroyed.", (const char*)config.stages[i].fileName, (const char*)shader.name);
            return false;
        }
    }

    static VkFormat* types = 0;
    static VkFormat t[11];
    if (!types)
    {
        t[SHADER_ATTRIB_TYPE_FLOAT32] = VK_FORMAT_R32_SFLOAT;
        t[SHADER_ATTRIB_TYPE_FLOAT32_2] = VK_FORMAT_R32G32_SFLOAT;
        t[SHADER_ATTRIB_TYPE_FLOAT32_3] = VK_FORMAT_R32G32B32_SFLOAT;
        t[SHADER_ATTRIB_TYPE_FLOAT32_4] = VK_FORMAT_R32G32B32A32_SFLOAT;
        t[SHADER_ATTRIB_TYPE_INT8] = VK_FORMAT_R8_SINT;
        t[SHADER_ATTRIB_TYPE_UINT8] = VK_FORMAT_R8_UINT;
        t[SHADER_ATTRIB_TYPE_INT16] = VK_FORMAT_R16_SINT;
        t[SHADER_ATTRIB_TYPE_UINT16] = VK_FORMAT_R16_UINT;
        t[SHADER_ATTRIB_TYPE_INT32] = VK_FORMAT_R32_SINT;
        t[SHADER_ATTRIB_TYPE_UINT32] = VK_FORMAT_R32_UINT;
        types = t;
    }

    U32 offset = 0;
    for (U32 i = 0; i < shader.attributes.Size(); ++i)
    {
        VkVertexInputAttributeDescription attribute;
        attribute.location = i;
        attribute.binding = 0;
        attribute.offset = offset;
        attribute.format = types[shader.attributes[i].type];

        config.attributes[i] = attribute;

        offset += shader.attributes[i].size;
    }

    for (U32 i = 0; i < shader.uniforms.Size(); ++i)
    {
        if (shader.uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER)
        {
            const U32 setIndex = (shader.uniforms[i].scope == SHADER_SCOPE_GLOBAL ? 0 : 1);
            DescriptorSetConfig* setConfig = &config.descriptorSets[setIndex];
            if (setConfig->bindingCount < 2)
            {
                setConfig->bindings[1].binding = 1;
                setConfig->bindings[1].descriptorCount = 1;
                setConfig->bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                setConfig->bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                ++setConfig->bindingCount;
            }
            else
            {
                setConfig->bindings[1].descriptorCount++;
            }
        }
    }

    VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = config.poolSizes;
    poolInfo.maxSets = config.maxDescriptorSetCount;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkCheck_ERROR(vkCreateDescriptorPool(rendererState->device->logicalDevice, &poolInfo, rendererState->allocator, &descriptorPool));

    Memory::Zero(descriptorSetLayouts, config.descriptorSetCount);
    for (U32 i = 0; i < config.descriptorSetCount; ++i)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        layoutInfo.bindingCount = config.descriptorSets[i].bindingCount;
        layoutInfo.pBindings = config.descriptorSets[i].bindings;
        VkCheck_ERROR(vkCreateDescriptorSetLayout(rendererState->device->logicalDevice, &layoutInfo, rendererState->allocator, &descriptorSetLayouts[i]));
    }

    // TODO: This feels wrong to have these here, at least in this fashion. Should probably
    // Be configured to pull from someplace instead.
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (F32)rendererState->framebufferHeight;
    viewport.width = (F32)rendererState->framebufferWidth;
    viewport.height = -(F32)rendererState->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = rendererState->framebufferWidth;
    scissor.extent.height = rendererState->framebufferHeight;

    VkPipelineShaderStageCreateInfo stageInfos[VULKAN_SHADER_MAX_STAGES];
    Memory::Zero(stageInfos, sizeof(VkPipelineShaderStageCreateInfo) * VULKAN_SHADER_MAX_STAGES);
    for (U32 i = 0; i < config.stageCount; ++i)
    {
        stageInfos[i] = stages[i].shaderStageInfo;
    }

    pipeline->Create(
        rendererState,
        renderpass,
        shader.attributeStride,
        shader.attributes.Size(),
        config.attributes,
        config.descriptorSetCount,
        descriptorSetLayouts,
        config.stageCount,
        stageInfos,
        viewport,
        scissor,
        false,
        true,
        shader.pushConstantRangeCount,
        shader.pushConstantRanges);

    shader.requiredUboAlignment = rendererState->device->properties.limits.minUniformBufferOffsetAlignment;

    shader.globalUboStride = AlignPow2(shader.globalUboSize, shader.requiredUboAlignment);
    shader.uboStride = AlignPow2(shader.uboSize, shader.requiredUboAlignment);

    U32 deviceLocalBits = rendererState->device->supportsDeviceLocalHostVisible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
    // TODO: max count should be configurable, or perhaps long term support of buffer resizing.
    U64 totalBufferSize = shader.globalUboStride + (shader.uboStride * VULKAN_MAX_MATERIAL_COUNT);
    uniformBuffer->Create(
        rendererState, totalBufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
        (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | deviceLocalBits),
        true, true);

    uniformBuffer->Allocate(shader.globalUboStride, &shader.globalUboOffset);

    mappedUniformBufferBlock = uniformBuffer->LockMemory(rendererState, 0, VK_WHOLE_SIZE, 0);

    VkDescriptorSetLayout globalLayouts[3] = {
        descriptorSetLayouts[0],
        descriptorSetLayouts[0],
        descriptorSetLayouts[0] };

    VkDescriptorSetAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    alloc_info.descriptorPool = descriptorPool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = globalLayouts;
    VkCheck_ERROR(vkAllocateDescriptorSets(rendererState->device->logicalDevice, &alloc_info, globalDescriptorSets));

    return true;
}

bool VulkanShader::Use(RendererState* rendererState)
{
    pipeline->Bind(rendererState->graphicsCommandBuffers[rendererState->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS);
    return true;
}

bool VulkanShader::BindGlobals(RendererState* rendererState)
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

    VkWriteDescriptorSet uboWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    uboWrite.dstSet = globalDescriptorSets[imageIndex];
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0] = uboWrite;

    U32 globalSetBindingCount = config.descriptorSets[0].bindingCount;
    if (globalSetBindingCount > 1)
    {
        // TODO: There are samplers to be written. Support this.
        globalSetBindingCount = 1;
        LOG_ERROR("Global image samplers are not yet supported.");

        // VkWriteDescriptorSet sampler_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        // descriptorWrites[1] = ...
    }

    vkUpdateDescriptorSets(rendererState->device->logicalDevice, globalSetBindingCount, descriptorWrites, 0, 0);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1, &globalDescriptor, 0, 0);
    return true;
}

bool VulkanShader::BindInstance(RendererState* rendererState, U32 instanceId)
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
                Texture* t = instanceStates[boundInstanceId].instanceTextures[i];
                VulkanTexture* internalData = (VulkanTexture*)t->internalData;
                imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[i].imageView = internalData->image->view;
                imageInfos[i].sampler = internalData->sampler;

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

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 1, 1, &objectDescriptorSet, 0, 0);
    return true;
}

U32 VulkanShader::AcquireInstanceResources(RendererState* rendererState)
{
    // TODO: dynamic
    U32 outInstanceId = INVALID_ID;
    for (U32 i = 0; i < 1024; ++i)
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
        LOG_ERROR("vulkan_shader_acquire_instance_resources failed to acquire new id");
        return INVALID_ID;
    }

    InstanceState* instanceState = &instanceStates[outInstanceId];
    U32 instanceTextureCount = config.descriptorSets[1].bindings[1].descriptorCount;

    instanceState->instanceTextures.Resize(instanceTextureCount);
    Texture* defaultTexture = Resources::DefaultTexture();

    for (U32 i = 0; i < instanceTextureCount; ++i)
    {
        instanceState->instanceTextures[i] = defaultTexture;
    }

    U64 size = uboStride;

    if (!uniformBuffer->Allocate(size, &instanceState->offset))
    {
        LOG_ERROR("vulkan_material_shader_acquire_resources failed to acquire ubo space");
        return INVALID_ID;
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

    return outInstanceId;
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

bool VulkanShader::SetUniform(RendererState* rendererState, Shader& shader, const ShaderUniform& uniform, const void* value)
{
    if (uniform.type == SHADER_UNIFORM_TYPE_SAMPLER)
    {
        if (uniform.scope == SHADER_SCOPE_GLOBAL)
        {
            shader.globalTextures[uniform.location] = (Texture*)value;
        }
        else
        {
            instanceStates[boundInstanceId].instanceTextures[uniform.location] = (Texture*)value;
        }
    }
    else
    {
        if (uniform.scope == SHADER_SCOPE_LOCAL)
        {
            VkCommandBuffer command_buffer = rendererState->graphicsCommandBuffers[rendererState->imageIndex].handle;
            vkCmdPushConstants(command_buffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, uniform.offset, uniform.size, value);
        }
        else
        {
            U64 addr = (U64)mappedUniformBufferBlock;
            addr += boundUboOffset + uniform.offset;
            Memory::Copy((void*)addr, value, uniform.size);
            if (addr)
            {
                //TODO:
            }
        }
    }
    return true;
}

bool VulkanShader::CreateShaderModule(RendererState* rendererState, ShaderStageConfig config, ShaderStage* shaderStage)
{
    Binary* binary = Resources::LoadBinary(config.fileName);

    if (!binary)
    {
        LOG_ERROR("Unable to read shader module: %s.", (const char*)config.fileName);
        return false;
    }

    Memory::Zero(&shaderStage->info, sizeof(VkShaderModuleCreateInfo));
    shaderStage->info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderStage->info.codeSize = binary->data.Size();
    shaderStage->info.pCode = (U32*)binary->data.Data();

    VkCheck(vkCreateShaderModule(
        rendererState->device->logicalDevice,
        &shaderStage->info,
        rendererState->allocator,
        &shaderStage->handle));

    Resources::UnloadBinary(binary);

    Memory::Zero(&shaderStage->shaderStageInfo, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStage->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage->shaderStageInfo.stage = config.stage;
    shaderStage->shaderStageInfo.module = shaderStage->handle;
    shaderStage->shaderStageInfo.pName = "main";

    return true;
}