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

bool Pipeline::Create()
{
	RenderpassCreation renderPassInfo{};
	renderPassInfo.colorOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	renderPassInfo.depthOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	renderPassInfo.stencilOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	renderPassInfo.width = Settings::WindowWidth();
	renderPassInfo.height = Settings::WindowHeight();

	if (!renderpass)
	{
		String textureName = name + "_output_";
		renderPassInfo.SetName(name + "_pass");

		for (U32 i = 0; i < shader->outputCount; ++i)
		{
			TextureCreation textureInfo{};
			textureInfo.name = textureName + i;
			textureInfo.format = shader->outputs[i];
			textureInfo.width = renderPassInfo.width;
			textureInfo.height = renderPassInfo.height;
			textureInfo.depth = 1;
			textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
			textureInfo.type = VK_IMAGE_TYPE_2D;

			Texture* texture = Resources::CreateTexture(textureInfo);
			renderPassInfo.AddRenderTarget(texture);
			//TODO: Add clear colors if not equal to render targets
		}

		if (shader->depthStencil.depthEnable)
		{
			TextureCreation textureInfo{};
			textureInfo.name = textureName + "depth";
			textureInfo.format = VK_FORMAT_D32_SFLOAT;
			textureInfo.width = renderPassInfo.width;
			textureInfo.height = renderPassInfo.height;
			textureInfo.depth = 1;
			textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
			textureInfo.type = VK_IMAGE_TYPE_2D;

			Texture* texture = Resources::CreateTexture(textureInfo);
			renderPassInfo.SetDepthStencilTexture(texture);
			renderPassInfo.AddClearDepth(1.0f);
		}

		this->renderpass = Resources::CreateRenderPass(renderPassInfo);
	}

	if (!CreatePipeline()) { return false; }

	return true;
}

void Pipeline::Update()
{
	Renderer::PushDescriptors(shader, descriptors);
}

void Pipeline::Resize()
{
	renderpass->Resize();
}

void Pipeline::Destroy()
{
	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); }

	name.Destroy();
}

void Pipeline::AddDescriptor(const Descriptor& descriptor)
{
	descriptors[descriptorCount++] = descriptor;
}

bool Pipeline::CreatePipeline()
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

	if (shader->bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.vertexAttributeDescriptionCount = shader->vertexAttributeCount;
		vertexInputInfo.pVertexAttributeDescriptions = shader->vertexAttributes;
		vertexInputInfo.vertexBindingDescriptionCount = shader->vertexStreamCount;
		vertexInputInfo.pVertexBindingDescriptions = shader->vertexStreams;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.pNext = nullptr;
		inputAssembly.flags = 0;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		if (shader->blendStateCount == 0)
		{
			shader->blendStates[0].blendEnable = VK_FALSE;
			shader->blendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			++shader->blendStateCount;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = shader->blendStateCount;
		colorBlending.pAttachments = shader->blendStates;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		depthStencilInfo.depthWriteEnable = shader->depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.stencilTestEnable = shader->depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthTestEnable = shader->depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthCompareOp = shader->depthStencil.depthComparison;
		if (shader->depthStencil.stencilEnable) { Logger::Error("Stencil Buffers Not Yet Supported!"); }

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
		rasterizer.cullMode = shader->rasterization.cullMode;
		rasterizer.frontFace = shader->rasterization.front;
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
		pipelineInfo.pStages = shader->stageInfos;
		pipelineInfo.stageCount = shader->stageCount;
		pipelineInfo.layout = shader->pipelineLayout;
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

		pipelineInfo.stage = shader->stageInfos[0];
		pipelineInfo.layout = shader->pipelineLayout;

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