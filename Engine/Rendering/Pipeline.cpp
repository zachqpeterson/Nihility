#include "Pipeline.hpp"

#include "Renderer.hpp"

#include "Math/Math.hpp"

struct VkVertex
{
	Vector3 position = 0.0f;
	Vector4 color = 1.0f;
	Vector3 normal = 0.0f;
	Vector2 uv = 0.0f;
	Vector4Int boneNumber = 0;
	Vector4 boneWeight = 0.0f;
};

bool Pipeline::Create(const PipelineLayout& layout, Vector<Shader> shaders)
{
	Vector<VkPipelineShaderStageCreateInfo> shaderInfos(shaders.Size());

	for (const Shader& shader : shaders)
	{
		VkPipelineShaderStageCreateInfo shaderStage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStage.pNext = nullptr;
		shaderStage.flags = 0;
		shaderStage.stage = (VkShaderStageFlagBits)shader.type;
		shaderStage.module = shader.vkShaderModule;
		shaderStage.pName = "main";
		shaderStage.pSpecializationInfo = nullptr;

		shaderInfos.Push(shaderStage);
	}

	Vector<VkVertexInputBindingDescription> vertexBindings = { { 0, (U32)sizeof(VkVertex), VK_VERTEX_INPUT_RATE_VERTEX } };

	VkVertexInputAttributeDescription positionAttribute{};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = (U32)offsetof(VkVertex, position);

	VkVertexInputAttributeDescription colorAttribute{};
	colorAttribute.binding = 0;
	colorAttribute.location = 1;
	colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorAttribute.offset = (U32)offsetof(VkVertex, color);

	VkVertexInputAttributeDescription normalAttribute{};
	normalAttribute.binding = 0;
	normalAttribute.location = 2;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = (U32)offsetof(VkVertex, normal);

	VkVertexInputAttributeDescription uvAttribute{};
	uvAttribute.binding = 0;
	uvAttribute.location = 3;
	uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribute.offset = (U32)offsetof(VkVertex, uv);

	VkVertexInputAttributeDescription jointsAttribute{};
	jointsAttribute.binding = 0;
	jointsAttribute.location = 4;
	jointsAttribute.format = VK_FORMAT_R32G32B32A32_UINT;
	jointsAttribute.offset = (U32)offsetof(VkVertex, boneNumber);

	VkVertexInputAttributeDescription weightAttribute{};
	weightAttribute.binding = 0;
	weightAttribute.location = 5;
	weightAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	weightAttribute.offset = (U32)offsetof(VkVertex, boneWeight);

	Vector<VkVertexInputAttributeDescription> attributes = {
		positionAttribute,
		colorAttribute,
		normalAttribute,
		uvAttribute,
		jointsAttribute,
		weightAttribute
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = (U32)vertexBindings.Size();
	vertexInputInfo.pVertexBindingDescriptions = vertexBindings.Data();
	vertexInputInfo.vertexAttributeDescriptionCount = (U32)attributes.Size();
	vertexInputInfo.pVertexAttributeDescriptions = attributes.Data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.flags = 0;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	VkRect2D scissor{};

	VkPipelineViewportStateCreateInfo viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportStateInfo.pNext = nullptr;
	viewportStateInfo.flags = 0;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizerInfo.pNext = nullptr;
	rasterizerInfo.flags = 0;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = VK_FALSE;
	rasterizerInfo.depthBiasConstantFactor = 0.0f;
	rasterizerInfo.depthBiasClamp = 0.0f;
	rasterizerInfo.depthBiasSlopeFactor = 0.0f;
	rasterizerInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisamplingInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisamplingInfo.pNext = nullptr;
	multisamplingInfo.flags = 0;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.minSampleShading = 0.0f;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = {};
	colorBlendAttachment.dstColorBlendFactor = {};
	colorBlendAttachment.colorBlendOp = {};
	colorBlendAttachment.srcAlphaBlendFactor = {};
	colorBlendAttachment.dstAlphaBlendFactor = {};
	colorBlendAttachment.alphaBlendOp = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendingInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendingInfo.pNext = nullptr;
	colorBlendingInfo.flags = 0;
	colorBlendingInfo.logicOpEnable = VK_FALSE;
	colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendingInfo.attachmentCount = 1;
	colorBlendingInfo.pAttachments = &colorBlendAttachment;
	colorBlendingInfo.blendConstants[0] = 0.0f;
	colorBlendingInfo.blendConstants[1] = 0.0f;
	colorBlendingInfo.blendConstants[2] = 0.0f;
	colorBlendingInfo.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilInfo.pNext = nullptr;
	depthStencilInfo.flags = 0;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.front = {};
	depthStencilInfo.back = {};
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;

	Vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStatesInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStatesInfo.pNext = nullptr;
	dynamicStatesInfo.flags = 0;
	dynamicStatesInfo.dynamicStateCount = (U32)dynamicStates.Size();
	dynamicStatesInfo.pDynamicStates = dynamicStates.Data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = (U32)shaderInfos.Size();
	pipelineCreateInfo.pStages = shaderInfos.Data();
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendingInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStatesInfo;
	pipelineCreateInfo.layout = layout;
	pipelineCreateInfo.renderPass = Renderer::renderpass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = 0;

	//TODO: Cache
	VkValidateFR(vkCreateGraphicsPipelines(Renderer::device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, Renderer::allocationCallbacks, &vkPipeline));

	return true;
}

void Pipeline::Destroy()
{
	if (vkPipeline)
	{
		vkDestroyPipeline(Renderer::device, vkPipeline, Renderer::allocationCallbacks);
	}
}

Pipeline::operator VkPipeline() const
{
	return vkPipeline;
}