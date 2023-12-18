#include "Pipeline.hpp"

#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Platform.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"

bool Pipeline::Create(const PipelineInfo& info, Renderpass* renderpass)
{
	name = info.name;
	shader = info.shader;
	subpass = info.subpass;
	this->renderpass = info.renderpass;

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

	if (shader->bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.pNext = nullptr;
		inputAssembly.flags = 0;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilInfo.pNext = nullptr;
		depthStencilInfo.flags = 0;
		depthStencilInfo.depthWriteEnable = shader->depthWriteEnable;
		depthStencilInfo.stencilTestEnable = shader->stencilEnable;
		depthStencilInfo.depthTestEnable = shader->depthEnable;
		depthStencilInfo.depthCompareOp = (VkCompareOp)shader->depthComparison;
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;
		if (shader->stencilEnable) { Logger::Error("Stencil Buffers Not Yet Supported!"); }

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
		rasterizer.cullMode = (VkCullModeFlags)shader->cullMode;
		rasterizer.frontFace = (VkFrontFace)shader->frontMode;
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
		pipelineInfo.subpass = subpass;

		shader->FillOutShaderInfo(pipelineInfo, vertexInputInfo, colorBlending, &info.specialization.specializationInfo);

		vkCreateGraphicsPipelines(Renderer::device, pipelineCache, 1, &pipelineInfo, Renderer::allocationCallbacks, &pipeline);
	}
	else
	{
		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

		pipelineInfo.layout = shader->pipelineLayout;

		shader->FillOutShaderInfo(pipelineInfo, &info.specialization.specializationInfo);

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

	return true;
}

Pipeline::Pipeline(Pipeline&& other) noexcept : name{ Move(other.name) }, shader{ other.shader }, pipeline{ other.pipeline }, renderpass{ other.renderpass }, subpass{ other.subpass }
{
	other.pipeline = nullptr;
}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
{
	name = Move(other.name);
	shader = other.shader;
	pipeline = other.pipeline;
	renderpass = other.renderpass;
	subpass = other.subpass;

	other.pipeline = nullptr;

	return *this;
}

void Pipeline::Destroy()
{
	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); }
}

void Pipeline::Run(CommandBuffer* commandBuffer, const Buffer& vertexBuffer, const Buffer& instanceBuffer, const Buffer& indexBuffer, const Buffer& drawBuffer, const Buffer& countsBuffer) const
{
	//TODO: Task Submitting
	//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, late ? meshlatePipelineMS : meshPipelineMS);
	//
	//DescriptorInfo pyramidDesc(depthSampler, depthPyramid.imageView, VK_IMAGE_LAYOUT_GENERAL);
	//DescriptorInfo descriptors[] = { dcb.buffer, db.buffer, mlb.buffer, mdb.buffer, vb.buffer, mvb.buffer, pyramidDesc };
	//// vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer, meshProgramMS.updateTemplate, meshProgramMS.layout, 0, descriptors);
	//pushDescriptors(meshProgramMS, descriptors);
	//
	//vkCmdPushConstants(commandBuffer, meshProgramMS.layout, meshProgramMS.pushConstantStages, 0, sizeof(globals), &globals);
	//vkCmdDrawMeshTasksIndirectEXT(commandBuffer, dccb.buffer, 4, 1, 0);

	commandBuffer->BindPipeline(this);

	shader->PushDescriptors(commandBuffer);
	if (shader->pushConstantStages) { Renderer::PushConstants(commandBuffer, shader); }

	switch (shader->bindPoint)
	{
	case VK_PIPELINE_BIND_POINT_GRAPHICS: {

		if (shader->useVertices)
		{
			if (drawCount)
			{
				commandBuffer->BindVertexBuffer(shader, vertexBuffer, vertexOffset);
				if (shader->useInstancing) { commandBuffer->BindInstanceBuffer(shader, instanceBuffer, instanceOffset); }

				if (shader->useIndexing)
				{
					commandBuffer->BindIndexBuffer(shader, indexBuffer, indexOffset);
					commandBuffer->DrawIndexedIndirectCount(drawBuffer, countsBuffer, drawOffset, countOffset);
				}
				else { commandBuffer->DrawIndirectCount(drawBuffer, countsBuffer); }
			}
		}
		else { commandBuffer->Draw(0, 3, 0, 1); } //TODO: Specify in shader how many vertices
	} break;
	case VK_PIPELINE_BIND_POINT_COMPUTE: {
		commandBuffer->Dispatch(shader->localSizeX, shader->localSizeY, shader->localSizeZ);
	} break;
	case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
		//TODO:
	} break;
	}
}

const String& Pipeline::Name() const
{
	return name;
}