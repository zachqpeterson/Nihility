#include "Pipeline.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Platform.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"

//#include "External\glslang\Include\glslang_c_interface.h"
#include "External\glslang\Public\ShaderLang.h"
#include "External\spirv_cross\spirv_reflect.h"
#include "External\SPIRV\GlslangToSpv.h"

#if defined(_MSC_VER)
#include <spirv-headers\spirv.h>
#else
#include <spirv_cross\spirv.h>
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

String Pipeline::CONFIG_BEGIN{ "#CONFIG" };
String Pipeline::CONFIG_END{ "#CONFIG_END" };
String Pipeline::VERT_BEGIN{ "#VERTEX" };
String Pipeline::VERT_END{ "#VERTEX_END" };
String Pipeline::CTRL_BEGIN{ "#CONTROL" };
String Pipeline::CTRL_END{ "#CONTROL_END" };
String Pipeline::EVAL_BEGIN{ "#EVALUATION" };
String Pipeline::EVAL_END{ "#EVALUATION_END" };
String Pipeline::GEOM_BEGIN{ "#GEOMETRY" };
String Pipeline::GEOM_END{ "#GEOMETRY_END" };
String Pipeline::FRAG_BEGIN{ "#FRAGMENT" };
String Pipeline::FRAG_END{ "#FRAGMENT_END" };
String Pipeline::COMP_BEGIN{ "#COMPUTE" };
String Pipeline::COMP_END{ "#COMPUTE_END" };

struct Member
{
	U32	idIndex;
	U32	offset;
};

struct Id
{
	SpvOp	op;
	U32		set;
	U32		binding;
	U32		location;

	// For integers and floats
	U8		width;
	U8		sign;

	// For arrays, vectors and matrices
	U32		typeIndex;
	U32		count;

	// For variables
	SpvStorageClass	storageClass;

	// For constants
	U32				value;

	// For structs
	Vector<Member>	members;
};

//void Shader::SetSpecializationData(const SpecializationData& data)
//{
//	Memory::Copy(specializationInfos[data.stage].specializationBuffer + specializationInfos[data.stage].specializationData[data.index].offset,
//		data.data, specializationInfos[data.stage].specializationData[data.index].size);
//}

bool Pipeline::Create(const String& shaderPath)
{
	String data{ NO_INIT };
	Resources::LoadBinary(shaderPath, data);

	DescriptorSetLayoutCreation setLayoutInfos[MAX_DESCRIPTOR_SETS]{};

	I64 index = -1;

	do
	{
		index = data.IndexOf('#', index + 1);

		//TODO: Validate only graphics or compute
		//TODO: Validate both or neither ctrl and eval
		if (data.CompareN(CONFIG_BEGIN, CONFIG_BEGIN.Size(), index)) { ParseConfig(data, index); }
		else if (data.CompareN(VERT_BEGIN, VERT_BEGIN.Size(), index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_VERTEX_BIT); }
		else if (data.CompareN(CTRL_BEGIN, CTRL_BEGIN.Size(), index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT); }
		else if (data.CompareN(EVAL_BEGIN, EVAL_BEGIN.Size(), index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT); }
		else if (data.CompareN(GEOM_BEGIN, GEOM_BEGIN.Size(), index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_GEOMETRY_BIT); }
		else if (data.CompareN(FRAG_BEGIN, FRAG_BEGIN.Size(), index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_FRAGMENT_BIT); }
		else if (data.CompareN(COMP_BEGIN, COMP_BEGIN.Size(), index)) { ParseStage(data, index, setLayoutInfos, VK_SHADER_STAGE_COMPUTE_BIT); }
	} while (index != -1);

	if (shader.shaderCount == 0) { Logger::Error("Pipeline '{}' Has No Shaders Stages!", shaderPath); BreakPoint; }

	VkDescriptorSetLayout vkLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];

	if (Renderer::bindlessSupported && useBindless)
	{
		vkLayouts[--shader.setCount] = Resources::bindlessDescriptorSetLayout;
	}

	descriptorSetCount = shader.setCount;

	for (U8 i = 0; i < descriptorSetCount; ++i)
	{
		shader.setLayouts[i] = Resources::CreateDescriptorSetLayout(setLayoutInfos[i]);
		vkLayouts[i] = shader.setLayouts[i]->descriptorSetLayout;

		for (U8 j = 0; j < MAX_SWAPCHAIN_IMAGES; ++j)
		{
			descriptorSets[j][i] = Resources::CreateDescriptorSet(shader.setLayouts[i]);
		}
	}

	RenderPassCreation renderPassInfo{};
	renderPassInfo.SetType(RENDERPASS_TYPE_GEOMETRY).SetName(name + "_pass");
	renderPassInfo.SetOperations(RENDER_PASS_OP_CLEAR, RENDER_PASS_OP_CLEAR, RENDER_PASS_OP_DONT_CARE);
	renderPassInfo.width = Settings::WindowWidth();
	renderPassInfo.height = Settings::WindowHeight();

	String textureName = name + "_output_";

	for (U32 i = 0; i < shader.outputCount; ++i)
	{
		TextureCreation textureInfo{};
		textureInfo.name = textureName + i;
		textureInfo.format = shader.outputs[i].format;
		textureInfo.width = renderPassInfo.width;
		textureInfo.height = renderPassInfo.height;
		textureInfo.depth = 1;
		textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET_MASK;
		textureInfo.type = TEXTURE_TYPE_2D;

		Texture* texture = Resources::CreateTexture(textureInfo);
		renderPassInfo.AddRenderTarget(texture);
	}

	renderpass = Resources::CreateRenderPass(renderPassInfo);

	if (!CreatePipeline(vkLayouts)) { return false; }

	return true;
}

void Pipeline::Destroy()
{
	for (U8 i = 0; i < shader.setCount; ++i)
	{
		Resources::DestroyDescriptorSetLayout(shader.setLayouts[i]);
	}

	for (U8 i = 0; i < descriptorSetCount; ++i)
	{
		for (U8 j = 0; j < MAX_SWAPCHAIN_IMAGES; ++j)
		{
			Resources::DestroyDescriptorSet(descriptorSets[i][j]);
		}
	}

	for (U8 i = 0; i < shader.shaderCount; ++i)
	{
		vkDestroyShaderModule(Renderer::device, shader.stages[i].module, Renderer::allocationCallbacks);
	}

	shader.shaderCount = 0;

	if (pipeline) { vkDestroyPipeline(Renderer::device, pipeline, Renderer::allocationCallbacks); }
	if (layout) { vkDestroyPipelineLayout(Renderer::device, layout, Renderer::allocationCallbacks); }
}

bool Pipeline::ParseConfig(const String& data, I64& index)
{
	static const String NAME{ "name" };
	static const String LANG{ "language" };
	static const String LANG_GLSL{ "GLSL" };
	static const String LANG_HLSL{ "HLSL" };
	static const String CULL{ "cull" };
	static const String CULL_NONE{ "NONE" };
	static const String CULL_FRONT{ "FRONT" };
	static const String CULL_BACK{ "BACK" };
	static const String CULL_BOTH{ "BOTH" };
	static const String FRONT{ "front" };
	static const String FRONT_CLOCKWISE{ "CLOCKWISE" };
	static const String FRONT_COUNTER{ "COUNTER" };
	static const String FILL{ "fill" };
	static const String FILL_SOLID{ "SOLID" };
	static const String FILL_LINE{ "SOLID" };
	static const String FILL_POINT{ "SOLID" };
	static const String DEPTH{ "depth" };
	static const String DEPTH_NONE{ "NONE" };
	static const String DEPTH_LESS{ "LESS" };
	static const String DEPTH_GREATER{ "GREATER" };
	static const String DEPTH_EQUAL{ "EQUAL" };
	static const String BLEND{ "blend" };
	static const String BLEND_ADD{ "ADD" };
	static const String BLEND_SUB{ "SUB" };

	//TODO: Blend Masks
	//TODO: Multiple Blend States
	//TODO: Add Stencils
	//TODO: Get Specialization Data
	while (index != -1)
	{
		if (data.CompareN(CONFIG_END, CONFIG_END.Size(), index + 1))
		{
			++index;
			return true;
		}
		else if (data.CompareN(NAME, NAME.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);
			data.SubString(name, index + 1, data.IndexOf('\n', index + 1) - index);
			name.Trim();
		}
		else if (data.CompareN(LANG, LANG.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN(LANG_GLSL, LANG_GLSL.Size(), index + 1)) { shader.language = glslang::EShSourceGlsl; }
			else if (data.CompareN(LANG_HLSL, LANG_HLSL.Size(), index + 1)) { shader.language = glslang::EShSourceHlsl; }
		}
		else if (data.CompareN(CULL, CULL.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN(CULL_NONE, CULL_NONE.Size(), index + 1)) { rasterization.cullMode = VK_CULL_MODE_NONE; }
			else if (data.CompareN(CULL_FRONT, CULL_FRONT.Size(), index + 1)) { rasterization.cullMode = VK_CULL_MODE_FRONT_BIT; }
			else if (data.CompareN(CULL_BACK, CULL_BACK.Size(), index + 1)) { rasterization.cullMode = VK_CULL_MODE_BACK_BIT; }
			else if (data.CompareN(CULL_BOTH, CULL_BOTH.Size(), index + 1)) { rasterization.cullMode = VK_CULL_MODE_FRONT_AND_BACK; }
		}
		else if (data.CompareN(FRONT, FRONT.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN(FRONT_CLOCKWISE, FRONT_CLOCKWISE.Size(), index + 1)) { rasterization.front = VK_FRONT_FACE_CLOCKWISE; }
			else if (data.CompareN(FRONT_COUNTER, FRONT_COUNTER.Size(), index + 1)) { rasterization.front = VK_FRONT_FACE_COUNTER_CLOCKWISE; }
		}
		else if (data.CompareN(FILL, FILL.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN(FILL_SOLID, FILL_SOLID.Size(), index + 1)) { rasterization.fill = VK_POLYGON_MODE_FILL; }
			else if (data.CompareN(FILL_LINE, FILL_LINE.Size(), index + 1)) { rasterization.fill = VK_POLYGON_MODE_LINE; }
			else if (data.CompareN(FILL_POINT, FILL_POINT.Size(), index + 1)) { rasterization.fill = VK_POLYGON_MODE_POINT; }
		}
		else if (data.CompareN(DEPTH, DEPTH.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN(DEPTH_NONE, DEPTH_NONE.Size(), index + 1))
			{
				depthStencil.depthEnable = false;
				depthStencil.depthWriteEnable = false;
				depthStencil.depthComparison = VK_COMPARE_OP_ALWAYS;
			}
			else if (data.CompareN(DEPTH_LESS, DEPTH_LESS.Size(), index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_LESS_OR_EQUAL;
			}
			else if (data.CompareN(DEPTH_GREATER, DEPTH_GREATER.Size(), index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
			}
			else if (data.CompareN(DEPTH_EQUAL, DEPTH_EQUAL.Size(), index + 1))
			{
				depthStencil.depthEnable = true;
				depthStencil.depthWriteEnable = true;
				depthStencil.depthComparison = VK_COMPARE_OP_EQUAL;
			}
		}
		else if (data.CompareN(BLEND, BLEND.Size(), index + 1))
		{
			index = data.IndexOf('=', index + 1);

			if (data.CompareN(BLEND_ADD, BLEND_ADD.Size(), index + 1))
			{
				BlendState& blendState = blendStates[blendStateCount];

				blendState.sourceColor = VK_BLEND_FACTOR_ONE;
				blendState.destinationColor = VK_BLEND_FACTOR_ONE;
				blendState.colorOperation = VK_BLEND_OP_ADD;

				blendState.sourceAlpha = VK_BLEND_FACTOR_ONE;
				blendState.destinationAlpha = VK_BLEND_FACTOR_ONE;
				blendState.alphaOperation = VK_BLEND_OP_ADD;

				blendState.colorWriteMask = COLOR_WRITE_ENABLE_ALL_MASK;

				blendState.blendEnabled = true;
				blendState.separateBlend = false; //Adding alpha

				++blendStateCount;
			}
			else if (data.CompareN(BLEND_SUB, BLEND_SUB.Size(), index + 1))
			{
				BlendState& blendState = blendStates[blendStateCount];

				blendState.sourceColor = VK_BLEND_FACTOR_ONE;
				blendState.destinationColor = VK_BLEND_FACTOR_ONE;
				blendState.colorOperation = VK_BLEND_OP_SUBTRACT;

				blendState.sourceAlpha = VK_BLEND_FACTOR_ONE;
				blendState.destinationAlpha = VK_BLEND_FACTOR_ONE;
				blendState.alphaOperation = VK_BLEND_OP_SUBTRACT;

				blendState.colorWriteMask = COLOR_WRITE_ENABLE_ALL_MASK;

				blendState.blendEnabled = true;
				blendState.separateBlend = false; //Adding alpha

				++blendStateCount;
			}
		}

		index = data.IndexOf('\n', index + 1);
	}

	return false;
}

bool Pipeline::ParseStage(const String& data, I64& index, DescriptorSetLayoutCreation* setLayoutInfos, VkShaderStageFlagBits stage)
{
	if (stage != VK_SHADER_STAGE_COMPUTE_BIT)
	{
		if (bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) { return false; }
		bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
	else
	{
		if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) { return false; }
		bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	}

	I64 begin = data.IndexOf('\n', index + 1) + 1;
	I64 end = begin;

	switch (stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { while (end != -1) { if (data.CompareN(VERT_END, VERT_END.Size(), end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { while (end != -1) { if (data.CompareN(CTRL_END, CTRL_END.Size(), end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { while (end != -1) { if (data.CompareN(EVAL_END, EVAL_END.Size(), end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_GEOMETRY_BIT: { while (end != -1) { if (data.CompareN(GEOM_END, GEOM_END.Size(), end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_FRAGMENT_BIT: { while (end != -1) { if (data.CompareN(FRAG_END, FRAG_END.Size(), end = data.IndexOf('#', end + 1))) { break; } } } break;
	case VK_SHADER_STAGE_COMPUTE_BIT: { while (end != -1) { if (data.CompareN(COMP_END, COMP_END.Size(), end = data.IndexOf('#', end + 1))) { break; } } } break;
	default: return false;
	}

	String code;
	data.SubString(code, begin, end - begin - 1);

	shader.stages[shader.shaderCount].stage = stage;
	shader.stageInfos[shader.shaderCount] = CompileShader(shader.stages[shader.shaderCount], code, name, setLayoutInfos);

	++shader.shaderCount;

	index = data.IndexOf('\n', end + 1);

	return true;
}

const String& Pipeline::ToStageDefines(VkShaderStageFlagBits value)
{
	static const String vertex{ "VERTEX" };
	static const String geometry{ "GEOMETRY" };
	static const String fragment{ "FRAGMENT" };
	static const String compute{ "COMPUTE" };
	static const String tessControl{ "TESSCONTROLL" };
	static const String tessEval{ "TESSEVAL" };
	static const String def{ "" };

	switch (value)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { return vertex; }
	case VK_SHADER_STAGE_GEOMETRY_BIT: { return geometry; }
	case VK_SHADER_STAGE_FRAGMENT_BIT: { return fragment; }
	case VK_SHADER_STAGE_COMPUTE_BIT: { return compute; }
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { return tessControl; }
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { return tessEval; }
	default: { return def; }
	}
}

const String& Pipeline::ToCompilerExtension(VkShaderStageFlagBits value)
{
	static const String vertex{ "vert" };
	static const String geometry{ "geom" };
	static const String fragment{ "frag" };
	static const String compute{ "comp" };
	static const String tessControl{ "tesc" };
	static const String tessEval{ "tese" };
	static const String def{ "" };

	switch (value)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { return vertex; }
	case VK_SHADER_STAGE_GEOMETRY_BIT: { return geometry; }
	case VK_SHADER_STAGE_FRAGMENT_BIT: { return fragment; }
	case VK_SHADER_STAGE_COMPUTE_BIT: { return compute; }
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: { return tessControl; }
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { return tessEval; }
	default: {return def; }
	}
}

//TODO: Support HLSL
//TODO: Cache compiled shaders
VkPipelineShaderStageCreateInfo Pipeline::CompileShader(ShaderStage& shaderStage, String& code, const String& name, DescriptorSetLayoutCreation* setLayoutInfos)
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
	}

	TShader tShader(stage);
	C8* c = code.Data();
	tShader.setStrings(&c, 1);
	tShader.setEnvInput((EShSource)shader.language, stage, EShClientVulkan, 450);
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

	ParseSPIRV((U32*)shaderInfo.pCode, shaderInfo.codeSize, shaderStage, setLayoutInfos);

	VkPipelineShaderStageCreateInfo shaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	shaderStageInfo.pNext = nullptr;
	shaderStageInfo.flags = 0;
	shaderStageInfo.stage = shaderStage.stage;
	shaderStageInfo.pName = shaderStage.entry.Data();
	shaderStageInfo.pSpecializationInfo = &shaderStage.specializationInfo.specializationInfo;

	vkCreateShaderModule(Renderer::device, &shaderInfo, Renderer::allocationCallbacks, &shaderStageInfo.module);

	return shaderStageInfo;
}

bool Pipeline::ParseSPIRV(U32* code, U64 codeSize, ShaderStage& stage, DescriptorSetLayoutCreation* setLayoutInfos)
{
	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(codeSize, code, &module);
	if (result != SPV_REFLECT_RESULT_SUCCESS) { return false; }

	stage.entry = module.entry_point_name;

	if (module.shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
	{
		for (U32 i = 0; i < module.input_variable_count; ++i)
		{
			SpvReflectInterfaceVariable* var = module.input_variables[i];

			U32 location = var->location;
			
			VertexAttribute attribute{};
			attribute.binding = location; //TODO: Support one vertex buffer vs multiple
			attribute.location = location;
			attribute.offset = 0;
			attribute.count = 1;

			VertexStream stream{};
			stream.binding = location; //TODO: Support one vertex buffer vs multiple
			stream.inputRate = VERTEX_INPUT_RATE_VERTEX;

			if (var->type_description->op == SpvOpTypeVector)
			{
				SpvReflectNumericTraits traits = var->type_description->traits.numeric;

				attribute.count = traits.vector.component_count;
				stream.stride = traits.scalar.width * attribute.count;
				attribute.format = (VkFormat)var->format;
			}

			++shader.vertexAttributeCount;
			++shader.vertexStreamCount;
			shader.vertexAttributes[location] = attribute;
			shader.vertexStreams[location] = stream;
		}
	}

	shader.setCount = module.descriptor_set_count;

	for (U32 i = 0; i < shader.setCount; ++i)
	{
		SpvReflectDescriptorSet set = module.descriptor_sets[i];

		DescriptorSetLayoutCreation& setLayout = setLayoutInfos[set.set];
		setLayout.setIndex = set.set;
		setLayout.bindingCount = set.binding_count;

		for (U32 j = 0; j < set.binding_count; ++j)
		{
			SpvReflectDescriptorBinding* binding = set.bindings[j];

			if (set.set == 1 && (binding->binding == Resources::bindlessTextureBinding ||
				binding->binding == (Resources::bindlessTextureBinding + 1)))
			{
				useBindless = true; continue;
			}

			DescriptorBinding setBinding{};
			setBinding.binding = binding->binding;
			setBinding.count = binding->count;

			switch (binding->type_description->op)
			{
			case SpvOpTypeStruct: { setBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; } break;
			case SpvOpTypeSampledImage: { setBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; } break;
			}

			setLayout.bindings[binding->binding] = setBinding;
		}
	}

	if (module.shader_stage & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT)
	{
		for (U32 i = 0; i < module.output_variable_count; ++i)
		{
			SpvReflectInterfaceVariable* var = module.output_variables[i];

			U32 location = var->location;

			ShaderOutput output{};
			output.format = (VkFormat)var->format;

			shader.outputs[shader.outputCount++] = output;
		}
	}

	//TODO: Fragment Shader Outputs
	//TODO: Specialization Constants
	//TODO: Push Constants

	spvReflectDestroyShaderModule(&module);

	return true;
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

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.pSetLayouts = vkLayouts;
	pipelineLayoutInfo.setLayoutCount = descriptorSetCount + useBindless;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.pNext = nullptr;
	//TODO:
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout;
	VkValidate(vkCreatePipelineLayout(Renderer::device, &pipelineLayoutInfo, Renderer::allocationCallbacks, &pipelineLayout));

	layout = pipelineLayout;

	if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		pipelineInfo.pStages = shader.stageInfos;
		pipelineInfo.stageCount = shader.shaderCount;
		pipelineInfo.layout = pipelineLayout;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		VkVertexInputAttributeDescription vertexAttributes[8]{};
		if (shader.vertexAttributeCount)
		{
			for (U32 i = 0; i < shader.vertexAttributeCount; ++i)
			{
				const VertexAttribute& vertexAttribute = shader.vertexAttributes[i];
				vertexAttributes[i] = { vertexAttribute.location, vertexAttribute.binding, vertexAttribute.format, vertexAttribute.offset };
			}

			vertexInputInfo.vertexAttributeDescriptionCount = shader.vertexAttributeCount;
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes;
		}
		else
		{
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		}

		VkVertexInputBindingDescription vertexBindings[8]{};
		if (shader.vertexStreamCount)
		{
			vertexInputInfo.vertexBindingDescriptionCount = shader.vertexStreamCount;

			for (U32 i = 0; i < shader.vertexStreamCount; ++i)
			{
				const VertexStream& vertexStream = shader.vertexStreams[i];
				VkVertexInputRate vertexRate = vertexStream.inputRate == VERTEX_INPUT_RATE_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
				vertexBindings[i] = { vertexStream.binding, vertexStream.stride, vertexRate };
			}
			vertexInputInfo.pVertexBindingDescriptions = vertexBindings;
		}
		else
		{
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr;
		}

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.pNext = nullptr;
		inputAssembly.flags = 0;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		pipelineInfo.pInputAssemblyState = &inputAssembly;

		VkPipelineColorBlendAttachmentState colorBlendAttachment[MAX_IMAGE_OUTPUTS]{};

		if (blendStateCount)
		{
			for (U64 i = 0; i < blendStateCount; ++i)
			{
				const BlendState& blendState = blendStates[i];

				colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachment[i].blendEnable = blendState.blendEnabled ? VK_TRUE : VK_FALSE;
				colorBlendAttachment[i].srcColorBlendFactor = blendState.sourceColor;
				colorBlendAttachment[i].dstColorBlendFactor = blendState.destinationColor;
				colorBlendAttachment[i].colorBlendOp = blendState.colorOperation;

				if (blendState.separateBlend)
				{
					colorBlendAttachment[i].srcAlphaBlendFactor = blendState.sourceAlpha;
					colorBlendAttachment[i].dstAlphaBlendFactor = blendState.destinationAlpha;
					colorBlendAttachment[i].alphaBlendOp = blendState.alphaOperation;
				}
				else
				{
					colorBlendAttachment[i].srcAlphaBlendFactor = blendState.sourceColor;
					colorBlendAttachment[i].dstAlphaBlendFactor = blendState.destinationColor;
					colorBlendAttachment[i].alphaBlendOp = blendState.colorOperation;
				}
			}
		}
		else
		{
			colorBlendAttachment[0].blendEnable = VK_FALSE;
			colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = blendStateCount ? blendStateCount : 1;
		colorBlending.pAttachments = colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		pipelineInfo.pColorBlendState = &colorBlending;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		depthStencilInfo.depthWriteEnable = depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.stencilTestEnable = depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthTestEnable = depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthCompareOp = depthStencil.depthComparison;
		if (depthStencil.stencilEnable)
		{
			// TODO: Support stencil buffers
			Logger::Error("Stencil Buffers Not Yet Supported!");
		}

		pipelineInfo.pDepthStencilState = &depthStencilInfo;

		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		pipelineInfo.pMultisampleState = &multisampling;

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

		pipelineInfo.pRasterizationState = &rasterizer;

		pipelineInfo.pTessellationState = nullptr;

		VkViewport viewport{};
		VkRect2D scissor{};

		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		pipelineInfo.pViewportState = &viewportState;

		VkDynamicState dynamicStates[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = CountOf32(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;

		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.renderPass = renderpass->renderpass;

		//TODO: Setup dynamic rendering
		//VkPipelineRenderingCreateInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
		//renderingInfo.pNext = nullptr;
		//renderingInfo.viewMask = 0;
		//renderingInfo.colorAttachmentCount;
		//renderingInfo.pColorAttachmentFormats;
		//renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

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
		cache.Open(cachePath, FILE_OPEN_WRITE_SETTINGS);

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