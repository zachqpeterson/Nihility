#include "Pipeline.hpp"

#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Platform.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"

Dependancy::Dependancy(DependancyType type) : type{ type } {}
Dependancy::Dependancy(const ResourceRef<Pipeline>& pipeline, DependancyType type, U8 index) : pipeline{ pipeline }, type{ type }, index{ index } {}

bool Pipeline::Create(U8 pushConstantCount, PushConstant* pushConstants)
{
	if (shaders.Size() == 0) { Logger::Error("Pipelines Must Have Stages!"); return false; }

	U32 stages = 0;

	VkDescriptorSetLayoutBinding bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8 bindingCount{ 0 };

	for (ResourceRef<Shader>& shader : shaders)
	{
		stages |= shader->stage;
		useVertices |= shader->useVertices;
		useInstances |= shader->useInstancing;
		useBindless |= shader->useBindless;

		switch (shader->stage)
		{
		case SHADER_STAGE_VERTEX_BIT: {
			instanceStride = shader->instanceStride;
			vertexBindingCount = shader->vertexBindingCount;
			Memory::Copy(vertexTypes, shader->vertexTypes, sizeof(VertexType) * vertexBindingCount);

			if (bindPoint != PIPELINE_BIND_POINT_MAX_ENUM && bindPoint != PIPELINE_BIND_POINT_GRAPHICS)
			{
				Logger::Error("Pipelines Can't Have Multiple Bindpoints!");
				Destroy();
				return false;
			}

			bindPoint = PIPELINE_BIND_POINT_GRAPHICS;
		} break;
		case SHADER_STAGE_FRAGMENT_BIT: {
			outputCount = shader->outputCount;
			if (shader->subpass.inputAttachmentCount) { subpass = shader->subpass; }

			if (bindPoint != PIPELINE_BIND_POINT_MAX_ENUM && bindPoint != PIPELINE_BIND_POINT_GRAPHICS)
			{
				Logger::Error("Pipelines Can't Have Multiple Bindpoints!");
				Destroy();
				return false;
			}

			bindPoint = PIPELINE_BIND_POINT_GRAPHICS;
		} break;
		case SHADER_STAGE_COMPUTE_BIT: {
			localSizeX = shader->localSizeX;
			localSizeY = shader->localSizeY;
			localSizeZ = shader->localSizeZ;

			if (bindPoint != PIPELINE_BIND_POINT_MAX_ENUM && bindPoint != PIPELINE_BIND_POINT_COMPUTE)
			{
				Logger::Error("Pipelines Can't Have Multiple Bindpoints!");
				Destroy();
				return false;
			}

			bindPoint = PIPELINE_BIND_POINT_COMPUTE;
		} break;
		}

		if (shader->usePushConstants)
		{
			pushConstantStages |= shader->stage;
		}

		for (U32 i = 0; i < shader->GetBindingCount(); ++i)
		{
			VkDescriptorSetLayoutBinding binding = shader->GetBinding(i);

			if (binding.stageFlags)
			{
				U32 stages = bindings[i].stageFlags;

				bindings[i] = binding;
				bindings[i].stageFlags |= stages;
			}
		}

		bindingCount = Math::Max(bindingCount, shader->GetBindingCount());
	}

	if (vertexCount && useVertices)
	{
		Logger::Error("Shaders That Use A Vertex Buffer Can't Specify A Vertex Count!");
		Destroy();
		return false;
	}

	if (vertexCount == 0 && !useVertices)
	{
		Logger::Error("Shaders That Don't Use A Vertex Buffer Must Specify A Vertex Count!");
		Destroy();
		return false;
	}

	if ((bool)(stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != (bool)(stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
	{
		Logger::Error("Shaders With Tessellation Must Have Both Tessellation Stages!");
		Destroy();
		return false;
	}

	if (bindPoint == PIPELINE_BIND_POINT_GRAPHICS && ((stages & VK_SHADER_STAGE_VERTEX_BIT) == 0 || (stages & VK_SHADER_STAGE_FRAGMENT_BIT) == 0))
	{
		Logger::Error("Shaders With Graphics Must Have Both A Vertex And Fragment Stage!");
		Destroy();
		return false;
	}

	VkDescriptorSetLayout layouts[2]{};

	if (bindingCount)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = Renderer::pushDescriptorsSupported ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
		layoutInfo.bindingCount = bindingCount;
		layoutInfo.pBindings = bindings;

		if (vkCreateDescriptorSetLayout(Renderer::device, &layoutInfo, Renderer::allocationCallbacks, &descriptorSetLayout) != VK_SUCCESS)
		{
			Logger::Error("Failed To Create Descriptor Set Layout!");
			Destroy();
			return false;
		}

		Renderer::SetResourceName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)descriptorSetLayout, name + "_dsl");

		layouts[0] = descriptorSetLayout;
	}
	else
	{
		layouts[0] = Shader::dummyDescriptorSetLayout;
	}

	if (Renderer::bindlessSupported && useBindless)
	{
		layouts[1] = Shader::bindlessDescriptorLayout;
	}

	U32 pushContantSize = 0;

	this->pushConstantCount = pushConstantCount;

	VkPushConstantRange pushRange{};
	pushRange.stageFlags = pushConstantStages;
	pushRange.offset = 0;

	for (U32 i = 0; i < pushConstantCount; ++i)
	{
		pushRange.size += pushConstants[i].size;

		this->pushConstants[i] = pushConstants[i];

		pushContantSize += pushConstants[i].size;
	}

	if (pushContantSize > MAX_PUSH_CONSTANT_SIZE)
	{
		Logger::Error("Total Push Constant Size Exceeds Limit Of 128 Bytes!");
		Destroy();
		return false;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.pNext = nullptr;
	pipelineLayoutInfo.pSetLayouts = layouts;
	pipelineLayoutInfo.setLayoutCount = useBindless ? 2 : bindingCount;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantStages > 0; //TODO: Support multiple ranges
	pipelineLayoutInfo.pPushConstantRanges = &pushRange;

	VkValidate(vkCreatePipelineLayout(Renderer::device, &pipelineLayoutInfo, Renderer::allocationCallbacks, &pipelineLayout));

	if (bindingCount)
	{
		VkDescriptorUpdateTemplateEntry entries[MAX_DESCRIPTORS_PER_SET]{};

		for (U32 i = 0; i < bindingCount; ++i)
		{
			VkDescriptorSetLayoutBinding& binding = bindings[i];
			VkDescriptorUpdateTemplateEntry& entry = entries[i];

			entry.dstBinding = binding.binding;
			entry.dstArrayElement = 0;
			entry.descriptorCount = binding.descriptorCount;
			entry.descriptorType = binding.descriptorType;
			entry.offset = sizeof(Descriptor) * i;
			entry.stride = sizeof(Descriptor);
		}

		VkDescriptorUpdateTemplateCreateInfo descriptorTemplateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO };
		descriptorTemplateInfo.pNext = nullptr;
		descriptorTemplateInfo.flags = 0;
		descriptorTemplateInfo.templateType = Renderer::pushDescriptorsSupported ? VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR : VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
		descriptorTemplateInfo.descriptorSetLayout = Renderer::pushDescriptorsSupported ? 0 : descriptorSetLayout;
		descriptorTemplateInfo.pipelineBindPoint = (VkPipelineBindPoint)bindPoint;
		descriptorTemplateInfo.pipelineLayout = pipelineLayout;
		descriptorTemplateInfo.descriptorUpdateEntryCount = bindingCount;
		descriptorTemplateInfo.pDescriptorUpdateEntries = entries;
		descriptorTemplateInfo.set = 0;

		if (vkCreateDescriptorUpdateTemplate(Renderer::device, &descriptorTemplateInfo, Renderer::allocationCallbacks, &updateTemplate) != VK_SUCCESS)
		{
			Logger::Error("Failed To Create Descriptor Update Template!");
			Destroy();
			return false;
		}
	}

	return true;
}

void Pipeline::Build(Renderpass* renderpass)
{
	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); pipeline = nullptr; }

	VkPipelineCache pipelineCache = nullptr;
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

	String cachePath = name + ".cache";

	bool cacheExists = false;
	File cache(cachePath, FILE_OPEN_RESOURCE_READ);

	if (cache.Opened())
	{
		U8* data = nullptr;
		U32 size = cache.ReadAll(&data);

		VkPipelineCacheHeaderVersionOne* cacheHeader = (VkPipelineCacheHeaderVersionOne*)data;

		const VkPhysicalDeviceProperties& deviceProperties = Renderer::GetDeviceProperties();

		if (cacheHeader->deviceID == deviceProperties.deviceID &&
			cacheHeader->vendorID == deviceProperties.vendorID &&
			memcmp(cacheHeader->pipelineCacheUUID, deviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0)
		{
			pipelineCacheCreateInfo.initialDataSize = size;
			pipelineCacheCreateInfo.pInitialData = data;
			cacheExists = true;
		}

		VkValidate(vkCreatePipelineCache(Renderer::device, &pipelineCacheCreateInfo, Renderer::allocationCallbacks, &pipelineCache));

		cache.Close();
	}
	else
	{
		VkValidate(vkCreatePipelineCache(Renderer::device, &pipelineCacheCreateInfo, Renderer::allocationCallbacks, &pipelineCache));
	}

	if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineShaderStageCreateInfo shaderStages[5];

		for (ResourceRef<Shader>& shader : shaders)
		{
			if (shader->stage == SHADER_STAGE_VERTEX_BIT)
			{
				shader->FillOutVertexInfo(vertexInputInfo);
			}

			shaderStages[pipelineInfo.stageCount++] = shader->GetShaderInfo();
		}

		pipelineInfo.pStages = shaderStages;

		VkPipelineColorBlendAttachmentState blendState{};

		switch (blendMode)
		{
		case BLEND_MODE_ADD: {
			blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendState.colorBlendOp = VK_BLEND_OP_ADD;

			blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendState.alphaBlendOp = VK_BLEND_OP_ADD;

			blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendState.blendEnable = VK_TRUE;
		} break;
		case BLEND_MODE_SUB: {
			blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendState.colorBlendOp = VK_BLEND_OP_SUBTRACT;

			blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

			blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendState.blendEnable = VK_TRUE;
		} break;
		case BLEND_MODE_NONE: {
			blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendState.blendEnable = VK_FALSE;
		} break;
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.pNext = nullptr;
		inputAssembly.flags = 0;
		inputAssembly.topology = (VkPrimitiveTopology)topologyMode;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &blendState;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilInfo.pNext = nullptr;
		depthStencilInfo.flags = 0;
		depthStencilInfo.depthWriteEnable = depthWriteEnable;
		depthStencilInfo.stencilTestEnable = stencilEnable;
		depthStencilInfo.depthTestEnable = depthEnable;
		depthStencilInfo.depthCompareOp = (VkCompareOp)depthComparison;
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;
		if (stencilEnable) { Logger::Error("Stencil Buffers Not Yet Supported!"); }

		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE; //TODO: look into this
		rasterizer.polygonMode = (VkPolygonMode)fillMode;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = (VkCullModeFlags)cullMode;
		rasterizer.frontFace = (VkFrontFace)frontMode;
		rasterizer.depthBiasEnable = depthBiasEnable;
		rasterizer.depthBiasConstantFactor = depthBiasConstant;
		rasterizer.depthBiasClamp = depthBiasClamp;
		rasterizer.depthBiasSlopeFactor = depthBiasSlope;

		VkViewport viewport{};
		VkRect2D scissor{};

		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkDynamicState dynamicStates[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = CountOf32(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;

		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.renderPass = renderpass->renderpass;
		pipelineInfo.subpass = subpassIndex;

		vkCreateGraphicsPipelines(Renderer::device, pipelineCache, 1, &pipelineInfo, Renderer::allocationCallbacks, &pipeline);
	}
	else
	{
		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;
		pipelineInfo.stage = shaders[0]->GetShaderInfo();
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.basePipelineHandle = nullptr;
		pipelineInfo.basePipelineIndex = 0;

		vkCreateComputePipelines(Renderer::device, pipelineCache, 1, &pipelineInfo, Renderer::allocationCallbacks, &pipeline);
	}

	if (!cacheExists)
	{
		cache.Open(cachePath, FILE_OPEN_RESOURCE_WRITE);

		U64 cacheDataSize = 0;
		VkValidate(vkGetPipelineCacheData(Renderer::device, pipelineCache, &cacheDataSize, nullptr));

		void* cacheData;
		Memory::AllocateSize(&cacheData, cacheDataSize);
		VkValidate(vkGetPipelineCacheData(Renderer::device, pipelineCache, &cacheDataSize, cacheData));

		cache.Write(cacheData, (U32)cacheDataSize);
		Memory::Free(&cacheData);

		cache.Close();
	}

	vkDestroyPipelineCache(Renderer::device, pipelineCache, Renderer::allocationCallbacks);
}

void Pipeline::Destroy()
{
	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); pipeline = nullptr; }
	if (pipelineLayout) { vkDestroyPipelineLayout(Renderer::device, pipelineLayout, Renderer::allocationCallbacks); pipelineLayout = nullptr; }

	if (descriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(Renderer::device, descriptorSetLayout, Renderer::allocationCallbacks);

		descriptorSetLayout = nullptr;
	}

	if (updateTemplate)
	{
		vkDestroyDescriptorUpdateTemplate(Renderer::device, updateTemplate, Renderer::allocationCallbacks);
		updateTemplate = nullptr;
	}
}

void Pipeline::SetBuffers(Buffer* buffers, Buffer instanceBuffer)
{
	for (; vertexBufferCount < vertexBindingCount; ++vertexBufferCount)
	{
		if (vertexTypes[vertexBufferCount] == VERTEX_TYPE_INSTANCE) { vertexBuffers[vertexBufferCount] = instanceBuffer.vkBuffer; vertexBufferOffsets[vertexBufferCount] = 0; }
		else { vertexBuffers[vertexBufferCount] = buffers[vertexTypes[vertexBufferCount]].vkBuffer; vertexBufferOffsets[vertexBufferCount] = 0; }
	}
}

void Pipeline::AddDescriptor(const Descriptor& descriptor)
{
	descriptors[descriptorCount++] = descriptor;
}

void Pipeline::AddDependancy(const Dependancy& dependancy)
{
	dependancies[dependancyCount++] = dependancy;
}

void Pipeline::Run(CommandBuffer* commandBuffer, Buffer& indexBuffer, Buffer& drawsBuffer, Buffer& countsBuffer)
{
	//TODO: Task Submitting

	commandBuffer->BindPipeline(this);

	PushDescriptors(commandBuffer);
	PushConstants(commandBuffer);

	switch (bindPoint)
	{
	case VK_PIPELINE_BIND_POINT_GRAPHICS: {
		if (useVertices)
		{
			commandBuffer->BindVertexBuffers(vertexBufferCount, vertexBuffers, vertexBufferOffsets);

			if (useIndices)
			{
				commandBuffer->BindIndexBuffer(indexBuffer.vkBuffer, 0);

				commandBuffer->DrawIndexedIndirectCount(drawsBuffer.vkBuffer, countsBuffer.vkBuffer, drawOffset, countOffset);
			}
			else
			{
				commandBuffer->DrawIndirectCount(drawsBuffer.vkBuffer, countsBuffer.vkBuffer, drawOffset, countOffset);
			}
		}
		else { commandBuffer->Draw(0, vertexCount, 0, 1); }
	} break;
	case VK_PIPELINE_BIND_POINT_COMPUTE: {
		commandBuffer->Dispatch(localSizeX, localSizeY, localSizeZ);
	} break;
	case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
		//TODO:
	} break;
	}
}

void Pipeline::PushDescriptors(CommandBuffer* commandBuffer)
{
	if (Renderer::pushDescriptorsSupported)
	{
		//vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer->commandBuffer, setLayouts[0]->updateTemplate, pipelineLayout, 0, descriptors);
	}
	else
	{
		if (descriptorCount)
		{
			VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

			allocateInfo.descriptorPool = Renderer::descriptorPools[Renderer::frameIndex];
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &descriptorSetLayout;

			VkDescriptorSet set;
			vkAllocateDescriptorSets(Renderer::device, &allocateInfo, &set);
			vkUpdateDescriptorSetWithTemplate(Renderer::device, set, updateTemplate, descriptors);

			if (useBindless)
			{
				VkDescriptorSet sets[]{ set, Shader::bindlessDescriptorSet };

				commandBuffer->BindDescriptorSets((VkPipelineBindPoint)bindPoint, pipelineLayout, 0, 2, sets);
			}
			else
			{
				commandBuffer->BindDescriptorSets((VkPipelineBindPoint)bindPoint, pipelineLayout, 0, 1, &set);
			}
		}
		else if(useBindless)
		{
			commandBuffer->BindDescriptorSets((VkPipelineBindPoint)bindPoint, pipelineLayout, 1, 1, &Shader::bindlessDescriptorSet);
		}
	}
}

void Pipeline::PushConstants(CommandBuffer* commandBuffer)
{
	for (U32 i = 0; i < pushConstantCount; ++i)
	{
		commandBuffer->PushConstants(pipelineLayout, pushConstantStages, pushConstants[i].offset, pushConstants[i].size, pushConstants[i].data);
	}
}
