#include "Pipeline.hpp"

#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Platform.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"

bool Pipeline::Create(const PipelineInfo& info, Renderpass* renderpass)
{
	shader = info.shader;

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
		depthStencilInfo.depthWriteEnable = shader->depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.stencilTestEnable = shader->depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthTestEnable = shader->depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthCompareOp = (VkCompareOp)shader->depthStencil.depthComparison;
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;
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
		rasterizer.cullMode = (VkCullModeFlags)shader->rasterization.cullMode;
		rasterizer.frontFace = (VkFrontFace)shader->rasterization.front;
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

	U32 vertexBufferSize = info.vertexBufferSize;
	U32 instanceBufferSize = info.instanceBufferSize;
	U32 indexBufferSize = info.indexBufferSize;
	U32 drawBufferSize = info.drawBufferSize;
	if (vertexBufferSize == U32_MAX) { vertexBufferSize = MEGABYTES(32); }
	if (instanceBufferSize == U32_MAX) { instanceBufferSize = MEGABYTES(32); }
	if (indexBufferSize == U32_MAX) { indexBufferSize = MEGABYTES(32); }
	if (drawBufferSize == U32_MAX) { drawBufferSize = MEGABYTES(32); }

	if (vertexBufferSize) { vertexBuffer = Renderer::CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); }
	if (instanceBufferSize) { instanceBuffer = Renderer::CreateBuffer(instanceBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); }
	if (indexBufferSize) { indexBuffer = Renderer::CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); }
	if (drawBufferSize) { drawBuffer = Renderer::CreateBuffer(drawBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); }
	countsBuffer = Renderer::CreateBuffer(sizeof(U32), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	return true;
}

void Pipeline::Destroy()
{
	Renderer::DestroyBuffer(vertexBuffer);
	Renderer::DestroyBuffer(instanceBuffer);
	Renderer::DestroyBuffer(indexBuffer);
	Renderer::DestroyBuffer(drawBuffer);

	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); }

	name.Destroy();
	handle = U64_MAX;
}

void Pipeline::Run(CommandBuffer* commandBuffer) const
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

	if (drawCount || shader->bindPoint != VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		commandBuffer->BindPipeline(this);

		shader->PushDescriptors(commandBuffer);
		if (shader->pushConstantStages) { Renderer::PushConstants(commandBuffer, shader); }

		if (shader->drawType == DRAW_TYPE_FULLSCREEN)
		{
			commandBuffer->Draw(0, 3, 0, 1);
		}
		else
		{
			switch (shader->bindPoint)
			{
			case VK_PIPELINE_BIND_POINT_GRAPHICS: {
				commandBuffer->BindIndexBuffer(shader, indexBuffer);
				if (shader->instanceLocation) { commandBuffer->BindVertexBuffer(shader, vertexBuffer); }
				if (shader->instanceLocation != U8_MAX) { commandBuffer->BindInstanceBuffer(shader, instanceBuffer); }

				commandBuffer->DrawIndexedIndirectCount(drawBuffer, countsBuffer, 0, 0);
			} break;
			case VK_PIPELINE_BIND_POINT_COMPUTE: {
				commandBuffer->Dispatch((U32)Renderer::renderArea.z, (U32)Renderer::renderArea.w, 1);
			} break;
			case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
				//TODO:
			} break;
			}
		}
	}
}

U32 Pipeline::UploadIndices(U32 size, const void* data)
{
	VkBufferCopy region{};
	region.dstOffset = indexOffset;
	region.size = size;
	region.srcOffset = 0;

	Renderer::FillBuffer(indexBuffer, size, data, 1, &region);

	U32 offset = indexOffset;
	indexOffset += (U32)size;

	return offset;
}

U32 Pipeline::UploadVertices(U32 size, const void* data)
{
	VkBufferCopy region{};
	region.dstOffset = vertexOffset;
	region.size = size;
	region.srcOffset = 0;

	Renderer::FillBuffer(vertexBuffer, size, data, 1, &region);

	U32 offset = vertexOffset;
	vertexOffset += (U32)size;

	return offset;
}

void Pipeline::UpdateVertices(U32 size, const void* data, U32 regionCount, BufferCopy* regions)
{
	Renderer::FillBuffer(vertexBuffer, size, data, regionCount, (VkBufferCopy*)regions);
}

void Pipeline::UpdateVertices(const Buffer& stagingBuffer, U32 regionCount, BufferCopy* regions)
{
	Renderer::FillBuffer(vertexBuffer, stagingBuffer, regionCount, (VkBufferCopy*)regions);
}

U32 Pipeline::UploadInstances(U32 size, const void* data)
{
	VkBufferCopy region{};
	region.dstOffset = instanceOffset;
	region.size = size;
	region.srcOffset = 0;

	Renderer::FillBuffer(instanceBuffer, size, data, 1, &region);

	U32 offset = instanceOffset;
	instanceOffset += (U32)size;

	return offset;
}

void Pipeline::UpdateInstances(U32 size, const void* data, U32 regionCount, BufferCopy* regions)
{
	Renderer::FillBuffer(instanceBuffer, size, data, regionCount, (VkBufferCopy*)regions);
}

void Pipeline::UpdateInstances(const Buffer& stagingBuffer, U32 regionCount, BufferCopy* regions)
{
	Renderer::FillBuffer(instanceBuffer, stagingBuffer, regionCount, (VkBufferCopy*)regions);
}

void Pipeline::UploadDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset)
{
	VkDrawIndexedIndirectCommand drawCommand{};
	drawCommand.indexCount = indexCount;
	drawCommand.instanceCount = instanceCount;
	drawCommand.firstIndex = indexOffset;
	drawCommand.vertexOffset = vertexOffset;
	drawCommand.firstInstance = instanceOffset;

	VkBufferCopy region{};
	region.dstOffset = drawCount * sizeof(VkDrawIndexedIndirectCommand);
	region.size = sizeof(VkDrawIndexedIndirectCommand);
	region.srcOffset = 0;

	Renderer::FillBuffer(drawBuffer, sizeof(VkDrawIndexedIndirectCommand), &drawCommand, 1, &region);

	++drawCount;

	region.dstOffset = 0;
	region.size = sizeof(U32);
	region.srcOffset = 0;

	Renderer::FillBuffer(countsBuffer, sizeof(U32), &drawCount, 1, &region);
}

void Pipeline::UpdateDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset, U32 drawOffset)
{
	VkDrawIndexedIndirectCommand drawCommand{};
	drawCommand.indexCount = indexCount;
	drawCommand.instanceCount = instanceCount;
	drawCommand.firstIndex = indexOffset;
	drawCommand.vertexOffset = vertexOffset;
	drawCommand.firstInstance = instanceOffset;

	VkBufferCopy region{};
	region.dstOffset = drawOffset;
	region.size = sizeof(VkDrawIndexedIndirectCommand);
	region.srcOffset = 0;

	Renderer::FillBuffer(drawBuffer, sizeof(VkDrawIndexedIndirectCommand), &drawCommand, 1, &region);
}

const String& Pipeline::Name() const
{
	return name;
}