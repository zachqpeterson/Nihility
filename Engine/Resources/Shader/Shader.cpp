#include "Shader.hpp"

#include "Resources\Resources.hpp"

#include "Rendering\RenderingDefines.hpp"
#include "Rendering\Renderer.hpp"

//TODO: #define ENABLE_OPT for optimised shaders
#include "SPIRV\GlslangToSpv.h"
#include "glslang\Public\ShaderLang.h"
#include "spirv_cross\spirv_reflect.h"

#if defined(_MSC_VER)
#include "spirv-headers\spirv.h"
#else
#include "spirv_cross\spirv.h"
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

Descriptor::Descriptor(const ResourceRef<Texture>& texture)
{
	imageInfo.imageView = texture->imageView;
	imageInfo.imageLayout = texture->format == VK_FORMAT_D32_SFLOAT ? IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

	String name;
};

struct ShaderInfo
{
	VkPipelineShaderStageCreateInfo		stageInfo;

	VkVertexInputBindingDescription		vertexStreams[MAX_VERTEX_STREAMS];
	U32									vertexStreamCount = 0;

	VkVertexInputAttributeDescription	vertexAttributes[MAX_VERTEX_ATTRIBUTES];
	U32									vertexAttributeCount = 0;

	VkDescriptorSetLayoutBinding		bindings[MAX_DESCRIPTORS_PER_SET];
	U8									bindingCount = 0;
};

VkDescriptorSetLayout			Shader::dummyDescriptorSetLayout;

VkDescriptorPool				Shader::bindlessDescriptorPool;
VkDescriptorSet					Shader::bindlessDescriptorSet;
VkDescriptorSetLayout_T* Shader::bindlessDescriptorLayout;

bool Shader::Initialize()
{
	if (!glslang::InitializeProcess()) { return false; }

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

		VkValidateFR(vkCreateDescriptorSetLayout(Renderer::device, &layoutInfo, Renderer::allocationCallbacks, &bindlessDescriptorLayout));

		Renderer::SetResourceName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)bindlessDescriptorLayout, "bindless_dsl");

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = bindlessDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &bindlessDescriptorLayout;

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
		vkDestroyDescriptorSetLayout(Renderer::device, bindlessDescriptorLayout, Renderer::allocationCallbacks);
		vkDestroyDescriptorPool(Renderer::device, bindlessDescriptorPool, Renderer::allocationCallbacks);
	}
}

bool Shader::Create(const String& shaderPath)
{
	String data = Resources::LoadBinaryString(shaderPath);

	Memory::Allocate(&shaderInfo);

	I64 index = 0;

	while (data[index] != '#')
	{
		I64 value = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

		switch (Hash::StringHash(data.Data() + index, data.IndexOf('=', index) - index))
		{
		case "instanceLocation"_Hash: { instanceLocation = data.ToType<U8>(value); } break;
		}

		index = data.IndexOf('\n', value + 1) + 1;
	}

	String code = data.SubString(index);
	CompileShader(code);

	return true;
}

void Shader::Destroy()
{
	if (shaderInfo)
	{
		if (shaderInfo->stageInfo.module) { vkDestroyShaderModule(Renderer::device, shaderInfo->stageInfo.module, Renderer::allocationCallbacks); }

		shaderInfo->stageInfo.module = nullptr;

		Memory::Free(&shaderInfo);
	}

	entryPoint.Destroy();
	name.Destroy();
	handle = U64_MAX;
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
	default: { return def; }
	}
}

//TODO: Cache compiled shaders
void Shader::CompileShader(String& code)
{
	using namespace glslang;

	EShLanguage language = EShLangCount;

	switch (stage)
	{
	case SHADER_STAGE_VERTEX_BIT: { language = EShLangVertex; } break;
	case SHADER_STAGE_TESSELLATION_CONTROL_BIT: { language = EShLangTessControl; } break;
	case SHADER_STAGE_TESSELLATION_EVALUATION_BIT: { language = EShLangTessEvaluation; } break;
	case SHADER_STAGE_GEOMETRY_BIT: { language = EShLangGeometry; } break;
	case SHADER_STAGE_FRAGMENT_BIT: { language = EShLangFragment; } break;
	case SHADER_STAGE_COMPUTE_BIT: { language = EShLangCompute; } break;
	case SHADER_STAGE_TASK_BIT_EXT: { language = EShLangTask; } break;
	case SHADER_STAGE_MESH_BIT_EXT: { language = EShLangMesh; } break;
	}

	TShader tShader(language);
	C8* c = code.Data();
	tShader.setStrings(&c, 1);
	tShader.setEnvInput(EShSourceGlsl, language, EShClientVulkan, 450);
	tShader.setEnvClient(EShClientVulkan, EShTargetVulkan_1_3);
	tShader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_6);
	if (!tShader.parse(&resources, 450, false, EShMsgDefault)) { Logger::Error(tShader.getInfoLog()); }

	TProgram tProgram;
	tProgram.addShader(&tShader);
	if (!tProgram.link(EShMsgDefault)) { Logger::Error(tProgram.getInfoLog()); }

	std::vector<U32> spv;
	GlslangToSpv(*tProgram.getIntermediate(language), spv);

	ParseSPIRV(&spv[0], (U32)spv.size());
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

void Shader::ParseSPIRV(U32* code, U32 codeSize)
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
		case SpvOpName:
		{
			U32 id = it[1];
			ids[id].name = (C8*)(it + 2);
		} break;
		case SpvOpEntryPoint:
		{
			stage = (ShaderStageType)GetShaderStage((SpvExecutionModel)it[1]);
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

	U8 combinedBinding = U8_MAX;

	for (const Id& id : ids)
	{
		if (id.set == 1 && id.binding == 10) { useBindless = true; continue; }
		else if (id.set == 1) { Logger::Error("Multiple Descriptor Sets Aren't Supported!"); }

		if (id.opcode == SpvOpVariable)
		{
			switch (id.storageClass)
			{
			case SpvStorageClassInput: {
				if (stage == VK_SHADER_STAGE_VERTEX_BIT)
				{
					Id& type = ids[ids[id.typeId].typeId];

					if (type.opcode == SpvOpTypeMatrix)
					{
						if (id.location < instanceLocation) { Logger::Error("Matrix Vertex Attributes Must Be Instanced!"); }

						useInstancing = true;

						if (instanceBinding == U8_MAX)
						{
							instanceBinding = vertexBindingCount++;
							vertexTypes[instanceBinding] = VERTEX_TYPE_INSTANCE;
						}

						const Id& component = ids[type.typeId];

						switch (type.count)
						{
						case 2: {
							VkVertexInputAttributeDescription attribute{};
							attribute.location = id.location;
							attribute.binding = instanceBinding;
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
							attribute.binding = instanceBinding;
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
							attribute.binding = instanceBinding;
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
						attribute.format = GetFormat(ids, type);
						attribute.offset = 0;

						if (id.location >= instanceLocation)
						{
							useInstancing = true;

							if (instanceBinding == U8_MAX)
							{
								instanceBinding = vertexBindingCount++;
								vertexTypes[instanceBinding] = VERTEX_TYPE_INSTANCE;
							}

							attribute.binding = instanceBinding;
						}
						else
						{
							switch (Hash::StringHash(id.name.Data(), id.name.Size()))
							{
							case "gl_VertexIndex"_Hash: continue;
							case "position"_Hash: { attribute.binding = vertexBindingCount++; vertexTypes[attribute.binding] = VERTEX_TYPE_POSITION; } break;
							case "normal"_Hash: { attribute.binding = vertexBindingCount++; vertexTypes[attribute.binding] = VERTEX_TYPE_NORMAL; } break;
							case "tangent"_Hash: { attribute.binding = vertexBindingCount++; vertexTypes[attribute.binding] = VERTEX_TYPE_TANGENT; } break;
							case "texcoord"_Hash: { attribute.binding = vertexBindingCount++; vertexTypes[attribute.binding] = VERTEX_TYPE_TEXCOORD; } break;
							case "color"_Hash: { attribute.binding = vertexBindingCount++; vertexTypes[attribute.binding] = VERTEX_TYPE_COLOR; } break;
							default: { if (combinedBinding == U8_MAX) { combinedBinding = vertexBindingCount++; vertexTypes[combinedBinding] = VERTEX_TYPE_COMBINED; } attribute.binding = combinedBinding; } break;
							}

							useVertices = true;
						}

						shaderInfo->vertexAttributes[id.location] = attribute;

						++shaderInfo->vertexAttributeCount;
					}
				}
			} break;
			case SpvStorageClassPushConstant: { usePushConstants = true; } break;
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

				VkDescriptorSetLayoutBinding& binding = shaderInfo->bindings[id.binding];
				binding.descriptorType = descriptorType;
				binding.stageFlags = stage;
				binding.binding = id.binding;
				binding.descriptorCount = 1;

				shaderInfo->bindingCount = Math::Max(shaderInfo->bindingCount, (U8)(id.binding + 1));
			} break;
			case SpvStorageClassOutput: {
				if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) { ++outputCount; }
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

	if (stage == VK_SHADER_STAGE_VERTEX_BIT && shaderInfo->vertexAttributeCount)
	{
		U8 combinedStream = U8_MAX;

		VkVertexInputBindingDescription instanceInput{};
		instanceInput.binding = instanceBinding;
		instanceInput.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		VkVertexInputBindingDescription combinedInput{};
		combinedInput.binding = combinedBinding;
		combinedInput.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		for (U32 i = 0; i < shaderInfo->vertexAttributeCount; ++i)
		{
			VkVertexInputAttributeDescription& attribute = shaderInfo->vertexAttributes[i];

			if (attribute.binding == instanceBinding)
			{
				attribute.offset = instanceInput.stride;
				instanceInput.stride += FormatStride(attribute.format);
			}
			else if (attribute.binding == combinedBinding)
			{
				attribute.offset = combinedInput.stride;
				combinedInput.stride += FormatStride(attribute.format);
			}
			else
			{
				VkVertexInputBindingDescription binding{};
				binding.binding = attribute.binding;
				binding.stride = FormatStride(attribute.format);
				binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				shaderInfo->vertexStreams[shaderInfo->vertexStreamCount++] = binding;
			}
		}

		if (instanceBinding != U8_MAX)
		{
			shaderInfo->vertexStreams[shaderInfo->vertexStreamCount++] = instanceInput;
			instanceStride = instanceInput.stride;
		}

		if (combinedBinding != U8_MAX)
		{
			shaderInfo->vertexStreams[shaderInfo->vertexStreamCount++] = combinedInput;
		}
	}

	if (stage == VK_SHADER_STAGE_COMPUTE_BIT)
	{
		if (localSizeIdX >= 0) { localSizeX = ids[localSizeIdX].constant; }
		if (localSizeIdY >= 0) { localSizeY = ids[localSizeIdY].constant; }
		if (localSizeIdZ >= 0) { localSizeZ = ids[localSizeIdZ].constant; }
	}

	VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleInfo.codeSize = codeSize * sizeof(U32);
	moduleInfo.pCode = code;

	VkShaderModule module;
	vkCreateShaderModule(Renderer::device, &moduleInfo, Renderer::allocationCallbacks, &module);

	shaderInfo->stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo->stageInfo.pNext = nullptr;
	shaderInfo->stageInfo.flags = 0;
	shaderInfo->stageInfo.module = module;
	shaderInfo->stageInfo.stage = (VkShaderStageFlagBits)stage;
	shaderInfo->stageInfo.pName = entryPoint.Data();
}

VkPipelineShaderStageCreateInfo& Shader::GetShaderInfo()
{
	return shaderInfo->stageInfo;
}

void Shader::FillOutVertexInfo(VkPipelineVertexInputStateCreateInfo& info)
{
	info.vertexAttributeDescriptionCount = shaderInfo->vertexAttributeCount;
	info.pVertexAttributeDescriptions = shaderInfo->vertexAttributes;
	info.vertexBindingDescriptionCount = shaderInfo->vertexStreamCount;
	info.pVertexBindingDescriptions = shaderInfo->vertexStreams;
}

U8 Shader::GetBindingCount() const
{
	return shaderInfo->bindingCount;
}

VkDescriptorSetLayoutBinding Shader::GetBinding(U8 i) const
{
	return shaderInfo->bindings[i];
}