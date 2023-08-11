#include "Pipeline.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Platform.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"

//void Shader::SetSpecializationData(const SpecializationData& data)
//{
//	Memory::Copy(specializationInfos[data.stage].specializationBuffer + specializationInfos[data.stage].specializationData[data.index].offset,
//		data.data, specializationInfos[data.stage].specializationData[data.index].size);
//}

bool Pipeline::Create(Shader* shader, Renderpass* renderpass)
{
	this->shader = shader;

	RenderpassCreation renderPassInfo{};
	renderPassInfo.colorOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	renderPassInfo.depthOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	renderPassInfo.stencilOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	renderPassInfo.width = Settings::WindowWidth();
	renderPassInfo.height = Settings::WindowHeight();

	descriptorSetCount = shader->setCount;

	for (U8 i = 0; i < descriptorSetCount; ++i)
	{
		for (U8 j = 0; j < MAX_SWAPCHAIN_IMAGES; ++j)
		{
			descriptorSets[j][i] = Resources::CreateDescriptorSet(shader->setLayouts[i]);
		}
	}

	if (renderpass)
	{
		this->renderpass = renderpass;
	}
	else
	{
		String textureName = name + "_output_";
		renderPassInfo.SetName(name + "_pass");

		for (U32 i = 0; i < shader.outputCount; ++i)
		{
			TextureCreation textureInfo{};
			textureInfo.name = textureName + i;
			textureInfo.format = shader.outputs[i].format;
			textureInfo.width = renderPassInfo.width;
			textureInfo.height = renderPassInfo.height;
			textureInfo.depth = 1;
			textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET_MASK;
			textureInfo.type = VK_IMAGE_TYPE_2D;

			Texture* texture = Resources::CreateTexture(textureInfo);
			renderPassInfo.AddRenderTarget(texture);
			//TODO: Add clear colors if not equal to render targets
		}

		if (useDepth)
		{
			TextureCreation textureInfo{};
			textureInfo.name = textureName + "depth";
			textureInfo.format = VK_FORMAT_D32_SFLOAT;
			textureInfo.width = renderPassInfo.width;
			textureInfo.height = renderPassInfo.height;
			textureInfo.depth = 1;
			textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET_MASK;
			textureInfo.type = VK_IMAGE_TYPE_2D;

			Texture* texture = Resources::CreateTexture(textureInfo);
			renderPassInfo.SetDepthStencilTexture(texture);
			renderPassInfo.AddClearDepth(1.0f);
		}

		this->renderpass = Resources::CreateRenderPass(renderPassInfo);
	}

	if (!CreatePipeline(vkLayouts)) { return false; }

	return true;
}

void Pipeline::Resize()
{
	//TODO: Renderpass->Resize();
	if (!outsideRenderpass)
	{
		renderpass->width = Settings::WindowWidth();
		renderpass->height = Settings::WindowHeight();

		for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
		{
			Resources::RecreateTexture(renderpass->outputTextures[i], renderpass->width, renderpass->height, 1);
			vkDestroyFramebuffer(Renderer::device, renderpass->frameBuffers[i], Renderer::allocationCallbacks);
		}

		if(renderpass->outputDepth)
		{
			Resources::RecreateTexture(renderpass->outputDepth, renderpass->width, renderpass->height, 1);
		}

		vkDestroyRenderPass(Renderer::device, renderpass->renderpass, Renderer::allocationCallbacks);

		Renderer::CreateRenderPass(renderpass);
	}
}

void Pipeline::UpdateDescriptors()
{
	for (U32 i = 0; i < connectionCount; ++i)
	{
		PipelineConnection& connection = pipelineConnections[i];

		switch (connection.type)
		{
		case CONNECTION_TYPE_RENDERTARGET: {
			Texture* texture = connection.pipeline->renderpass->outputTextures[connection.index];

			for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
			{
				Resources::UpdateDescriptorSet(descriptorSets[i][connection.set], texture, connection.binding);
			}
		} break;
		case CONNECTION_TYPE_DEPTHBUFFER: {
			Texture* texture = connection.pipeline->renderpass->outputDepth;

			for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
			{
				Resources::UpdateDescriptorSet(descriptorSets[i][connection.set], texture, connection.binding);
			}
		} break;
		case CONNECTION_TYPE_BUFFER: {
			for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
			{
				Resources::UpdateDescriptorSet(descriptorSets[i][connection.set], connection.buffer, connection.binding);
			}
		} break;
		}
	}
}

void Pipeline::Destroy()
{
	for (U8 i = 0; i < descriptorSetCount; ++i)
	{
		for (U8 j = 0; j < MAX_SWAPCHAIN_IMAGES; ++j)
		{
			Resources::DestroyDescriptorSet(descriptorSets[j][i]);
		}
	}

	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); }

	name.Destroy();
}

bool Pipeline::CreatePipeline(VkDescriptorSetLayout* vkLayouts)
{
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

		if (cacheHeader->deviceID == Renderer::physicalDeviceProperties.deviceID &&
			cacheHeader->vendorID == Renderer::physicalDeviceProperties.vendorID &&
			memcmp(cacheHeader->pipelineCacheUUID, Renderer::physicalDeviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0)
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
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.vertexAttributeDescriptionCount = shader.vertexAttributeCount;
		vertexInputInfo.pVertexAttributeDescriptions = shader.vertexAttributes;
		vertexInputInfo.vertexBindingDescriptionCount = shader.vertexStreamCount;
		vertexInputInfo.pVertexBindingDescriptions = shader.vertexStreams;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.pNext = nullptr;
		inputAssembly.flags = 0;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		if (blendStateCount == 0)
		{
			blendStates[0].blendEnable = VK_FALSE;
			blendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			++blendStateCount;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = blendStateCount;
		colorBlending.pAttachments = blendStates;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		depthStencilInfo.depthWriteEnable = depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.stencilTestEnable = depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthTestEnable = depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthCompareOp = depthStencil.depthComparison;
		if (depthStencil.stencilEnable) { Logger::Error("Stencil Buffers Not Yet Supported!"); }

		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = rasterization.cullMode;
		rasterizer.frontFace = rasterization.front;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

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

		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.pStages = shader.stageInfos;
		pipelineInfo.stageCount = shader.shaderCount;
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

		vkCreateGraphicsPipelines(Renderer::device, pipelineCache, 1, &pipelineInfo, Renderer::allocationCallbacks, &pipeline);
	}
	else
	{
		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

		pipelineInfo.stage = shader.stageInfos[0];
		pipelineInfo.layout = pipelineLayout;

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
		Memory::FreeSize(&cacheData);

		cache.Close();
	}

	vkDestroyPipelineCache(Renderer::device, pipelineCache, Renderer::allocationCallbacks);

	return true;
}

void Pipeline::AddConnection(const PipelineConnection& connection)
{
	pipelineConnections[connectionCount++] = connection;

	switch (connection.type)
	{
	case CONNECTION_TYPE_RENDERTARGET: {
		Texture* texture = connection.pipeline->renderpass->outputTextures[connection.index];

		for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
		{
			Resources::UpdateDescriptorSet(descriptorSets[i][connection.set], texture, connection.binding);
		}
	} break;
	case CONNECTION_TYPE_DEPTHBUFFER: {
		Texture* texture = connection.pipeline->renderpass->outputDepth;

		for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
		{
			Resources::UpdateDescriptorSet(descriptorSets[i][connection.set], texture, connection.binding);
		}
	} break;
	case CONNECTION_TYPE_BUFFER: {
		for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
		{
			Resources::UpdateDescriptorSet(descriptorSets[i][connection.set], connection.buffer, connection.binding);
		}
	} break;
	}
}