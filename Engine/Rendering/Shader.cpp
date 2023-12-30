#include "Shader.hpp"

#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"

//TODO: #define ENABLE_OPT for optimised shaders
#include "External\LunarG\SPIRV\GlslangToSpv.h"
#include "External\LunarG\glslang\Public\ShaderLang.h"
#include "External\LunarG\spirv_cross\spirv_reflect.h"

#if defined(_MSC_VER)
#include "External\LunarG\spirv-headers\spirv.h"
#else
#include "External\LunarG\spirv_cross\spirv.h"
#endif

Descriptor::Descriptor(VkBuffer buffer, U64 offset, U64 range)
{
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = range;
}

Descriptor::Descriptor(VkImageView imageView, ImageLayout imageLayout, VkSampler sampler)
{
	imageInfo.imageView = imageView;
	imageInfo.imageLayout = imageLayout;
	imageInfo.sampler = sampler;
}

Descriptor::Descriptor(Texture* texture)
{
	imageInfo.imageView = texture->imageView;
	imageInfo.imageLayout = IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = texture->sampler.vkSampler;
}

TBuiltInResource resources{
	.maxLights = 32,
		.maxClipPlanes = 6,
		.maxTextureUnits = 32,
		.maxTextureCoords = 32,
		.maxVertexAttribs = 64,
		.maxVertexUniformComponents = 4096,
		.maxVaryingFloats = 64,
		.maxVertexTextureImageUnits = 32,
		.maxCombinedTextureImageUnits = 80,
		.maxTextureImageUnits = 32,
		.maxFragmentUniformComponents = 4096,
		.maxDrawBuffers = 32,
		.maxVertexUniformVectors = 128,
		.maxVaryingVectors = 8,
		.maxFragmentUniformVectors = 16,
		.maxVertexOutputVectors = 16,
		.maxFragmentInputVectors = 15,
		.minProgramTexelOffset = -8,
		.maxProgramTexelOffset = 7,
		.maxClipDistances = 8,
		.maxComputeWorkGroupCountX = 65535,
		.maxComputeWorkGroupCountY = 65535,
		.maxComputeWorkGroupCountZ = 65535,
		.maxComputeWorkGroupSizeX = 1024,
		.maxComputeWorkGroupSizeY = 1024,
		.maxComputeWorkGroupSizeZ = 64,
		.maxComputeUniformComponents = 1024,
		.maxComputeTextureImageUnits = 16,
		.maxComputeImageUniforms = 8,
		.maxComputeAtomicCounters = 8,
		.maxComputeAtomicCounterBuffers = 1,
		.maxVaryingComponents = 60,
		.maxVertexOutputComponents = 64,
		.maxGeometryInputComponents = 64,
		.maxGeometryOutputComponents = 128,
		.maxFragmentInputComponents = 128,
		.maxImageUnits = 8,
		.maxCombinedImageUnitsAndFragmentOutputs = 8,
		.maxCombinedShaderOutputResources = 8,
		.maxImageSamples = 0,
		.maxVertexImageUniforms = 0,
		.maxTessControlImageUniforms = 0,
		.maxTessEvaluationImageUniforms = 0,
		.maxGeometryImageUniforms = 0,
		.maxFragmentImageUniforms = 8,
		.maxCombinedImageUniforms = 8,
		.maxGeometryTextureImageUnits = 16,
		.maxGeometryOutputVertices = 256,
		.maxGeometryTotalOutputComponents = 1024,
		.maxGeometryUniformComponents = 1024,
		.maxGeometryVaryingComponents = 64,
		.maxTessControlInputComponents = 128,
		.maxTessControlOutputComponents = 128,
		.maxTessControlTextureImageUnits = 16,
		.maxTessControlUniformComponents = 1024,
		.maxTessControlTotalOutputComponents = 4096,
		.maxTessEvaluationInputComponents = 128,
		.maxTessEvaluationOutputComponents = 128,
		.maxTessEvaluationTextureImageUnits = 16,
		.maxTessEvaluationUniformComponents = 1024,
		.maxTessPatchComponents = 120,
		.maxPatchVertices = 32,
		.maxTessGenLevel = 64,
		.maxViewports = 16,
		.maxVertexAtomicCounters = 0,
		.maxTessControlAtomicCounters = 0,
		.maxTessEvaluationAtomicCounters = 0,
		.maxGeometryAtomicCounters = 0,
		.maxFragmentAtomicCounters = 8,
		.maxCombinedAtomicCounters = 8,
		.maxAtomicCounterBindings = 1,
		.maxVertexAtomicCounterBuffers = 0,
		.maxTessControlAtomicCounterBuffers = 0,
		.maxTessEvaluationAtomicCounterBuffers = 0,
		.maxGeometryAtomicCounterBuffers = 0,
		.maxFragmentAtomicCounterBuffers = 1,
		.maxCombinedAtomicCounterBuffers = 1,
		.maxAtomicCounterBufferSize = 16384,
		.maxTransformFeedbackBuffers = 4,
		.maxTransformFeedbackInterleavedComponents = 64,
		.maxCullDistances = 8,
		.maxCombinedClipAndCullDistances = 8,
		.maxSamples = 4,
		.maxMeshOutputVerticesNV = 256,
		.maxMeshOutputPrimitivesNV = 512,
		.maxMeshWorkGroupSizeX_NV = 32,
		.maxMeshWorkGroupSizeY_NV = 1,
		.maxMeshWorkGroupSizeZ_NV = 1,
		.maxTaskWorkGroupSizeX_NV = 32,
		.maxTaskWorkGroupSizeY_NV = 1,
		.maxTaskWorkGroupSizeZ_NV = 1,
		.maxMeshViewCountNV = 4,

		.limits{
		.nonInductiveForLoops = 1,
			.whileLoops = 1,
			.doWhileLoops = 1,
			.generalUniformIndexing = 1,
			.generalAttributeMatrixVectorIndexing = 1,
			.generalVaryingIndexing = 1,
			.generalSamplerIndexing = 1,
			.generalVariableIndexing = 1,
			.generalConstantMatrixVectorIndexing = 1,
	}
};

struct Id
{
	U32 opcode;
	U32 typeId;
	U32 storageClass;

	U32 set;
	U32 binding;
	U32 location;
	U32 inputIndex;
	bool inputAttachment;

	U32 width;
	U32 sign;
	U32 count;

	U32 constant;
};

struct DescriptorSetLayout
{
	U64								handle{ U64_MAX };

	VkDescriptorSetLayout_T* descriptorSetLayout{ nullptr };
	VkDescriptorUpdateTemplate_T* updateTemplate{ nullptr };

	VkDescriptorSetLayoutBinding	bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8								bindingCount{ 0 };
	U8								setIndex{ 0 };
};

struct ShaderInfo
{
	VkPipelineColorBlendAttachmentState blendStates[MAX_IMAGE_OUTPUTS]{};
	U8									blendStateCount{ 0 };

	VkPipelineShaderStageCreateInfo		stageInfos[MAX_SHADER_STAGES]{};
	U32									stageCount{ 0 };

	VkVertexInputBindingDescription		vertexStreams[MAX_VERTEX_STREAMS]{};
	U32									vertexStreamCount{ 0 };

	VkVertexInputAttributeDescription	vertexAttributes[MAX_VERTEX_ATTRIBUTES]{};
	U32									vertexAttributeCount{ 0 };

	I32									stages{ 0 };
};

VkDescriptorSetLayout			Shader::dummyDescriptorSetLayout;

Pool<DescriptorSetLayout, 256>	Shader::descriptorSetLayouts;
VkDescriptorPool				Shader::bindlessDescriptorPool;
VkDescriptorSet					Shader::bindlessDescriptorSet;
DescriptorSetLayout*			Shader::bindlessDescriptorSetLayout;

bool Shader::Initialize()
{
	if (!glslang::InitializeProcess()) { return false; }
	descriptorSetLayouts.Create();

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0;
	layoutInfo.bindingCount = 0;
	layoutInfo.pBindings = nullptr;

	if (vkCreateDescriptorSetLayout(Renderer::device, &layoutInfo, Renderer::allocationCallbacks, &dummyDescriptorSetLayout) != VK_SUCCESS)
	{
		Logger::Error("Failed To Create Dummy Descriptor Set Layout!");
		return false;
	}

	Renderer::SetResourceName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)dummyDescriptorSetLayout, "dummy_dsl");

	if (Renderer::bindlessSupported)
	{
		VkDescriptorPoolSize bindlessPoolSizes[]
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxBindlessResources },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxBindlessResources },
		};

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
		poolInfo.maxSets = maxBindlessResources * CountOf32(bindlessPoolSizes);
		poolInfo.poolSizeCount = CountOf32(bindlessPoolSizes);
		poolInfo.pPoolSizes = bindlessPoolSizes;

		VkValidateFR(vkCreateDescriptorPool(Renderer::device, &poolInfo, Renderer::allocationCallbacks, &bindlessDescriptorPool));

		const U32 poolCount = CountOf32(bindlessPoolSizes);
		VkDescriptorSetLayoutBinding vkBinding[4];

		// Actual descriptor set layout
		VkDescriptorSetLayoutBinding& imageSamplerBinding = vkBinding[0];
		imageSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageSamplerBinding.descriptorCount = maxBindlessResources;
		imageSamplerBinding.binding = bindlessTextureBinding;
		imageSamplerBinding.stageFlags = VK_SHADER_STAGE_ALL;
		imageSamplerBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding& storageImageBinding = vkBinding[1];
		storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		storageImageBinding.descriptorCount = maxBindlessResources;
		storageImageBinding.binding = bindlessTextureBinding + 1;
		storageImageBinding.stageFlags = VK_SHADER_STAGE_ALL;
		storageImageBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = poolCount;
		layoutInfo.pBindings = vkBinding;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		// TODO: reenable variable descriptor count
		// Binding flags
		VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
		VkDescriptorBindingFlags bindingFlags[4];

		bindingFlags[0] = bindlessFlags;
		bindingFlags[1] = bindlessFlags;

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		extendedInfo.bindingCount = poolCount;
		extendedInfo.pBindingFlags = bindingFlags;

		layoutInfo.pNext = &extendedInfo;

		U64 handle;
		bindlessDescriptorSetLayout = descriptorSetLayouts.Request(handle);
		bindlessDescriptorSetLayout->handle = handle;

		VkValidateFR(vkCreateDescriptorSetLayout(Renderer::device, &layoutInfo, Renderer::allocationCallbacks, &bindlessDescriptorSetLayout->descriptorSetLayout));

		Renderer::SetResourceName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)bindlessDescriptorSetLayout->descriptorSetLayout, "bindless_dsl");

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = bindlessDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &bindlessDescriptorSetLayout->descriptorSetLayout;

		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
		U32 maxBinding = maxBindlessResources - 1;
		countInfo.descriptorSetCount = 1;
		// This number is the max allocatable count
		countInfo.pDescriptorCounts = &maxBinding;
		//allocInfo.pNext = &countInfo;

		VkValidateFR(vkAllocateDescriptorSets(Renderer::device, &allocInfo, &bindlessDescriptorSet));
	}

	return true;
}

void Shader::Shutdown()
{
	vkDestroyDescriptorSetLayout(Renderer::device, dummyDescriptorSetLayout, Renderer::allocationCallbacks);

	if (Renderer::bindlessSupported)
	{
		vkDestroyDescriptorSetLayout(Renderer::device, bindlessDescriptorSetLayout->descriptorSetLayout, Renderer::allocationCallbacks);
		vkDestroyDescriptorPool(Renderer::device, bindlessDescriptorPool, Renderer::allocationCallbacks);
	}

	descriptorSetLayouts.Destroy();
}

bool Shader::Create(const String& shaderPath, U8 pushConstantCount_, PushConstant* pushConstants_)
{
	String data = Resources::LoadBinaryString(shaderPath);

	Memory::Allocate(&shaderInfo);

	U64 handle;
	setLayout = descriptorSetLayouts.Request(handle);
	bindlessDescriptorSetLayout->handle = handle;

	VkDescriptorSetLayout vkLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	VkPushConstantRange pushRange{};

	I64 index = -1;
	bool result = false;

	do
	{
		index = data.IndexOf('#', index + 1);

		if (data.CompareN("#CONFIG", index)) { result = ParseConfig(data, index); }
		else if (data.CompareN("#VERTEX", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_VERTEX_BIT); }
		else if (data.CompareN("#CONTROL", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT); }
		else if (data.CompareN("#EVALUATION", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT); }
		else if (data.CompareN("#GEOMETRY", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_GEOMETRY_BIT); }
		else if (data.CompareN("#FRAGMENT", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_FRAGMENT_BIT); }
		else if (data.CompareN("#COMPUTE", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_COMPUTE_BIT); }
		else if (data.CompareN("#TASK", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_TASK_BIT_EXT); }
		else if (data.CompareN("#MESH", index)) { result = ParseStage(data, index, VK_SHADER_STAGE_MESH_BIT_EXT); }

		if (!result)
		{
			Logger::Error("Failed To Create Shader!");
			Destroy();
			return false;
		}
	} while (index != -1);

	U32 tessellation = (shaderInfo->stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) + (shaderInfo->stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

	if (tessellation != 6 && tessellation != 0)
	{
		Logger::Error("Shaders With Tessellation Must Have Both Tessellation Stages!");
		Destroy();
		return false;
	}

	if (bindPoint == PIPELINE_BIND_POINT_GRAPHICS && ((shaderInfo->stages & VK_SHADER_STAGE_VERTEX_BIT) == 0 || (shaderInfo->stages & VK_SHADER_STAGE_FRAGMENT_BIT) == 0))
	{
		Logger::Error("Shaders With Graphics Must Have Both A Vertex And Fragment Stage!");
		Destroy();
		return false;
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

	if (shaderInfo->stageCount == 0) { Logger::Error("Shader '{}' Has No Stages!", shaderPath); return false; }

	if (shaderInfo->blendStateCount == 0)
	{
		shaderInfo->blendStates[0].blendEnable = VK_FALSE;
		shaderInfo->blendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		++shaderInfo->blendStateCount;
	}

	if (useSet0)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = Renderer::pushDescriptorsSupported ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
		layoutInfo.bindingCount = setLayout->bindingCount;
		layoutInfo.pBindings = setLayout->bindings;

		if (vkCreateDescriptorSetLayout(Renderer::device, &layoutInfo, Renderer::allocationCallbacks, &setLayout->descriptorSetLayout) != VK_SUCCESS)
		{
			Logger::Error("Failed To Create Descriptor Set Layout!");
			Destroy();
			return false;
		}

		Renderer::SetResourceName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)setLayout->descriptorSetLayout, name + "_dsl");

		vkLayouts[0] = setLayout->descriptorSetLayout;
	}
	else
	{
		vkLayouts[0] = dummyDescriptorSetLayout;
	}

	if (Renderer::bindlessSupported && useBindless)
	{
		vkLayouts[1] = bindlessDescriptorSetLayout->descriptorSetLayout;
	}

	U32 pushContantSize = 0;

	pushConstantCount = pushConstantCount_;

	pushRange.stageFlags = pushConstantStages;
	pushRange.offset = 0;

	for (U32 i = 0; i < pushConstantCount; ++i)
	{
		pushRange.size += pushConstants_[i].size;

		pushConstants[i] = pushConstants_[i];

		pushContantSize += pushConstants_[i].size;
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
	pipelineLayoutInfo.pSetLayouts = vkLayouts;
	pipelineLayoutInfo.setLayoutCount = useBindless ? 2 : useSet0;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantStages > 0; //TODO: Support multiple ranges
	pipelineLayoutInfo.pPushConstantRanges = &pushRange;

	VkValidate(vkCreatePipelineLayout(Renderer::device, &pipelineLayoutInfo, Renderer::allocationCallbacks, &pipelineLayout));

	if (useSet0)
	{
		VkDescriptorUpdateTemplateEntry entries[MAX_DESCRIPTORS_PER_SET]{};

		for (U32 i = 0; i < setLayout->bindingCount; ++i)
		{
			VkDescriptorSetLayoutBinding& binding = setLayout->bindings[i];
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
		descriptorTemplateInfo.descriptorSetLayout = Renderer::pushDescriptorsSupported ? 0 : setLayout->descriptorSetLayout;
		descriptorTemplateInfo.pipelineBindPoint = (VkPipelineBindPoint)bindPoint;
		descriptorTemplateInfo.pipelineLayout = pipelineLayout;
		descriptorTemplateInfo.descriptorUpdateEntryCount = setLayout->bindingCount;
		descriptorTemplateInfo.pDescriptorUpdateEntries = entries;
		descriptorTemplateInfo.set = setLayout->setIndex;

		if (vkCreateDescriptorUpdateTemplate(Renderer::device, &descriptorTemplateInfo, Renderer::allocationCallbacks, &setLayout->updateTemplate) != VK_SUCCESS)
		{
			Logger::Error("Failed To Create Descriptor Update Template!");
			Destroy();
			return false;
		}
	}

	return true;
}

void Shader::Destroy()
{
	for (U8 i = 0; i < shaderInfo->stageCount; ++i)
	{
		vkDestroyShaderModule(Renderer::device, shaderInfo->stageInfos[i].module, Renderer::allocationCallbacks);
	}

	if (setLayout)
	{
		if (useSet0)
		{
			vkDestroyDescriptorSetLayout(Renderer::device, setLayout->descriptorSetLayout, Renderer::allocationCallbacks);
			vkDestroyDescriptorUpdateTemplate(Renderer::device, setLayout->updateTemplate, Renderer::allocationCallbacks);
			useSet0 = false;
		}

		descriptorSetLayouts.Release(setLayout->handle);
		setLayout = nullptr;
	}

	shaderInfo->stageCount = 0;
	Memory::Free(&shaderInfo);

	if (pipelineLayout) { vkDestroyPipelineLayout(Renderer::device, pipelineLayout, Renderer::allocationCallbacks); pipelineLayout = nullptr; }

	for (U32 i = 0; i < MAX_SHADER_STAGES; ++i)
	{
		stages[i].entryPoint.Destroy();
	}

	name.Destroy();
	handle = U64_MAX;
}

bool Shader::ParseConfig(const String& data, I64& index)
{
	//TODO: Blend Masks
	//TODO: Add Stencils
	while (index != -1)
	{
		if (data.CompareN("#CONFIG_END", index + 1))
		{
			++index;
			return true;
		}
		else if (data.CompareN("cull", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("NONE", index)) { cullMode = CULL_MODE_NONE; }
			else if (data.CompareN("FRONT", index)) { cullMode = CULL_MODE_FRONT_BIT; }
			else if (data.CompareN("BACK", index)) { cullMode = CULL_MODE_BACK_BIT; }
			else if (data.CompareN("BOTH", index)) { cullMode = CULL_MODE_FRONT_AND_BACK; }
		}
		else if (data.CompareN("front", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("CLOCKWISE", index)) { frontMode = FRONT_FACE_MODE_CLOCKWISE; }
			else if (data.CompareN("COUNTER", index)) { frontMode = FRONT_FACE_MODE_COUNTER_CLOCKWISE; }
		}
		else if (data.CompareN("fill", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("SOLID", index)) { fillMode = POLYGON_MODE_FILL; }
			else if (data.CompareN("LINE", index)) { fillMode = POLYGON_MODE_LINE; }
			else if (data.CompareN("POINT", index)) { fillMode = POLYGON_MODE_POINT; }
		}
		else if (data.CompareN("depth", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("NONE", index))
			{
				depthEnable = false;
				depthWriteEnable = false;
				depthComparison = COMPARE_OP_ALWAYS;
			}
			else if (data.CompareN("LESS_EQUAL", index))
			{
				depthEnable = true;
				depthWriteEnable = true;
				depthComparison = COMPARE_OP_LESS_OR_EQUAL;
			}
			else if (data.CompareN("LESS", index))
			{
				depthEnable = true;
				depthWriteEnable = true;
				depthComparison = COMPARE_OP_LESS;
			}
			else if (data.CompareN("GREATER_EQUAL", index))
			{
				depthEnable = true;
				depthWriteEnable = true;
				depthComparison = COMPARE_OP_GREATER_OR_EQUAL;
			}
			else if (data.CompareN("GREATER", index))
			{
				depthEnable = true;
				depthWriteEnable = true;
				depthComparison = COMPARE_OP_GREATER;
			}
			else if (data.CompareN("EQUAL", index))
			{
				depthEnable = true;
				depthWriteEnable = true;
				depthComparison = COMPARE_OP_EQUAL;
			}
		}
		else if (data.CompareN("blend", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("ADD", index))
			{
				VkPipelineColorBlendAttachmentState& blendState = shaderInfo->blendStates[shaderInfo->blendStateCount];

				blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				blendState.colorBlendOp = VK_BLEND_OP_ADD;

				blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				blendState.alphaBlendOp = VK_BLEND_OP_ADD;

				blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

				blendState.blendEnable = VK_TRUE;

				++shaderInfo->blendStateCount;
			}
			else if (data.CompareN("SUB", index))
			{
				VkPipelineColorBlendAttachmentState& blendState = shaderInfo->blendStates[shaderInfo->blendStateCount];

				//TODO: Not sure how to use subtractive blending
				blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendState.colorBlendOp = VK_BLEND_OP_SUBTRACT;

				blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

				blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

				blendState.blendEnable = VK_TRUE;

				++shaderInfo->blendStateCount;
			}
		}
		else if (data.CompareN("topology", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("POINTS", index)) { topologyMode = TOPOLOGY_MODE_POINT_LIST; }
			else if (data.CompareN("LINES", index)) { topologyMode = TOPOLOGY_MODE_LINE_LIST; }
			else if (data.CompareN("LINE_STRIP", index)) { topologyMode = TOPOLOGY_MODE_LINE_STRIP; }
			else if (data.CompareN("TRIANGLES", index)) { topologyMode = TOPOLOGY_MODE_TRIANGLE_LIST; }
			else if (data.CompareN("TRIANGLE_STRIP", index)) { topologyMode = TOPOLOGY_MODE_TRIANGLE_STRIP; }
			else if (data.CompareN("TRIANGLE_FAN", index)) { topologyMode = TOPOLOGY_MODE_TRIANGLE_FAN; }
			else if (data.CompareN("LINES_ADJ", index)) { topologyMode = TOPOLOGY_MODE_LINE_LIST_WITH_ADJACENCY; }
			else if (data.CompareN("LINE_STRIP_ADJ", index)) { topologyMode = TOPOLOGY_MODE_LINE_STRIP_WITH_ADJACENCY; }
			else if (data.CompareN("TRIANGLES_ADJ", index)) { topologyMode = TOPOLOGY_MODE_TRIANGLE_LIST_WITH_ADJACENCY; }
			else if (data.CompareN("TRIANGLE_STRIP_ADJ", index)) { topologyMode = TOPOLOGY_MODE_TRIANGLE_STRIP_WITH_ADJACENCY; }
			else if (data.CompareN("PATCHS", index)) { topologyMode = TOPOLOGY_MODE_PATCH_LIST; }
		}
		else if (data.CompareN("clear", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("COLOR", index)) { clearTypes |= CLEAR_TYPE_COLOR; }
			else if (data.CompareN("DEPTH", index)) { clearTypes |= CLEAR_TYPE_DEPTH; }
			else if (data.CompareN("STENCIL", index)) { clearTypes |= CLEAR_TYPE_STENCIL; }
		}
		else if (data.CompareN("useIndices", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("FALSE", index)) { useIndexing = false; }
		}
		else if (data.CompareN("useVertices", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			if (data.CompareN("FALSE", index)) { useVertices = false; }
		}
		else if (data.CompareN("instanceOffset", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);
			instanceLocation = data.ToType<U8>(index);
			useInstancing = true;
		}
		else if (data.CompareN("vertexCount", index + 1))
		{
			index = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);
			vertexCount = data.ToType<U8>(index);
		}

		index = data.IndexOf('\n', index + 1);
	}

	return true;
}

bool Shader::ParseStage(const String& data, I64& index, VkShaderStageFlagBits stage)
{
	if (stage == VK_SHADER_STAGE_COMPUTE_BIT)
	{
		if (bindPoint == PIPELINE_BIND_POINT_GRAPHICS)
		{
			Logger::Error("Compute Shaders Cannot Have Graphics Stages!");
			return false;
		}

		bindPoint = PIPELINE_BIND_POINT_COMPUTE;
	}
	else if (stage != VK_SHADER_STAGE_COMPUTE_BIT)
	{
		if (bindPoint == PIPELINE_BIND_POINT_COMPUTE)
		{
			Logger::Error("Graphics Shaders Cannot Have A Compute Stage!");
			return false;
		}

		bindPoint = PIPELINE_BIND_POINT_GRAPHICS;
	}

	shaderInfo->stages |= stage;

	I64 begin = data.IndexOf('\n', index + 1) + 1;
	I64 end = begin;

	switch (stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { while (end != -1) { if (data.CompareN("#VERTEX_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { while (end != -1) { if (data.CompareN("#CONTROL_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { while (end != -1) { if (data.CompareN("#EVALUATION_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_GEOMETRY_BIT: { while (end != -1) { if (data.CompareN("#GEOMETRY_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_FRAGMENT_BIT: { while (end != -1) { if (data.CompareN("#FRAGMENT_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_COMPUTE_BIT: { while (end != -1) { if (data.CompareN("#COMPUTE_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_TASK_BIT_EXT: { while (end != -1) { if (data.CompareN("#TASK_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_MESH_BIT_EXT: { while (end != -1) { if (data.CompareN("#MESH_END", end = data.IndexOf('#', end + 1))) { break; } } } break;
	default: return false;
	}

	String code = data.SubString(begin, end - begin - 1);

	stages[shaderInfo->stageCount].stage = (ShaderStageType)stage;
	shaderInfo->stageInfos[shaderInfo->stageCount] = CompileShader(stages[shaderInfo->stageCount], code, name);
	++shaderInfo->stageCount;

	index = data.IndexOf('\n', end + 1);

	return true;
}

const String& Shader::ToStageDefines(VkShaderStageFlagBits value)
{
	static const String vertex{ "VERTEX" };
	static const String tessControl{ "TESSCONTROL" };
	static const String tessEval{ "TESSEVAL" };
	static const String geometry{ "GEOMETRY" };
	static const String fragment{ "FRAGMENT" };
	static const String compute{ "COMPUTE" };
	static const String task{ "TASK" };
	static const String mesh{ "MESH" };
	static const String def{ "" };

	switch (value)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { return vertex; }
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { return tessControl; }
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { return tessEval; }
	case VK_SHADER_STAGE_GEOMETRY_BIT: { return geometry; }
	case VK_SHADER_STAGE_FRAGMENT_BIT: { return fragment; }
	case VK_SHADER_STAGE_COMPUTE_BIT: { return compute; }
	case VK_SHADER_STAGE_TASK_BIT_EXT: { return task; }
	case VK_SHADER_STAGE_MESH_BIT_EXT: { return mesh; }
	default: { return def; }
	}
}

const String& Shader::ToCompilerExtension(VkShaderStageFlagBits value)
{
	static const String vertex{ "vert" };
	static const String tessControl{ "tesc" };
	static const String tessEval{ "tese" };
	static const String geometry{ "geom" };
	static const String fragment{ "frag" };
	static const String compute{ "comp" };
	static const String task{ "task" };
	static const String mesh{ "mesh" };
	static const String def{ "" };

	switch (value)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { return vertex; }
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { return tessControl; }
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { return tessEval; }
	case VK_SHADER_STAGE_GEOMETRY_BIT: { return geometry; }
	case VK_SHADER_STAGE_FRAGMENT_BIT: { return fragment; }
	case VK_SHADER_STAGE_COMPUTE_BIT: { return compute; }
	case VK_SHADER_STAGE_TASK_BIT_EXT: { return task; }
	case VK_SHADER_STAGE_MESH_BIT_EXT: { return mesh; }
	default: {return def; }
	}
}

//TODO: Cache compiled shaders
VkPipelineShaderStageCreateInfo Shader::CompileShader(ShaderStage& shaderStage, String& code, const String& name)
{
	using namespace glslang;

	EShLanguage stage = EShLangCount;
	U8 stageIndex = U8_MAX;

	switch (shaderStage.stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { stage = EShLangVertex; stageIndex = 0; } break;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { stage = EShLangTessControl; stageIndex = 1; } break;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { stage = EShLangTessEvaluation; stageIndex = 2; } break;
	case VK_SHADER_STAGE_GEOMETRY_BIT: { stage = EShLangGeometry; stageIndex = 3; } break;
	case VK_SHADER_STAGE_FRAGMENT_BIT: { stage = EShLangFragment; stageIndex = 4; } break;
	case VK_SHADER_STAGE_COMPUTE_BIT: { stage = EShLangCompute; stageIndex = 5; } break;
	case VK_SHADER_STAGE_TASK_BIT_EXT: { stage = EShLangTask; stageIndex = 6; } break;
	case VK_SHADER_STAGE_MESH_BIT_EXT: { stage = EShLangMesh; stageIndex = 7; } break;
	}

	TShader tShader(stage);
	C8* c = code.Data();
	tShader.setStrings(&c, 1);
	tShader.setEnvInput(EShSourceGlsl, stage, EShClientVulkan, 450);
	tShader.setEnvClient(EShClientVulkan, EShTargetVulkan_1_3);
	tShader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_6);
	if (!tShader.parse(&resources, 450, false, EShMsgDefault)) { Logger::Error(tShader.getInfoLog()); }

	TProgram tProgram;
	tProgram.addShader(&tShader);
	if (!tProgram.link(EShMsgDefault)) { Logger::Error(tProgram.getInfoLog()); }

	tProgram.getIntermediate(stage);

	std::vector<U32> spv;
	GlslangToSpv(*tProgram.getIntermediate(stage), spv);

	return ParseSPIRV(&spv[0], (U32)spv.size(), shaderStage);
}

static VkShaderStageFlagBits GetShaderStage(SpvExecutionModel executionModel)
{
	switch (executionModel)
	{
	case SpvExecutionModelVertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case SpvExecutionModelFragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SpvExecutionModelGLCompute:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	case SpvExecutionModelTaskEXT:
		return VK_SHADER_STAGE_TASK_BIT_EXT;
	case SpvExecutionModelMeshEXT:
		return VK_SHADER_STAGE_MESH_BIT_EXT;
	default:
		Logger::Error("Unsupported execution model");
		return VkShaderStageFlagBits(0);
	}
}

static VkDescriptorType GetDescriptorType(SpvOp op)
{
	switch (op)
	{
	case SpvOpTypeStruct:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case SpvOpTypeImage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case SpvOpTypeSampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case SpvOpTypeSampledImage:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	default:
		Logger::Error("Unknown resource type");
		return VkDescriptorType(0);
	}
}

static VkFormat GetFormat(const Vector<Id>& ids, const Id& type)
{
	switch (type.opcode)
	{
	case SpvOpTypeInt: {
		switch (type.width)
		{
		case 8: { if (type.sign) { return VK_FORMAT_R8_SINT; } return VK_FORMAT_R8_UINT; }
		case 16: { if (type.sign) { return VK_FORMAT_R16_SINT; } return VK_FORMAT_R16_UINT; }
		case 32: { if (type.sign) { return VK_FORMAT_R32_SINT; } return VK_FORMAT_R32_UINT; }
		case 64: { if (type.sign) { return VK_FORMAT_R64_SINT; } return VK_FORMAT_R64_UINT; }
		}
	} break;
	case SpvOpTypeFloat: {
		switch (type.width)
		{
		case 32: { return VK_FORMAT_R32_SFLOAT; }
		case 64: { return VK_FORMAT_R64_SFLOAT; }
		}
	} break;
	case SpvOpTypeVector: {
		const Id& component = ids[type.typeId];

		switch (type.count)
		{
		case 2: {
			switch (component.opcode)
			{
			case SpvOpTypeFloat: {
				switch (component.width)
				{
				case 32: { return VK_FORMAT_R32G32_SFLOAT; }
				case 64: { return VK_FORMAT_R64G64_SFLOAT; }
				}
			} break;
			case SpvOpTypeInt: {
				switch (component.width)
				{
				case 8: { if (type.sign) { return VK_FORMAT_R8G8_SINT; } return VK_FORMAT_R8G8_UINT; }
				case 16: { if (type.sign) { return VK_FORMAT_R16G16_SINT; } return VK_FORMAT_R16G16_UINT; }
				case 32: { if (type.sign) { return VK_FORMAT_R32G32_SINT; } return VK_FORMAT_R32G32_UINT; }
				case 64: { if (type.sign) { return VK_FORMAT_R64G64_SINT; } return VK_FORMAT_R64G64_UINT; }
				}
			} break;
			}
		} break;
		case 3: {
			switch (component.opcode)
			{
			case SpvOpTypeFloat: {
				switch (component.width)
				{
				case 32: { return VK_FORMAT_R32G32B32_SFLOAT; }
				case 64: { return VK_FORMAT_R64G64B64_SFLOAT; }
				}
			} break;
			case SpvOpTypeInt: {
				switch (component.width)
				{
				case 8: { if (type.sign) { return VK_FORMAT_R8G8B8_SINT; } return VK_FORMAT_R8G8B8_UINT; }
				case 16: { if (type.sign) { return VK_FORMAT_R16G16B16_SINT; } return VK_FORMAT_R16G16B16_UINT; }
				case 32: { if (type.sign) { return VK_FORMAT_R32G32B32_SINT; } return VK_FORMAT_R32G32B32_UINT; }
				case 64: { if (type.sign) { return VK_FORMAT_R64G64B64_SINT; } return VK_FORMAT_R64G64B64_UINT; }
				}
			} break;
			}
		} break;
		case 4: {
			switch (component.opcode)
			{
			case SpvOpTypeFloat: {
				switch (component.width)
				{
				case 32: { return VK_FORMAT_R32G32B32A32_SFLOAT; }
				case 64: { return VK_FORMAT_R64G64B64A64_SFLOAT; }
				}
			} break;
			case SpvOpTypeInt: {
				switch (component.width)
				{
				case 8: { if (type.sign) { return VK_FORMAT_R8G8B8A8_SINT; } return VK_FORMAT_R8G8B8A8_UINT; }
				case 16: { if (type.sign) { return VK_FORMAT_R16G16B16A16_SINT; } return VK_FORMAT_R16G16B16A16_UINT; }
				case 32: { if (type.sign) { return VK_FORMAT_R32G32B32A32_SINT; } return VK_FORMAT_R32G32B32A32_UINT; }
				case 64: { if (type.sign) { return VK_FORMAT_R64G64B64A64_SINT; } return VK_FORMAT_R64G64B64A64_UINT; }
				}
			} break;
			}
		} break;
		}
	} break;
	}

	Logger::Error("Unkown Format!");
	BreakPoint;
	return VK_FORMAT_UNDEFINED;
}

static U32 FormatStride(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:	return 1;
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT: return 2;
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT: return 4;
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT: return 8;
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT: return 12;
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
	default: {
		Logger::Error("Unknown format stride!");
		BreakPoint;
		return 0;
	}
	}
}

VkPipelineShaderStageCreateInfo Shader::ParseSPIRV(U32* code, U32 codeSize, ShaderStage& stage)
{
	uint32_t idBound = code[3];

	Vector<Id> ids(idBound, {});

	const U32* it = code + 5;

	I32 localSizeIdX = -1;
	I32 localSizeIdY = -1;
	I32 localSizeIdZ = -1;

	while (it != code + codeSize)
	{
		U16 opcode = (U16)it[0];
		U16 wordCount = (U16)(it[0] >> 16);

		switch (opcode)
		{
		case SpvOpEntryPoint:
		{
			stage.stage = (ShaderStageType)GetShaderStage((SpvExecutionModel)it[1]);
			entryPoint = (C8*)(it + 3);
		} break;
		case SpvOpExecutionMode:
		{
			U32 mode = it[2];

			switch (mode)
			{
			case SpvExecutionModeLocalSize: {
				localSizeX = it[3];
				localSizeY = it[4];
				localSizeZ = it[5];
			} break;
			}
		} break;
		case SpvOpExecutionModeId:
		{
			U32 mode = it[2];

			switch (mode)
			{
			case SpvExecutionModeLocalSizeId: {
				localSizeIdX = I32(it[3]);
				localSizeIdY = I32(it[4]);
				localSizeIdZ = I32(it[5]);
			} break;
			}
		} break;
		case SpvOpDecorate:
		{
			U32 id = it[1];

			switch (it[2])
			{
			case SpvDecorationBinding: { ids[id].binding = it[3]; } break;
			case SpvDecorationDescriptorSet: { ids[id].set = it[3]; } break;
			case SpvDecorationLocation: { ids[id].location = it[3]; } break;
			case SpvDecorationInputAttachmentIndex: { ids[id].inputIndex = it[3]; ids[id].inputAttachment = true; } break;
			}
		} break;
		case SpvOpTypeInt: {
			Id& id = ids[it[1]];
			id.opcode = opcode;
			id.width = (U8)it[2];
			id.sign = (U8)it[3];
		} break;
		case SpvOpTypeFloat: {
			Id& id = ids[it[1]];
			id.opcode = opcode;
			id.width = (U8)it[2];
		} break;
		case SpvOpTypeVector: {
			Id& id = ids[it[1]];
			id.opcode = opcode;
			id.typeId = it[2];
			id.count = it[3];
		} break;
		case SpvOpTypeMatrix: {
			Id& id = ids[it[1]];
			id.opcode = opcode;
			id.typeId = it[2];
			id.count = it[3];
		} break;
		case SpvOpTypeStruct:
		case SpvOpTypeImage:
		case SpvOpTypeSampler:
		case SpvOpTypeSampledImage: { ids[it[1]].opcode = opcode; } break;
		case SpvOpTypePointer:
		{
			U32 id = it[1];
			ids[id].opcode = opcode;
			ids[id].typeId = it[3];
			ids[id].storageClass = it[2];
		} break;
		case SpvOpConstant:
		{
			//TODO: Support constants bigger than 32-bit
			U32 id = it[2];
			ids[id].opcode = opcode;
			ids[id].typeId = it[1];
			ids[id].constant = it[3];
		} break;
		case SpvOpVariable:
		{
			U32 id = it[2];
			ids[id].opcode = opcode;
			ids[id].typeId = it[1];
			ids[id].storageClass = it[3];
		} break;
		}

		it += wordCount;
	}

	for (const Id& id : ids)
	{
		if (id.set == 1 && id.binding == 10) { useBindless = true; continue; }
		else if (id.set == 1) { Logger::Error("Multiple Descriptor Sets Aren't Supported!"); }

		if (id.opcode == SpvOpVariable)
		{
			switch (id.storageClass)
			{
			case SpvStorageClassInput: {
				if (stage.stage == VK_SHADER_STAGE_VERTEX_BIT && useVertices)
				{
					Id& type = ids[ids[id.typeId].typeId];

					if (type.opcode == SpvOpTypeMatrix)
					{
						const Id& component = ids[type.typeId];

						switch (type.count)
						{
						case 2: {
							VkVertexInputAttributeDescription attribute{};
							attribute.location = id.location;
							attribute.binding = id.location >= instanceLocation;
							attribute.format = GetFormat(ids, component);
							attribute.offset = 0;

							shaderInfo->vertexAttributes[id.location] = attribute;

							attribute.location = id.location + 1;
							shaderInfo->vertexAttributes[id.location + 1] = attribute;

							shaderInfo->vertexAttributeCount += 2;
						} break;
						case 3: {
							VkVertexInputAttributeDescription attribute{};
							attribute.location = id.location;
							attribute.binding = id.location >= instanceLocation;
							attribute.format = GetFormat(ids, component);
							attribute.offset = 0;

							shaderInfo->vertexAttributes[id.location] = attribute;

							attribute.location = id.location + 1;
							shaderInfo->vertexAttributes[id.location + 1] = attribute;

							attribute.location = id.location + 2;
							shaderInfo->vertexAttributes[id.location + 2] = attribute;

							shaderInfo->vertexAttributeCount += 3;
						} break;
						case 4: {
							VkVertexInputAttributeDescription attribute{};
							attribute.location = id.location;
							attribute.binding = id.location >= instanceLocation;
							attribute.format = GetFormat(ids, component);
							attribute.offset = 0;

							shaderInfo->vertexAttributes[id.location] = attribute;

							attribute.location = id.location + 1;
							shaderInfo->vertexAttributes[id.location + 1] = attribute;

							attribute.location = id.location + 2;
							shaderInfo->vertexAttributes[id.location + 2] = attribute;

							attribute.location = id.location + 3;
							shaderInfo->vertexAttributes[id.location + 3] = attribute;

							shaderInfo->vertexAttributeCount += 4;
						} break;
						}
					}
					else
					{
						VkVertexInputAttributeDescription attribute{};
						attribute.location = id.location;
						attribute.binding = id.location >= instanceLocation;
						attribute.format = GetFormat(ids, type);
						attribute.offset = 0;

						shaderInfo->vertexAttributes[id.location] = attribute;

						++shaderInfo->vertexAttributeCount;
					}
				}
			} break;
			case SpvStorageClassPushConstant:
			{
				stage.usePushConstants = true;
				pushConstantStages |= stage.stage;
			} break;
			case SpvStorageClassUniform:
			case SpvStorageClassUniformConstant:
			case SpvStorageClassStorageBuffer: {
				U32 type = ids[ids[id.typeId].typeId].opcode;
				VkDescriptorType descriptorType;
				if (id.inputAttachment)
				{
					subpass.inputAttachments[subpass.inputAttachmentCount++] = id.inputIndex;
					descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				}
				else { descriptorType = GetDescriptorType(SpvOp(type)); }

				VkDescriptorSetLayoutBinding& binding = setLayout->bindings[id.binding];
				binding.descriptorType = descriptorType;
				binding.stageFlags |= stage.stage;
				binding.binding = id.binding;
				binding.descriptorCount = 1;

				setLayout->bindingCount = Math::Max(setLayout->bindingCount, (U8)(id.binding + 1));
				useSet0 = true;
			} break;
			case SpvStorageClassOutput: {
				if (stage.stage == VK_SHADER_STAGE_FRAGMENT_BIT) { ++outputCount; }
			} break;
			}
		}
		else if (id.opcode == SpvDecorationSpecId)
		{
			Id id0 = ids[ids[id.typeId].typeId];
			Id id1 = ids[id.typeId];
			BreakPoint;
		}
	}

	if (stage.stage == VK_SHADER_STAGE_VERTEX_BIT && shaderInfo->vertexAttributeCount)
	{
		if (instanceLocation != U8_MAX)
		{
			for (U32 i = 1; i < instanceLocation; ++i)
			{
				shaderInfo->vertexAttributes[i].offset = shaderInfo->vertexAttributes[i - 1].offset + FormatStride(shaderInfo->vertexAttributes[i - 1].format);
			}

			VkVertexInputBindingDescription binding{};
			binding.binding = 0;
			binding.stride = shaderInfo->vertexAttributes[instanceLocation - 1].offset + FormatStride(shaderInfo->vertexAttributes[instanceLocation - 1].format);
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			shaderInfo->vertexStreams[shaderInfo->vertexStreamCount++] = binding;

			vertexSize = binding.stride;

			for (U32 i = instanceLocation + 1; i < shaderInfo->vertexAttributeCount; ++i)
			{
				shaderInfo->vertexAttributes[i].offset = shaderInfo->vertexAttributes[i - 1].offset + FormatStride(shaderInfo->vertexAttributes[i - 1].format);
			}

			binding.binding = 1;
			binding.stride = shaderInfo->vertexAttributes[shaderInfo->vertexAttributeCount - 1].offset + FormatStride(shaderInfo->vertexAttributes[shaderInfo->vertexAttributeCount - 1].format);
			binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			instanceStride = binding.stride;

			shaderInfo->vertexStreams[shaderInfo->vertexStreamCount++] = binding;
		}
		else
		{
			for (U32 i = 1; i < shaderInfo->vertexAttributeCount; ++i)
			{
				shaderInfo->vertexAttributes[i].offset = shaderInfo->vertexAttributes[i - 1].offset + FormatStride(shaderInfo->vertexAttributes[i - 1].format);
			}

			VkVertexInputBindingDescription binding{};
			binding.binding = 0;
			binding.stride = shaderInfo->vertexAttributes[shaderInfo->vertexAttributeCount - 1].offset + FormatStride(shaderInfo->vertexAttributes[shaderInfo->vertexAttributeCount - 1].format);
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			shaderInfo->vertexStreams[shaderInfo->vertexStreamCount++] = binding;
		}
	}

	if (stage.stage == VK_SHADER_STAGE_COMPUTE_BIT)
	{
		if (localSizeIdX >= 0) { localSizeX = ids[localSizeIdX].constant; }
		if (localSizeIdY >= 0) { localSizeY = ids[localSizeIdY].constant; }
		if (localSizeIdZ >= 0) { localSizeZ = ids[localSizeIdZ].constant; }
	}

	//TODO: Push Constants

	VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleInfo.codeSize = codeSize * sizeof(U32);
	moduleInfo.pCode = code;

	VkShaderModule module;
	vkCreateShaderModule(Renderer::device, &moduleInfo, Renderer::allocationCallbacks, &module);

	VkPipelineShaderStageCreateInfo shaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	shaderStageInfo.pNext = nullptr;
	shaderStageInfo.flags = 0;
	shaderStageInfo.module = module;
	shaderStageInfo.stage = (VkShaderStageFlagBits)stage.stage;
	shaderStageInfo.pName = entryPoint.Data();

	return shaderStageInfo;
}

void Shader::FillOutShaderInfo(VkGraphicsPipelineCreateInfo& pipelineInfo, VkPipelineVertexInputStateCreateInfo& vertexInput, 
	VkPipelineColorBlendStateCreateInfo& colorBlending, const Specialization* specialization)
{
	vertexInput.vertexAttributeDescriptionCount = shaderInfo->vertexAttributeCount;
	vertexInput.pVertexAttributeDescriptions = shaderInfo->vertexAttributes;
	vertexInput.vertexBindingDescriptionCount = shaderInfo->vertexStreamCount;
	vertexInput.pVertexBindingDescriptions = shaderInfo->vertexStreams;

	colorBlending.attachmentCount = shaderInfo->blendStateCount;
	colorBlending.pAttachments = shaderInfo->blendStates;

	for (U32 i = 0; i < shaderInfo->stageCount; ++i)
	{
		shaderInfo->stageInfos[i].pSpecializationInfo = reinterpret_cast<const VkSpecializationInfo*>(specialization);
	}

	pipelineInfo.stageCount = shaderInfo->stageCount;

	pipelineInfo.pStages = shaderInfo->stageInfos;
}

void Shader::FillOutShaderInfo(VkComputePipelineCreateInfo& pipelineInfo, const Specialization* specialization)
{
	shaderInfo->stageInfos[0].pSpecializationInfo = reinterpret_cast<const VkSpecializationInfo*>(specialization);

	pipelineInfo.stage = shaderInfo->stageInfos[0];
}

void Shader::AddDescriptor(const Descriptor& descriptor)
{
	descriptors[descriptorCount++] = descriptor;
}

void Shader::PushDescriptors(CommandBuffer* commandBuffer)
{
	if (Renderer::pushDescriptorsSupported)
	{
		//vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer->commandBuffer, shader->setLayouts[0]->updateTemplate, shader->pipelineLayout, 0, shader->descriptors);
	}
	else
	{
		VkDescriptorSet sets[]{ nullptr, bindlessDescriptorSet };

		if (useSet0)
		{
			VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

			allocateInfo.descriptorPool = Renderer::descriptorPools[Renderer::frameIndex];
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &setLayout->descriptorSetLayout;

			VkValidate(vkAllocateDescriptorSets(Renderer::device, &allocateInfo, sets));

			vkUpdateDescriptorSetWithTemplate(Renderer::device, sets[0], setLayout->updateTemplate, descriptors);
		}

		commandBuffer->BindDescriptorSets(this, !useSet0, useSet0 + useBindless, sets + !useSet0);
	}
}