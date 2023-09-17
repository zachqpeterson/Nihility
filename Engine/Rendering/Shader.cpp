#include "Shader.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"

//TODO: #define ENABLE_OPT for optimised shaders
#include "External\LunarG\glslang\SPIRV\GlslangToSpv.h"
#include "External\LunarG\glslang\Public\ShaderLang.h"
#include "External\LunarG\spirv_cross\spirv_reflect.h"

#if defined(_MSC_VER)
#include "External\LunarG\spirv-headers\spirv.h"
#else
#include "External\LunarG\spirv_cross\spirv.h"
#endif

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

	U32 width;
	U32 sign;
	U32 count;

	U32 constant;
};

bool Shader::Create(const String& shaderPath, U8 pushConstantCount, VkPushConstantRange* pushConstants)
{
	String data{  };
	Resources::LoadBinary(shaderPath, data);

	DescriptorSetLayoutInfo setLayoutInfos[MAX_DESCRIPTOR_SET_LAYOUTS]{};

	I64 index = -1;

	do
	{
		index = data.IndexOf('#', index + 1);

		//TODO: Validate stage combinations
		if (data.CompareN("#CONFIG", index)) { ParseConfig(data, index); }
		else if (data.CompareN("#VERTEX", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_VERTEX_BIT); }
		else if (data.CompareN("#CONTROL", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT); }
		else if (data.CompareN("#EVALUATION", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT); }
		else if (data.CompareN("#GEOMETRY", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_GEOMETRY_BIT); }
		else if (data.CompareN("#FRAGMENT", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_FRAGMENT_BIT); }
		else if (data.CompareN("#COMPUTE", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_COMPUTE_BIT); }
		else if (data.CompareN("#TASK", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_TASK_BIT_EXT); }
		else if (data.CompareN("#MESH", index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_MESH_BIT_EXT); }
	} while (index != -1);

	if (stageCount == 0) { Logger::Error("Shader '{}' Has No Stages!", shaderPath); return false; }

	VkDescriptorSetLayout vkLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];

	if (Renderer::bindlessSupported && useBindless)
	{
		setLayouts[setCount] = &Resources::bindlessDescriptorSetLayout;
		vkLayouts[setCount] = Resources::bindlessDescriptorSetLayout.descriptorSetLayout;
	}

	for (U8 i = 0; i < setCount; ++i)
	{
		setLayouts[i] = Resources::CreateDescriptorSetLayout(setLayoutInfos[i]);
		vkLayouts[i] = setLayouts[i]->descriptorSetLayout;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.pNext = nullptr;
	pipelineLayoutInfo.pSetLayouts = vkLayouts;
	pipelineLayoutInfo.setLayoutCount = setCount + useBindless;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantCount;
	pipelineLayoutInfo.pPushConstantRanges = pushConstants;

	VkValidate(vkCreatePipelineLayout(Renderer::device, &pipelineLayoutInfo, Renderer::allocationCallbacks, &pipelineLayout));

	for (U8 i = 0; i < setCount; ++i)
	{
		Renderer::CreateDescriptorUpdateTemplate(setLayouts[i], this);
	}

	return true;
}

void Shader::Destroy()
{
	for (U8 i = 0; i < stageCount; ++i)
	{
		vkDestroyShaderModule(Renderer::device, stageInfos[i].module, Renderer::allocationCallbacks);
	}

	for (U8 i = 0; i < setCount; ++i)
	{
		Resources::DestroyDescriptorSetLayout(setLayouts[i]);
	}

	stageCount = 0;

	if (pipelineLayout) { vkDestroyPipelineLayout(Renderer::device, pipelineLayout, Renderer::allocationCallbacks); pipelineLayout = nullptr; }

	for (U32 i = 0; i < MAX_SHADER_STAGES; ++i)
	{
		stages[i].entryPoint.Destroy();
	}

	name.Destroy();
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
		else if (data.CompareN("language", index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN("GLSL", index + 1)) { language = glslang::EShSourceGlsl; }
			else if (data.CompareN("HLSL", index + 1)) { language = glslang::EShSourceHlsl; }
		}
		else if (data.CompareN("cull", index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN("NONE", index + 1)) { rasterization.cullMode = VK_CULL_MODE_NONE; }
			else if (data.CompareN("FRONT", index + 1)) { rasterization.cullMode = VK_CULL_MODE_FRONT_BIT; }
			else if (data.CompareN("BACK", index + 1)) { rasterization.cullMode = VK_CULL_MODE_BACK_BIT; }
			else if (data.CompareN("BOTH", index + 1)) { rasterization.cullMode = VK_CULL_MODE_FRONT_AND_BACK; }
		}
		else if (data.CompareN("front", index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN("CLOCKWISE", index + 1)) { rasterization.front = VK_FRONT_FACE_CLOCKWISE; }
			else if (data.CompareN("COUNTER", index + 1)) { rasterization.front = VK_FRONT_FACE_COUNTER_CLOCKWISE; }
		}
		else if (data.CompareN("fill", index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN("SOLID", index + 1)) { rasterization.fill = VK_POLYGON_MODE_FILL; }
			else if (data.CompareN("LINE", index + 1)) { rasterization.fill = VK_POLYGON_MODE_LINE; }
			else if (data.CompareN("POINT", index + 1)) { rasterization.fill = VK_POLYGON_MODE_POINT; }
		}
		else if (data.CompareN("depth", index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN("NONE", index + 1))
			{
				depthStencil.depthEnable = false;
				depthStencil.depthWriteEnable = false;
				depthStencil.depthComparison = VK_COMPARE_OP_ALWAYS;
			}
			else if (data.CompareN("LESS_EQUAL", index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_LESS_OR_EQUAL;
			}
			else if (data.CompareN("LESS", index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_LESS;
			}
			else if (data.CompareN("GREATER_EQUAL", index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
			}
			else if (data.CompareN("GREATER", index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_GREATER;
			}
			else if (data.CompareN("EQUAL", index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_EQUAL;
			}
		}
		else if (data.CompareN("blend", index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN("ADD", index + 1))
			{
				VkPipelineColorBlendAttachmentState& blendState = blendStates[blendStateCount];

				blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendState.colorBlendOp = VK_BLEND_OP_ADD;

				blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendState.alphaBlendOp = VK_BLEND_OP_ADD;

				blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

				blendState.blendEnable = VK_TRUE;

				++blendStateCount;
			}
			else if (data.CompareN("SUB", index + 1))
			{
				VkPipelineColorBlendAttachmentState& blendState = blendStates[blendStateCount];

				blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendState.colorBlendOp = VK_BLEND_OP_SUBTRACT;

				blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

				blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

				blendState.blendEnable = VK_TRUE;

				++blendStateCount;
			}
		}
		else if (data.CompareN("instanceOffset", index + 1))
		{
			index = data.IndexOf('=', index + 1);
			instanceOffset = data.ToType<U8>(index + 1);
		}

		index = data.IndexOf('\n', index + 1);
	}

	return false;
}

bool Shader::ParseStage(const String& data, I64& index, DescriptorSetLayoutInfo* setLayoutInfos, VkShaderStageFlagBits stage)
{
	if (stage != VK_SHADER_STAGE_COMPUTE_BIT) { bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; }
	else { bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; }

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

	stages[stageCount].stage = stage;
	stageInfos[stageCount] = CompileShader(stages[stageCount], code, name, setLayoutInfos);
	++stageCount;

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

//TODO: Support HLSL
//TODO: Cache compiled shaders
VkPipelineShaderStageCreateInfo Shader::CompileShader(ShaderStage& shaderStage, String& code, const String& name, DescriptorSetLayoutInfo* setLayoutInfos)
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
	tShader.setEnvInput((EShSource)language, stage, EShClientVulkan, 450);
	tShader.setEnvClient(EShClientVulkan, EShTargetVulkan_1_3);
	tShader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_6);
	if (!tShader.parse(&resources, 450, false, EShMsgDefault)) { Logger::Error(tShader.getInfoLog()); }

	TProgram tProgram;
	tProgram.addShader(&tShader);
	if (!tProgram.link(EShMsgDefault)) { Logger::Error(tProgram.getInfoLog()); }

	tProgram.getIntermediate(stage);

	std::vector<U32> spv;
	GlslangToSpv(*tProgram.getIntermediate(stage), spv);

	VkShaderModuleCreateInfo shaderInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	shaderInfo.codeSize = spv.size() * 4;
	shaderInfo.pCode = spv.data();

	ParseSPIRV((U32*)shaderInfo.pCode, spv.size(), shaderStage, setLayoutInfos);

	VkShaderModule module;
	vkCreateShaderModule(Renderer::device, &shaderInfo, Renderer::allocationCallbacks, &module);

	VkPipelineShaderStageCreateInfo shaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	shaderStageInfo.pNext = nullptr;
	shaderStageInfo.flags = 0;
	shaderStageInfo.module = module;
	shaderStageInfo.stage = shaderStage.stage;
	shaderStageInfo.pName = shaderStage.entryPoint.Data();

	return shaderStageInfo;
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

bool Shader::ParseSPIRV(U32* code, U64 codeSize, ShaderStage& stage, DescriptorSetLayoutInfo* setLayoutInfos)
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
			stage.stage = GetShaderStage((SpvExecutionModel)it[1]);
			stage.entryPoint = (C8*)(it + 3);
		} break;
		case SpvOpExecutionMode:
		{
			U32 mode = it[2];

			switch (mode)
			{
			case SpvExecutionModeLocalSize: {
				stage.localSizeX = it[3];
				stage.localSizeY = it[4];
				stage.localSizeZ = it[5];
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

		if (id.opcode == SpvOpVariable)
		{
			switch (id.storageClass)
			{
			case SpvStorageClassInput: {
				if (stage.stage == VK_SHADER_STAGE_VERTEX_BIT)
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
							attribute.binding = id.location >= instanceOffset;
							attribute.format = GetFormat(ids, component);
							attribute.offset = 0;

							vertexAttributes[id.location] = attribute;

							attribute.location = id.location + 1;
							vertexAttributes[id.location + 1] = attribute;

							vertexAttributeCount += 2;
						} break;
						case 3: {
							VkVertexInputAttributeDescription attribute{};
							attribute.location = id.location;
							attribute.binding = id.location >= instanceOffset;
							attribute.format = GetFormat(ids, component);
							attribute.offset = 0;

							vertexAttributes[id.location] = attribute;

							attribute.location = id.location + 1;
							vertexAttributes[id.location + 1] = attribute;

							attribute.location = id.location + 2;
							vertexAttributes[id.location + 2] = attribute;

							vertexAttributeCount += 3;
						} break;
						case 4: {
							VkVertexInputAttributeDescription attribute{};
							attribute.location = id.location;
							attribute.binding = id.location >= instanceOffset;
							attribute.format = GetFormat(ids, component);
							attribute.offset = 0;

							vertexAttributes[id.location] = attribute;

							attribute.location = id.location + 1;
							vertexAttributes[id.location + 1] = attribute;

							attribute.location = id.location + 2;
							vertexAttributes[id.location + 2] = attribute;

							attribute.location = id.location + 3;
							vertexAttributes[id.location + 3] = attribute;

							vertexAttributeCount += 4;
						} break;
						}
					}
					else
					{
						VkVertexInputAttributeDescription attribute{};
						attribute.location = id.location;
						attribute.binding = id.location >= instanceOffset;
						attribute.format = GetFormat(ids, type);
						attribute.offset = 0;

						vertexAttributes[id.location] = attribute;

						++vertexAttributeCount;
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
				VkDescriptorType descriptorType = GetDescriptorType(SpvOp(type));
				VkDescriptorSetLayoutBinding& binding = setLayoutInfos[id.set].bindings[id.binding];
				binding.descriptorType = descriptorType;
				binding.stageFlags |= stage.stage;
				binding.binding = id.binding;
				binding.descriptorCount = 1;

				setLayoutInfos[id.set].bindingCount = Math::Max(setLayoutInfos[id.set].bindingCount, (U8)(id.binding + 1));
				setCount = Math::Max(setCount, id.set + 1);
			} break;
			case SpvStorageClassOutput: {
				if (stage.stage == VK_SHADER_STAGE_FRAGMENT_BIT)
				{
					outputs[outputCount++] = Renderer::swapchain.renderTargets[0]->format;
				}
			}
			}
		}
		else if (id.opcode == SpvDecorationSpecId)
		{
			Id id0 = ids[ids[id.typeId].typeId];
			Id id1 = ids[id.typeId];
			BreakPoint;
		}
	}

	if (stage.stage == VK_SHADER_STAGE_VERTEX_BIT && vertexAttributeCount)
	{
		if (instanceOffset != U8_MAX)
		{
			for (U32 i = 1; i < instanceOffset; ++i)
			{
				vertexAttributes[i].offset = vertexAttributes[i - 1].offset + FormatStride(vertexAttributes[i - 1].format);
			}

			VkVertexInputBindingDescription binding{};
			binding.binding = 0;
			binding.stride = vertexAttributes[instanceOffset - 1].offset + FormatStride(vertexAttributes[instanceOffset - 1].format);
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			vertexStreams[vertexStreamCount++] = binding;

			for (U32 i = instanceOffset + 1; i < vertexAttributeCount; ++i)
			{
				vertexAttributes[i].offset = vertexAttributes[i - 1].offset + FormatStride(vertexAttributes[i - 1].format);
			}

			binding.binding = 1;
			binding.stride = vertexAttributes[vertexAttributeCount - 1].offset + FormatStride(vertexAttributes[vertexAttributeCount - 1].format);
			binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			vertexStreams[vertexStreamCount++] = binding;
		}
		else
		{
			for (U32 i = 1; i < vertexAttributeCount; ++i)
			{
				vertexAttributes[i].offset = vertexAttributes[i - 1].offset + FormatStride(vertexAttributes[i - 1].format);
			}

			VkVertexInputBindingDescription binding{};
			binding.binding = 0;
			binding.stride = vertexAttributes[vertexAttributeCount - 1].offset + FormatStride(vertexAttributes[vertexAttributeCount - 1].format);
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			vertexStreams[vertexStreamCount++] = binding;
		}
	}

	if (stage.stage == VK_SHADER_STAGE_COMPUTE_BIT)
	{
		if (localSizeIdX >= 0) { stage.localSizeX = ids[localSizeIdX].constant; }
		if (localSizeIdY >= 0) { stage.localSizeY = ids[localSizeIdY].constant; }
		if (localSizeIdZ >= 0) { stage.localSizeZ = ids[localSizeIdZ].constant; }
	}

	//TODO: Push Constants

	return true;
}