#pragma once

#include "Defines.hpp"
#include "Core\Logger.hpp"
#include "Memory\Memory.hpp"
#include "Math\Math.hpp"

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

#include "External\LunarG\Vulkan\vulkan.h"

//#define VK_ADDITIONAL_VALIDATION

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
struct Renderpass;
struct Texture;

/*---------CONSTANTS---------*/

static constexpr U8	MAX_IMAGE_OUTPUTS = 8;				// Maximum number of images/render targets/fbo attachments usable
static constexpr U8	MAX_DESCRIPTOR_SET_LAYOUTS = 8;		// Maximum number of layouts in the pipeline
static constexpr U8	MAX_DESCRIPTORS_PER_SET = 16;		// Maximum list elements for both descriptor set layout and descriptor sets
static constexpr U8 MAX_DESCRIPTOR_SETS = 8;
static constexpr U8 MAX_PIPELINE_CONNECTIONS = 16;
static constexpr U8 MAX_SPECIALIZATION_CONSTANTS = 8;	// Maximum specialization contants a shader can have
static constexpr U8	MAX_SHADER_STAGES = 5;				// Maximum simultaneous shader stages, applicable to all different type of pipelines
static constexpr U8	MAX_VERTEX_STREAMS = 16;			// Maximum vertex streams a shader can have
static constexpr U8	MAX_VERTEX_ATTRIBUTES = 16;			// Maximum vertex attributes a shader can have
static constexpr U8	MAX_VERTEX_BUFFERS = 8;
static constexpr U8	MAX_PROGRAM_PASSES = 8;				// Maximum number of passes in a program pass group
static constexpr U8	MAX_SWAPCHAIN_IMAGES = 3;			// Maximum images a swapchain can support
static constexpr U8	MAX_VIEWPORTS = 8;					// Maximum viewports a renderpass can have
static constexpr U8	MAX_BLOOM_PASSES = 8;				// Maximum renderpasses to calculate bloom
static constexpr U8	MAX_SCISSORS = 8;					// Maximum scissors a renderpass can have

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
	VkViewport	viewports[MAX_VIEWPORTS];
	VkRect2D	scissors[MAX_SCISSORS];
	U8			viewportCount{ 0 };
	U8			scissorCount{ 0 };
};

struct Descriptor
{
	union
	{
		VkDescriptorBufferInfo bufferInfo{};
		VkDescriptorImageInfo imageInfo;
	};

	Descriptor();
	Descriptor(VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE);
	Descriptor(VkImageView imageView, VkImageLayout imageLayout, VkSampler sampler = nullptr);
	Descriptor(Texture* texture);
};

struct DescriptorSetLayout
{
	U64								handle{ U64_MAX };

	VkDescriptorSetLayout			descriptorSetLayout{ nullptr };
	VkDescriptorUpdateTemplate		updateTemplate{ nullptr };

	VkDescriptorSetLayoutBinding	bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8								bindingCount{ 0 };
	U8								setIndex{ 0 };
};

struct DescriptorSetLayoutInfo
{
	VkDescriptorSetLayoutBinding	bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8								bindingCount{ 0 };
	U8								setIndex{ 0 };
};

struct DescriptorSet
{
	U64						handle{ U64_MAX };

	VkDescriptorSet			descriptorSet{ nullptr };
	DescriptorSetLayout*	layout{ nullptr };
};

struct NH_API Camera
{
	void Reset();
	void SetOrthograpic(F32 nearPlane, F32 farPlane, F32 viewportWidth, F32 viewportHeight, F32 zoom);
	void SetPerspective(F32 nearPlane, F32 farPlane, F32 fov, F32 aspectRatio);
	void SetAspectRatio(F32 aspectRatio);

	const F32& Near() const;
	const F32& Far() const;
	const Matrix4& Projection() const;
	const Matrix4& ViewProjection() const;
	Vector4 Eye() const;
	const Vector3& Position() const;
	Quaternion3 Rotation() const;
	Vector3 Euler() const;

	bool Perspective() const;

	void SetPosition(const Vector3& position);
	void SetRotation(const Quaternion3& rotation);
	void SetRotation(const Vector3& rotation);

	void Update();

private:
	F32		mouseSensitivity{ 1.0f };
	F32		movementDelta{ 0.1f };
	U32		ignoreDraggingFrames{ 3 };

	Vector3	targetMovement{ Vector3::Zero };

	bool	mouseDragging{ false };

	F32		rotationSpeed{ 20.0f };
	F32		movementSpeed{ 5.0f };

	Matrix4	view{ Matrix4::Identity };
	Matrix4	projection{ Matrix4::Identity };
	Matrix4	viewProjection{ Matrix4::Identity };

	Vector3	position{ Vector3::Zero };
	Vector3	right{ Vector3::Right };
	Vector3	forward{ Vector3::Forward };
	Vector3	up{ Vector3::Up };

	F32		yaw{ 0.0f };
	F32		pitch{ 0.0f };
	F32		targetYaw{ 0.0f };
	F32		targetPitch{ 0.0f };

	F32		nearPlane{ 0.0f };
	F32		farPlane{ 0.0f };

	F32		fov{ 0.0f };
	F32		aspectRatio{ 0.0f };

	F32		zoom{ 0.0f };
	F32		viewportWidth{ 0.0f };
	F32		viewportHeight{ 0.0f };

	bool	perspective{ false };
	bool	updateProjection{ false };

	friend class Resources;
};

struct CommandBuffer;
struct Renderpass;
struct Pipeline;

struct Pass
{
	Renderpass* renderpass;
	Vector<Pipeline*> pipelines;
};

struct RenderGraph
{
	void AddPipeline(Pipeline* pipeline);

private:
	void Run(CommandBuffer* commandBuffer);

	Vector<Pass> passes;

	friend class Renderer;
};