#pragma once

#include "Defines.hpp"
#include "Core\Logger.hpp"
#include "Memory\Memory.hpp"

/*---------DEFINES---------*/

#if defined PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined PLATFORM_LINUX
#define VK_USE_PLATFORM_XCB_KHR
#elif defined PLATFORM_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined PLATFORM_APPLE
#define VK_USE_PLATFORM_METAL_EXT
#elif defined PLATFORM_IOS
#define VK_USE_PLATFORM_IOS_MVK
#endif

#include <vulkan\vulkan.h>

#define VK_ADDITIONAL_VALIDATION

/// <summary>
/// Gets the description of a VkResult
/// </summary>
/// <param name="result:">A VkResult</param>
/// <returns>The description of a VkResult</returns>
inline CSTR ResultString(VkResult result)
{
#if LOG_TRACE_ENABLED
	switch (result)
	{
	case VK_SUCCESS:
		return "VK_SUCCESS Command successfully completed";
	case VK_NOT_READY:
		return "VK_NOT_READY A fence or query has not yet completed";
	case VK_TIMEOUT:
		return "VK_TIMEOUT A wait operation has not completed in the specified time";
	case VK_EVENT_SET:
		return "VK_EVENT_SET An event is signaled";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET An event is unsignaled";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE A return array was too small for the result";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";
	case VK_THREAD_IDLE_KHR:
		return "VK_THREAD_IDLE_KHR A deferred operation is not complete but there is currently no work for this thread to do at the time of this call.";
	case VK_THREAD_DONE_KHR:
		return "VK_THREAD_DONE_KHR A deferred operation is not complete but there is no work remaining to assign to additional threads.";
	case VK_OPERATION_DEFERRED_KHR:
		return "VK_OPERATION_DEFERRED_KHR A deferred operation was requested and at least some of the work was deferred.";
	case VK_OPERATION_NOT_DEFERRED_KHR:
		return "VK_OPERATION_NOT_DEFERRED_KHR A deferred operation was requested and no operations were deferred.";
	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		return "VK_PIPELINE_COMPILE_REQUIRED_EXT A requested pipeline creation would have required compilation, but the application requested compilation to not be performed.";

	default:
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has failed.";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for implementation-specific reasons.";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST The logical or physical device has been lost. See Lost Device";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or could not be loaded.";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not supported.";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created.";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported on this device.";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to fragmentation of the pool's memory. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool allocation failure was due to fragmentation.";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again.";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile or link. More details are reported back to the application via VK_EXT_debug_report if enabled.";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. If the failure was definitely due to fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid handle of the specified type.";
	case VK_ERROR_FRAGMENTATION:
		return "VK_ERROR_FRAGMENTATION A descriptor pool creation has failed due to fragmentation.";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
		return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed because the requested address is not available.";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have exlusive full-screen access. This may occur due to implementation-dependent reasons, outside of the application's control.";
	case VK_ERROR_UNKNOWN:
		return "VK_ERROR_UNKNOWN An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred.";
	}

#else
	switch (result)
	{
	case VK_SUCCESS:                                    return "VK_SUCCESS";
	case VK_NOT_READY:                                  return "VK_NOT_READY";
	case VK_TIMEOUT:                                    return "VK_TIMEOUT";
	case VK_EVENT_SET:                                  return "VK_EVENT_SET";
	case VK_EVENT_RESET:                                return "VK_EVENT_RESET";
	case VK_INCOMPLETE:                                 return "VK_INCOMPLETE";
	case VK_SUBOPTIMAL_KHR:                             return "VK_SUBOPTIMAL_KHR";
	case VK_THREAD_IDLE_KHR:                            return "VK_THREAD_IDLE_KHR";
	case VK_THREAD_DONE_KHR:                            return "VK_THREAD_DONE_KHR";
	case VK_OPERATION_DEFERRED_KHR:                     return "VK_OPERATION_DEFERRED_KHR";
	case VK_OPERATION_NOT_DEFERRED_KHR:                 return "VK_OPERATION_NOT_DEFERRED_KHR";
	case VK_PIPELINE_COMPILE_REQUIRED_EXT:              return "VK_PIPELINE_COMPILE_REQUIRED_EXT";

	default:
	case VK_ERROR_OUT_OF_HOST_MEMORY:                   return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:                 return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:                return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:                          return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:                    return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:                    return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:                return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:                  return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:                  return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:                     return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:                 return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:                      return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_SURFACE_LOST_KHR:                     return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:             return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:                      return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:             return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_INVALID_SHADER_NV:                    return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_OUT_OF_POOL_MEMORY:                   return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:              return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_FRAGMENTATION:                        return "VK_ERROR_FRAGMENTATION";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:           return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:  return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_ERROR_UNKNOWN:                              return "VK_ERROR_UNKNOWN";
	}
#endif
}

/// <summary>
/// Decides if a VkResult is a success or not
/// </summary>
/// <param name="result:">A VkResult</param>
/// <returns>True if VkResult is a success, false otherwise</returns>
inline bool ResultSuccess(VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS:
	case VK_NOT_READY:
	case VK_TIMEOUT:
	case VK_EVENT_SET:
	case VK_EVENT_RESET:
	case VK_INCOMPLETE:
	case VK_SUBOPTIMAL_KHR:
	case VK_THREAD_IDLE_KHR:
	case VK_THREAD_DONE_KHR:
	case VK_OPERATION_DEFERRED_KHR:
	case VK_OPERATION_NOT_DEFERRED_KHR:
	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		return true;

	default:
	case VK_ERROR_OUT_OF_HOST_MEMORY:
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	case VK_ERROR_INITIALIZATION_FAILED:
	case VK_ERROR_DEVICE_LOST:
	case VK_ERROR_MEMORY_MAP_FAILED:
	case VK_ERROR_LAYER_NOT_PRESENT:
	case VK_ERROR_EXTENSION_NOT_PRESENT:
	case VK_ERROR_FEATURE_NOT_PRESENT:
	case VK_ERROR_INCOMPATIBLE_DRIVER:
	case VK_ERROR_TOO_MANY_OBJECTS:
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_SURFACE_LOST_KHR:
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
	case VK_ERROR_INVALID_SHADER_NV:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
	case VK_ERROR_FRAGMENTATION:
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
	case VK_ERROR_UNKNOWN:
		return false;
	}
}

/// <summary>
/// Evaluate an expression that returns VkResult and Logs an error if it wasn't successful
/// </summary>
#define VkValidate(expr)						\
{												\
    VkResult result = expr;						\
	if (!ResultSuccess(result))					\
	{											\
		Logger::Error(ResultString(result));	\
	}											\
}

/// <summary>
/// Evaluate an expression that returns VkResult and Logs a fatal if it wasn't successful
/// </summary>
#define VkValidateF(expr)						\
{												\
    VkResult result = expr;						\
	if (!ResultSuccess(result))					\
	{											\
		Logger::Fatal(ResultString(result));	\
	}											\
}

/// <summary>
/// Evaluate an expression that returns VkResult and Logs an error if it wasn't successful and returns false
/// </summary>
#define VkValidateR(expr)						\
{												\
    VkResult result = expr;						\
	if (!ResultSuccess(result))					\
	{											\
		Logger::Error(ResultString(result));	\
		return false;							\
	}											\
}


/// <summary>
/// Evaluate an expression that returns VkResult and Logs a fatal if it wasn't successful and returns false
/// </summary>
#define VkValidateFR(expr)						\
{												\
    VkResult result = expr;						\
	if (!ResultSuccess(result))					\
	{											\
		Logger::Fatal(ResultString(result));	\
		return false;							\
	}											\
}

struct VmaAllocation_T;
struct VmaAllocator_T;

/*---------CONSTANTS---------*/

NH_HEADER_STATIC constexpr U8	MAX_IMAGE_OUTPUTS = 8;			// Maximum number of images/render targets/fbo attachments usable
NH_HEADER_STATIC constexpr U8	MAX_DESCRIPTOR_SET_LAYOUTS = 8;	// Maximum number of layouts in the pipeline
NH_HEADER_STATIC constexpr U8	MAX_SHADER_STAGES = 5;			// Maximum simultaneous shader stages, applicable to all different type of pipelines
NH_HEADER_STATIC constexpr U8	MAX_DESCRIPTORS_PER_SET = 16;	// Maximum list elements for both descriptor set layout and descriptor sets
NH_HEADER_STATIC constexpr U8	MAX_SWAPCHAIN_IMAGES = 3;		// Maximum images a swapchain can support
NH_HEADER_STATIC constexpr U8	MAX_VERTEX_STREAMS = 16;
NH_HEADER_STATIC constexpr U8	MAX_VERTEX_ATTRIBUTES = 16;
NH_HEADER_STATIC constexpr U32	INVALID_INDEX = U32_MAX;		// Invalid index to a resource

typedef U32 ResourceHandle;

//TODO: Operators
struct BufferHandle { ResourceHandle index; };
struct TextureHandle { ResourceHandle index; };
struct ShaderStateHandle { ResourceHandle index; };
struct SamplerHandle { ResourceHandle index; };
struct DescriptorSetLayoutHandle { ResourceHandle index; };
struct DescriptorSetHandle { ResourceHandle index; };
struct PipelineHandle { ResourceHandle index; };
struct RenderPassHandle { ResourceHandle index; };

NH_HEADER_STATIC constexpr BufferHandle					INVALID_BUFFER{ INVALID_INDEX };
NH_HEADER_STATIC constexpr TextureHandle				INVALID_TEXTURE{ INVALID_INDEX };
NH_HEADER_STATIC constexpr ShaderStateHandle			INVALID_SHADER{ INVALID_INDEX };
NH_HEADER_STATIC constexpr SamplerHandle				INVALID_SAMPLER{ INVALID_INDEX };
NH_HEADER_STATIC constexpr DescriptorSetLayoutHandle	INVALID_LAYOUT{ INVALID_INDEX };
NH_HEADER_STATIC constexpr DescriptorSetHandle			INVALID_SET{ INVALID_INDEX };
NH_HEADER_STATIC constexpr PipelineHandle				INVALID_PIPELINE{ INVALID_INDEX };
NH_HEADER_STATIC constexpr RenderPassHandle				INVALID_PASS{ INVALID_INDEX };

/*---------ENUMS---------*/

#pragma region Enums
enum ColorWriteEnable
{
	COLOR_WRITE_ENABLE_RED,
	COLOR_WRITE_ENABLE_GREED,
	COLOR_WRITE_ENABLE_BLUE,
	COLOR_WRITE_ENABLE_ALPHA,
	COLOR_WRITE_ENABLE_ALL,

	COLOR_WRITE_ENABLE_CONUT
};

enum ColorWriteEnableMask
{
	COLOR_WRITE_ENABLE_RED_MASK = 1 << 0,
	COLOR_WRITE_ENABLE_GREED_MASK = 1 << 1,
	COLOR_WRITE_ENABLE_BLUE_MASK = 1 << 2,
	COLOR_WRITE_ENABLE_ALPHA_MASK = 1 << 3,

	COLOR_WRITE_ENABLE_ALL_MASK =
	COLOR_WRITE_ENABLE_RED_MASK |
	COLOR_WRITE_ENABLE_GREED_MASK |
	COLOR_WRITE_ENABLE_BLUE_MASK |
	COLOR_WRITE_ENABLE_ALPHA_MASK
};

enum CullMode
{
	CULL_MODE_NONE,
	CULL_MODE_FRONT,
	CULL_MODE_BACK,

	CULL_MODE_COUNT
};

enum CullModeMask
{
	CULL_MODE_NONE_MASK = 1 << 0,
	CULL_MODE_FRONT_MASK = 1 << 1,
	CULL_MODE_BACK_MASK = 1 << 2
};

enum DepthWrite
{
	DEPTH_WRITE_ZERO,
	DEPTH_WRITE_ALL,

	DEPTH_WRITE_COUNT
};

enum DepthWriteMask
{
	DEPTH_WRITE_ZERO_MASK = 1 << 0,
	DEPTH_WRITE_ALL_MASK = 1 << 1
};

enum FillMode
{
	FILL_MODE_WIREFRAME,
	FILL_MODE_SOLID,
	FILL_MODE_POINT,

	FILL_MODE_COUNT
};

enum FillModeMask
{
	FILL_MODE_WIREFRAME_MASK = 1 << 0,
	FILL_MODE_SOLID_MASK = 1 << 1,
	FILL_MODE_POINT_MASK = 1 << 2
};

enum StencilOperation
{
	STENCIL_OP_KEEP,
	STENCIL_OP_ZERO,
	STENCIL_OP_REPLACE,
	STENCIL_OP_INCR_CLAMP,
	STENCIL_OP_DECR_CLAMP,
	STENCIL_OP_INVERT,
	STENCIL_OP_INCR_WRAP,
	STENCIL_OP_DECR_WRAP,

	STENCIL_OP_COUNT
};

enum StencilOperationMask
{
	STENCIL_OP_KEEP_MASK = 1 << 0,
	STENCIL_OP_ZERO_MASK = 1 << 1,
	STENCIL_OP_REPLACE_MASK = 1 << 2,
	STENCIL_OP_INCR_CLAMP_MASK = 1 << 3,
	STENCIL_OP_DECR_CLAMP_MASK = 1 << 4,
	STENCIL_OP_INVERT_MASK = 1 << 5,
	STENCIL_OP_INCR_WRAP_MASK = 1 << 6,
	STENCIL_OP_DECR_WRAP_MASK = 1 << 7
};

enum TopologyType
{
	TOPOLOGY_TYPE_UNKNOWN,
	TOPOLOGY_TYPE_POINT,
	TOPOLOGY_TYPE_LINE,
	TOPOLOGY_TYPE_TRIANGLE,
	TOPOLOGY_TYPE_PATCH,

	TOPOLOGY_TYPE_COUNT
};

enum TopologyTypeMask {
	TOPOLOGY_TYPE_UNKNOWN_MASK = 1 << 0,
	TOPOLOGY_TYPE_POINT_MASK = 1 << 1,
	TOPOLOGY_TYPE_LINE_MASK = 1 << 2,
	TOPOLOGY_TYPE_TRIANGLE_MASK = 1 << 3,
	TOPOLOGY_TYPE_PATCH_MASK = 1 << 4
};

enum ResourceUsage
{
	RESOURCE_USAGE_IMMUTABLE,
	RESOURCE_USAGE_DYNAMIC,
	RESOURCE_USAGE_STREAM,

	RESOURCE_USAGE_COUNT
};

enum ResourceUsageMask
{
	RESOURCE_USAGE_IMMUTABLE_MASK = 1 << 0,
	RESOURCE_USAGE_DYNAMIC_MASK = 1 << 1,
	RESOURCE_USAGE_STREAM_MASK = 1 << 2
};

enum IndexType
{
	INDEX_TYPE_U16,
	INDEX_TYPE_U32,

	INDEX_TYPE_COUNT
};

enum IndexTypeMask
{
	INDEX_TYPE_U16_MASK = 1 << 0,
	INDEX_TYPE_MASK = 1 << 1
};

enum TextureType
{
	TEXTURE_TYPE_1D,
	TEXTURE_TYPE_2D,
	TEXTURE_TYPE_3D,
	TEXTURE_TYPE_1D_ARRAY,
	TEXTURE_TYPE_2D_ARRAY,
	TEXTURE_TYPE_3D_ARRAY,

	TEXTURE_TYPE_COUNT
};

enum VertexComponentFormat
{
	VERTEX_COMPONENT_FLOAT,
	VERTEX_COMPONENT_FLOAT2,
	VERTEX_COMPONENT_FLOAT3,
	VERTEX_COMPONENT_FLOAT4,
	VERTEX_COMPONENT_MAT4,
	VERTEX_COMPONENT_BYTE,
	VERTEX_COMPONENT_BYTE4N,
	VERTEX_COMPONENT_UBYTE,
	VERTEX_COMPONENT_UBYTE4N,
	VERTEX_COMPONENT_SHORT2,
	VERTEX_COMPONENT_SHORT2N,
	VERTEX_COMPONENT_SHORT4,
	VERTEX_COMPONENT_SHORT4N,
	VERTEX_COMPONENT_UINT,
	VERTEX_COMPONENT_UINT2,
	VERTEX_COMPONENT_UINT4,

	VERTEX_COMPONENT_COUNT
};

enum VertexInputRate
{
	VERTEX_INPUT_RATE_VERTEX,
	VERTEX_INPUT_RATE_INSTANCE,

	VERTEX_INPUT_RATE_COUNT
};

enum VertexInputRateMask
{
	VERTEX_INPUT_RATE_VERTEX_MASK = 1 << 0,
	VERTEX_INPUT_RATE_INSTANCE_MASK = 1 << 1
};

enum LogicOperation
{
	LOGIC_OP_CLEAR,
	LOGIC_OP_SET,
	LOGIC_OP_COPY,
	LOGIC_OP_COPY_INVERTED,
	LOGIC_OP_NO_OP,
	LOGIC_OP_INVERT,
	LOGIC_OP_AND,
	LOGIC_OP_NAND,
	LOGIC_OP_OR,
	LOGIC_OP_NOR,
	LOGIC_OP_XOR,
	LOGIC_OP_EQUIVALENT,
	LOGIC_OP_AND_REVERSE,
	LOGIC_OP_AND_INVERTED,
	LOGIC_OP_OR_REVERSE,
	LOGIC_OP_OR_INVERTED,

	LOGIC_OP_COUNT
};

enum LogicOperationMask
{
	LOGIC_OP_CLEAR_MASK = 1 << 0,
	LOGIC_OP_SET_MASK = 1 << 1,
	LOGIC_OP_COPY_MASK = 1 << 2,
	LOGIC_OP_COPY_INVERTED_MASK = 1 << 3,
	LOGIC_OP_NO_OP_MASK = 1 << 4,
	LOGIC_OP_INVERT_MASK = 1 << 5,
	LOGIC_OP_AND_MASK = 1 << 6,
	LOGIC_OP_NAND_MASK = 1 << 7,
	LOGIC_OP_OR_MASK = 1 << 8,
	LOGIC_OP_NOR_MASK = 1 << 9,
	LOGIC_OP_XOR_MASK = 1 << 10,
	LOGIC_OP_EQUIVALENT_MASK = 1 << 11,
	LOGIC_OP_AND_REVERSE_MASK = 1 << 12,
	LOGIC_OP_AND_INVERTED_MASK = 1 << 13,
	LOGIC_OP_OR_REVERSE_MASK = 1 << 14,
	LOGIC_OP_OR_INVERTED_MASK = 1 << 15
};

enum QueueType
{
	QUEUE_TYPE_GRAPHICS,
	QUEUE_TYPE_COMPUTE,
	QUEUE_TYPE_COPY_TRANSFER,

	QUEUE_TYPE_COUNT
};

enum QueueTypeMask
{
	QUEUE_TYPE_GRAPHICS_MASK = 1 << 0,
	QUEUE_TYPE_COMPUTE_MASK = 1 << 1,
	QUEUE_TYPE_COPY_TRANSFER_MASK = 1 << 2
};

enum CommandType
{
	COMMAND_TYPE_BIND_PIPELINE,
	COMMAND_TYPE_BIND_RESOURCE_TABLE,
	COMMAND_TYPE_BIND_VERTEX_BUFFER,
	COMMAND_TYPE_BIND_INDEX_BUFFER,
	COMMAND_TYPE_BIND_RESOURCE_SET,
	COMMAND_TYPE_DRAW,
	COMMAND_TYPE_DRAWINDEXED,
	COMMAND_TYPE_DRAWINSTANCED,
	COMMAND_TYPE_DRAWINDEXEDINSTANCED,
	COMMAND_TYPE_DISPATCH,
	COMMAND_TYPE_COPY_RESOURCE,
	COMMAND_TYPE_SET_SCISSOR,
	COMMAND_TYPE_SET_VIEWPORT,
	COMMAND_TYPE_CLEAR,
	COMMAND_TYPE_CLEAR_DEPTH,
	COMMAND_TYPE_CLEAR_STENCIL,
	COMMAND_TYPE_BEGIN_PASS,
	COMMAND_TYPE_END_PASS,

	COMMAND_TYPE_COUNT
};

enum TextureFlag
{
	TEXTURE_FLAG_DEFAULT,
	TEXTURE_FLAG_RENDER_TARGET,
	TEXTURE_FLAG_COMPUTE,

	TEXTURE_FLAG_COUNT
};

enum TextureFlagMask
{
	TEXTURE_FLAG_DEFAULT_MASK = 1 << 0,
	TEXTURE_FLAG_RENDER_TARGET_MASK = 1 << 1,
	TEXTURE_FLAG_COMPUTE_MASK = 1 << 2
};

enum PipelineStage
{
	PIPELINE_STAGE_DRAW_INDIRECT,
	PIPELINE_STAGE_VERTEX_INPUT,
	PIPELINE_STAGE_VERTEX_SHADER,
	PIPELINE_STAGE_FRAGMENT_SHADER,
	PIPELINE_STAGE_RENDER_TARGET,
	PIPELINE_STAGE_COMPUTE_SHADER,
	PIPELINE_STAGE_TRANSFER,

	PIPELINE_STAGE_COUNT
};

enum PipelineStageMask
{
	PIPELINE_STAGE_DRAW_INDIRECT_MASK = 1 << 0,
	PIPELINE_STAGE_VERTEX_INPUT_MASK = 1 << 1,
	PIPELINE_STAGE_VERTEX_SHADER_MASK = 1 << 2,
	PIPELINE_STAGE_FRAGMENT_SHADER_MASK = 1 << 3,
	PIPELINE_STAGE_RENDER_TARGET_MASK = 1 << 4,
	PIPELINE_STAGE_COMPUTE_SHADER_MASK = 1 << 5,
	PIPELINE_STAGE_TRANSFER_MASK = 1 << 6
};

enum RenderPassType
{
	RENDER_PASS_TYPE_GEOMETRY,
	RENDER_PASS_TYPE_SWAPCHAIN,
	RENDER_PASS_TYPE_COMPUTE,

	RENDER_PASS_TYPE_COUNT
};

enum ResourceDeleteType
{
	RESOURCE_DELETE_TYPE_BUFFER,
	RESOURCE_DELETE_TYPE_TEXTURE,
	RESOURCE_DELETE_TYPE_PIPELINE,
	RESOURCE_DELETE_TYPE_SAMPLER,
	RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT,
	RESOURCE_DELETE_TYPE_DESCRIPTOR_SET,
	RESOURCE_DELETE_TYPE_RENDER_PASS,
	RESOURCE_DELETE_TYPE_SHADER_STATE,

	RESOURCE_DELETE_TYPE_COUNT
};

enum RenderPassOperation
{
	RENDER_PASS_OP_DONT_CARE,
	RENDER_PASS_OP_LOAD,
	RENDER_PASS_OP_CLEAR,

	RENDER_PASS_OP_COUNT
};

enum ResourceType {
	RESOURCE_TYPE_UNDEFINED = 0,
	RESOURCE_TYPE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
	RESOURCE_TYPE_INDEX_BUFFER = 0x2,
	RESOURCE_TYPE_RENDER_TARGET = 0x4,
	RESOURCE_TYPE_UNORDERED_ACCESS = 0x8,
	RESOURCE_TYPE_DEPTH_WRITE = 0x10,
	RESOURCE_TYPE_DEPTH_READ = 0x20,
	RESOURCE_TYPE_NON_PIXEL_SHADER_RESOURCE = 0x40,
	RESOURCE_TYPE_PIXEL_SHADER_RESOURCE = 0x80,
	RESOURCE_TYPE_SHADER_RESOURCE = 0x40 | 0x80,
	RESOURCE_TYPE_STREAM_OUT = 0x100,
	RESOURCE_TYPE_INDIRECT_ARGUMENT = 0x200,
	RESOURCE_TYPE_COPY_DEST = 0x400,
	RESOURCE_TYPE_COPY_SOURCE = 0x800,
	RESOURCE_TYPE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
	RESOURCE_TYPE_PRESENT = 0x1000,
	RESOURCE_TYPE_COMMON = 0x2000,
	RESOURCE_TYPE_RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
	RESOURCE_TYPE_SHADING_RATE_SOURCE = 0x8000,
};
#pragma endregion

static inline CSTR ToCompilerExtension(VkShaderStageFlagBits value)
{
	switch (value)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { return "vert"; }
	case VK_SHADER_STAGE_FRAGMENT_BIT: { return "frag"; }
	case VK_SHADER_STAGE_COMPUTE_BIT: { return "comp"; }
	default: { return ""; }
	}
}

static inline CSTR ToStageDefines(VkShaderStageFlagBits value)
{
	switch (value)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: { return "VERTEX"; }
	case VK_SHADER_STAGE_FRAGMENT_BIT: { return "FRAGMENT"; }
	case VK_SHADER_STAGE_COMPUTE_BIT: { return "COMPUTE"; }
	default: {return ""; }
	}
}

static inline VkImageType ToVkImageType(TextureType type)
{
	static VkImageType vkTarget[TEXTURE_TYPE_COUNT] = { VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D, VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D };
	return vkTarget[type];
}

static inline VkImageViewType ToVkImageViewType(TextureType type)
{
	static VkImageViewType vkData[] = { VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_VIEW_TYPE_3D, VK_IMAGE_VIEW_TYPE_1D_ARRAY, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY };
	return vkData[type];
}

static inline VkFormat ToVkVertexFormat(VertexComponentFormat value)
{
	// Float, Float2, Float3, Float4, Mat4, Byte, Byte4N, UByte, UByte4N, Short2, Short2N, Short4, Short4N, Uint, Uint2, Uint4, Count
	static VkFormat vkVertexFormats[VERTEX_COMPONENT_COUNT] = { VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, /*MAT4 TODO*/ VK_FORMAT_R32G32B32A32_SFLOAT,
																		  VK_FORMAT_R8_SINT, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8_UINT, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SNORM,
																		  VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32A32_UINT };

	return vkVertexFormats[value];
}

static inline VkPipelineStageFlags ToVkPipelineStage(PipelineStage value)
{
	static VkPipelineStageFlags vkValues[] = { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };
	return vkValues[value];
}

/*---------STRUCTURES---------*/

struct Rect2D
{
	F32	x = 0.0f;
	F32	y = 0.0f;
	F32	width = 0.0f;
	F32	height = 0.0f;
};

struct Rect2DInt
{
	I16 x = 0;
	I16 y = 0;
	U16 width = 0;
	U16 height = 0;
};

struct Viewport
{
	Rect2DInt	rect;
	F32			min_depth = 0.0f;
	F32			max_depth = 0.0f;
};

struct ViewportState
{
	U32 num_viewports = 0;
	U32 num_scissors = 0;

	Viewport* viewport = nullptr;
	Rect2DInt* scissors = nullptr;
};

struct StencilOperationState
{
	VkStencilOp	fail = VK_STENCIL_OP_KEEP;
	VkStencilOp	pass = VK_STENCIL_OP_KEEP;
	VkStencilOp	depthFail = VK_STENCIL_OP_KEEP;
	VkCompareOp	compare = VK_COMPARE_OP_ALWAYS;
	U32			compareMask = 0xff;
	U32			writeMask = 0xff;
	U32			reference = 0xff;
};

struct VertexAttribute
{
	U16						location = 0;
	U16						binding = 0;
	U32						offset = 0;
	VertexComponentFormat	format = VERTEX_COMPONENT_COUNT;
};

struct VertexStream
{
	U16				binding = 0;
	U16				stride = 0;
	VertexInputRate	inputRate = VERTEX_INPUT_RATE_COUNT;
};

struct VertexInputCreation
{
	U32                             numVertexStreams = 0;
	U32                             numVertexAttributes = 0;

	VertexStream                    vertexStreams[MAX_VERTEX_STREAMS];
	VertexAttribute                 vertexAttributes[MAX_VERTEX_ATTRIBUTES];

	VertexInputCreation& Reset();
	VertexInputCreation& AddVertexStream(const VertexStream& stream);
	VertexInputCreation& AddVertexAttribute(const VertexAttribute& attribute);
};

struct DepthStencilCreation
{
	StencilOperationState	front;
	StencilOperationState	back;
	VkCompareOp				depthComparison = VK_COMPARE_OP_ALWAYS;

	U8						depthEnable : 1;
	U8						depthWriteEnable : 1;
	U8						stencilEnable : 1;
	U8						pad : 5;

	DepthStencilCreation() : depthEnable{ 0 }, depthWriteEnable{ 0 }, stencilEnable{ 0 } {}

	DepthStencilCreation& SetDepth(bool write, VkCompareOp comparison_test);
};

struct BlendState
{
	VkBlendFactor			sourceColor = VK_BLEND_FACTOR_ONE;
	VkBlendFactor			destinationColor = VK_BLEND_FACTOR_ONE;
	VkBlendOp				colorOperation = VK_BLEND_OP_ADD;

	VkBlendFactor			sourceAlpha = VK_BLEND_FACTOR_ONE;
	VkBlendFactor			destinationAlpha = VK_BLEND_FACTOR_ONE;
	VkBlendOp				alphaOperation = VK_BLEND_OP_ADD;

	ColorWriteEnableMask	colorWriteMask = COLOR_WRITE_ENABLE_ALL_MASK;

	U8						blendEnabled : 1;
	U8						separateBlend : 1;
	U8						pad : 6;

	BlendState() : blendEnabled{ 0 }, separateBlend{ 0 } {}

	BlendState& SetColor(VkBlendFactor sourceColor, VkBlendFactor destinationColor, VkBlendOp colorOperation);
	BlendState& SetAlpha(VkBlendFactor sourceColor, VkBlendFactor destinationColor, VkBlendOp colorOperation);
	BlendState& SetColorWriteMask(ColorWriteEnableMask value);
};

struct BlendStateCreation
{
	BlendState	blendStates[MAX_IMAGE_OUTPUTS];
	U32			activeStates = 0;

	BlendStateCreation& Reset();
	BlendState& AddBlendState();
};

struct RasterizationCreation
{
	VkCullModeFlagBits	cullMode = VK_CULL_MODE_NONE;
	VkFrontFace			front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	FillMode			fill = FILL_MODE_SOLID;
};

struct ShaderStage
{
	CSTR					code = nullptr;
	U32						codeSize = 0;
	VkShaderStageFlagBits	type = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct ShaderStateCreation
{
	ShaderStage	stages[MAX_SHADER_STAGES];

	CSTR		name = nullptr;

	U32			stagesCount = 0;
	U32			spvInput = 0;

	// Building helpers
	ShaderStateCreation& Reset();
	ShaderStateCreation& SetName(CSTR name);
	ShaderStateCreation& AddStage(CSTR code, U32 codeSize, VkShaderStageFlagBits type);
	ShaderStateCreation& SetSpvInput(bool value);
};

struct DescriptorSetLayoutCreation
{
	struct Binding
	{
		VkDescriptorType	type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		U16					start = 0;
		U16					count = 0;
		CSTR				name = nullptr;
	};

	Binding					bindings[MAX_DESCRIPTORS_PER_SET];
	U32						numBindings = 0;
	U32						setIndex = 0;

	CSTR					name = nullptr;

	DescriptorSetLayoutCreation& Reset();
	DescriptorSetLayoutCreation& AddBinding(const Binding& binding);
	DescriptorSetLayoutCreation& SetName(CSTR name);
	DescriptorSetLayoutCreation& SetSetIndex(U32 index);
};

struct DescriptorSetCreation
{
	ResourceHandle				resources[MAX_DESCRIPTORS_PER_SET];
	SamplerHandle				samplers[MAX_DESCRIPTORS_PER_SET];
	U16							bindings[MAX_DESCRIPTORS_PER_SET];

	DescriptorSetLayoutHandle	layout;
	U32							numResources = 0;

	CSTR						name = nullptr;

	DescriptorSetCreation& Eeset();
	DescriptorSetCreation& SetLayout(DescriptorSetLayoutHandle layout);
	DescriptorSetCreation& Texture(TextureHandle texture, U16 binding);
	DescriptorSetCreation& Buffer(BufferHandle buffer, U16 binding);
	DescriptorSetCreation& TextureSampler(TextureHandle texture, SamplerHandle sampler, U16 binding);   // TODO: separate samplers from textures
	DescriptorSetCreation& SetName(CSTR name);
};

struct BufferCreation
{
	VkBufferUsageFlags	typeFlags = 0;
	ResourceUsage		usage = RESOURCE_USAGE_IMMUTABLE;
	U32					size = 0;
	void* initialData = nullptr;

	CSTR				name = nullptr;

	BufferCreation& Reset();
	BufferCreation& Set(VkBufferUsageFlags flags, ResourceUsage usage, U32 size);
	BufferCreation& SetData(void* data);
	BufferCreation& SetName(CSTR name);
};

struct TextureCreation
{
	void* initialData = nullptr;
	U16			width = 1;
	U16			height = 1;
	U16			depth = 1;
	U8			mipmaps = 1;
	U8			flags = 0;    // TextureFlags bitmasks

	VkFormat	format = VK_FORMAT_UNDEFINED;
	TextureType	type = TEXTURE_TYPE_2D;

	CSTR		name = nullptr;

	TextureCreation& SetSize(U16 width, U16 height, U16 depth);
	TextureCreation& SetFlags(U8 mipmaps, U8 flags);
	TextureCreation& SetFormatType(VkFormat format, TextureType type);
	TextureCreation& SetName(CSTR name);
	TextureCreation& SetData(void* data);
};

struct RenderPassOutput
{
	VkFormat			colorFormats[MAX_IMAGE_OUTPUTS];
	VkFormat			depthStencilFormat;
	U32					numColorFormats;

	RenderPassOperation	colorOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation	depthOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation	stencilOperation = RENDER_PASS_OP_DONT_CARE;

	RenderPassOutput& Reset();
	RenderPassOutput& Color(VkFormat format);
	RenderPassOutput& Depth(VkFormat format);
	RenderPassOutput& SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);
};

struct RenderPass
{
	VkRenderPass		renderPass;
	VkFramebuffer		frameBuffer;

	RenderPassOutput	output;

	TextureHandle		outputTextures[MAX_IMAGE_OUTPUTS];
	TextureHandle		outputDepth;

	RenderPassType		type;

	F32					scaleX = 1.f;
	F32					scaleY = 1.f;
	U16					width = 0;
	U16					height = 0;
	U16					dispatchX = 0;
	U16					dispatchY = 0;
	U16					dispatchZ = 0;

	U8					resize = 0;
	U8					numRenderTargets = 0;

	CSTR				name = nullptr;
};

struct PipelineCreation
{
	RasterizationCreation		rasterization;
	DepthStencilCreation		depthStencil;
	BlendStateCreation			blendState;
	VertexInputCreation			vertexInput;
	ShaderStateCreation			shaders;

	RenderPassOutput			renderPass;
	DescriptorSetLayoutHandle	descriptorSetLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	const ViewportState* viewport = nullptr;

	U32							numActiveLayouts = 0;

	CSTR						name = nullptr;

	PipelineCreation& AddDescriptorSetLayout(DescriptorSetLayoutHandle handle);
	RenderPassOutput& RenderPassOutput();
};

struct SamplerCreation
{
	VkFilter				minFilter = VK_FILTER_NEAREST;
	VkFilter				magFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode		mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode	addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	CSTR					name = nullptr;

	SamplerCreation& SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip);
	SamplerCreation& SetSddressModeU(VkSamplerAddressMode u);
	SamplerCreation& SetSddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v);
	SamplerCreation& SetSddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
	SamplerCreation& SetName(CSTR name);
};

struct RenderPassCreation
{
	U16						numRenderTargets = 0;
	RenderPassType			type = RENDER_PASS_TYPE_GEOMETRY;

	TextureHandle			outputTextures[MAX_IMAGE_OUTPUTS];
	TextureHandle			depthStencilTexture;

	F32						scaleX = 1.f;
	F32						scaleY = 1.f;
	U8						resize = 1;

	RenderPassOperation		colorOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation		depthOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation		stencilOperation = RENDER_PASS_OP_DONT_CARE;

	CSTR					name = nullptr;

	RenderPassCreation& Reset();
	RenderPassCreation& AddRenderTexture(TextureHandle texture);
	RenderPassCreation& SetScaling(F32 scaleX, F32 scaleY, U8 resize);
	RenderPassCreation& SetDepthStencilTexture(TextureHandle texture);
	RenderPassCreation& SetName(CSTR name);
	RenderPassCreation& SetType(RenderPassType type);
	RenderPassCreation& SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);
};

struct ResourceUpdate
{
	ResourceDeleteType	type;
	ResourceHandle		handle;
	U32					currentFrame;
};

struct DescriptorSetUpdate
{
	DescriptorSetHandle	descriptorSet;
	U32					frameIssued = 0;
};

struct Buffer
{
	VkBuffer			buffer;
	VmaAllocation_T* allocation;
	VkDeviceMemory		deviceMemory;
	VkDeviceSize		deviceSize;

	VkBufferUsageFlags	typeFlags = 0;
	ResourceUsage		usage = RESOURCE_USAGE_IMMUTABLE;
	U32					size = 0;
	U32					globalOffset = 0;	// Offset into global constant, if dynamic

	BufferHandle		handle;
	BufferHandle		parentBuffer;

	CSTR				name = nullptr;
};

struct Sampler
{
	VkSampler				sampler;

	VkFilter				minFilter = VK_FILTER_NEAREST;
	VkFilter				magFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode		mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode	addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	CSTR					name = nullptr;
};

struct Texture
{
	VkImage				image;
	VkImageView			imageView;
	VkFormat			format;
	VkImageLayout		imageLayout;
	VmaAllocation_T* allocation;

	U16					width = 1;
	U16					height = 1;
	U16					depth = 1;
	U8					mipmaps = 1;
	U8					flags = 0;

	TextureHandle		handle;
	TextureType			type = TEXTURE_TYPE_2D;

	Sampler* sampler = nullptr;

	CSTR				name = nullptr;
};

struct ShaderState
{
	VkPipelineShaderStageCreateInfo	shaderStageInfos[MAX_SHADER_STAGES];

	U32								activeShaders = 0;
	bool							graphicsPipeline = false;

	CSTR							name = nullptr;
};

struct DescriptorBinding
{
	VkDescriptorType	type;
	U16					start = 0;
	U16					count = 0;
	U16					set = 0;

	CSTR				name = nullptr;
};

struct DesciptorSetLayout
{
	VkDescriptorSetLayout			descriptorSetLayout;

	VkDescriptorSetLayoutBinding* binding = nullptr;
	DescriptorBinding* bindings = nullptr;
	U16								numBindings = 0;
	U16								setIndex = 0;

	DescriptorSetLayoutHandle		handle;
};

struct DesciptorSet
{
	VkDescriptorSet				descriptorSet;

	ResourceHandle* resources = nullptr;
	SamplerHandle* samplers = nullptr;
	U16* bindings = nullptr;

	const DesciptorSetLayout* layout = nullptr;
	U32							numResources = 0;
};

struct Pipeline
{
	VkPipeline					pipeline;
	VkPipelineLayout			pipelineLayout;

	VkPipelineBindPoint			bindPoint;

	ShaderStateHandle			shaderState;

	const DesciptorSetLayout* descriptorSetLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	DescriptorSetLayoutHandle	descriptorSetLayoutHandles[MAX_DESCRIPTOR_SET_LAYOUTS];
	U32							numActiveLayouts = 0;

	DepthStencilCreation		depthStencil;
	BlendStateCreation			blendState;
	RasterizationCreation		rasterization;

	PipelineHandle				handle;
	bool						graphicsPipeline = true;
};

struct GPUTimestamp
{
	U32		start;
	U32		end;

	F64		elapsedMs;

	U16		parentIndex;
	U16		depth;

	U32		color;
	U32		frameIndex;

	CSTR	name;
};

struct GPUTimestampManager
{
	void Create(U16 queriesPerFrame, U16 maxFrames);
	void Destroy();

	bool HasValidQueries() const;
	void Reset();
	U32 Resolve(U32 currentFrame, GPUTimestamp* timestampsToFill);    // Returns the total queries for this frame.

	U32 Push(U32 currentFrame, const char* name);    // Returns the timestamp query index.
	U32 Pop(U32 currentFrame);

	GPUTimestamp* timestamps = nullptr;
	U64* timestampsData = nullptr;

	U32				queriesPerFrame = 0;
	U32				currentQuery = 0;
	U32				parentIndex = 0;
	U32				depth = 0;

	bool			currentFrameResolved = false;    // Used to query the GPU only once per frame if get_gpu_timestamps is called more than once per frame.
};

struct MapBufferParameters
{
	BufferHandle	buffer;
	U32				offset = 0;
	U32				size = 0;
};

struct TextureBarrier
{
	TextureHandle texture;
};

struct BufferBarrier
{
	BufferHandle buffer;
};

struct ExecutionBarrier
{
	PipelineStage	sourcePipelineStage;
	PipelineStage	destinationPipelineStage;

	U32				newBarrierExperimental = U32_MAX;
	U32				loadOperation = 0;

	U32				numTextureBarriers;
	U32				numBufferBarriers;

	TextureBarrier	textureBarriers[8];
	BufferBarrier	bufferBarriers[8];

	ExecutionBarrier& Reset();
	ExecutionBarrier& Set(PipelineStage source, PipelineStage destination);
	ExecutionBarrier& AddImageBarrier(const TextureBarrier& textureBarrier);
	ExecutionBarrier& AddMemoryBarrier(const BufferBarrier& bufferBarrier);
};

template<class Type, U64 Count>
struct ResourcePool
{
	static constexpr U64 ResourceSize = sizeof(Type);
	static constexpr U64 ResourceCount = Count;
	static constexpr U64 MemorySize = ResourceCount * (ResourceSize + sizeof(U32));

	void Create();
	void Destroy();

	U32 ObtainResource();
	void ReleaseResource(U32 index);
	void FreeResources();

	Type* GetResource(U32 index);
	const Type* GetResource(U32 index) const;

	Type* memory{ nullptr };
	U32* freeIndices{ nullptr };
	U32 lastFree{ 0 };
};

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::Create()
{
	Memory::AllocateSize(&memory, MemorySize);
	freeIndices = (U32*)(memory + ResourceCount * ResourceSize);

	for (U32 i = 0; i < ResourceCount; ++i) { freeIndices[i] = i; }
}

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::Destroy()
{
	Memory::FreeSize(&memory);
}

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::FreeResources()
{
	lastFree = 0;

	for (uint32_t i = 0; i < ResourceCount; ++i) { freeIndices[i] = i; }
}

template<class Type, U64 Count>
inline U32 ResourcePool<Type, Count>::ObtainResource()
{
	// TODO: add bits for checking if resource is alive and use bitmasks.
	if (lastFree < ResourceCount) { return freeIndices[lastFree++]; }

	Logger::Error("No Free Resources Left!");
	return INVALID_INDEX;
}

template<class Type, U64 Count>
inline void ResourcePool<Type, Count>::ReleaseResource(U32 handle)
{
	// TODO: add bits for checking if resource is alive and use bitmasks.
	freeIndices[--lastFree] = handle;
}

template<class Type, U64 Count>
inline Type* ResourcePool<Type, Count>::GetResource(U32 handle)
{
	if (handle != INVALID_INDEX) { return &memory[handle * ResourceSize]; }

	return nullptr;
}

template<class Type, U64 Count>
inline const Type* ResourcePool<Type, Count>::GetResource(U32 handle) const
{
	if (handle != INVALID_INDEX) { return &memory[handle * ResourceSize]; }

	return nullptr;
}