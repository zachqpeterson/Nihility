#pragma once

#include "VulkanInclude.hpp"
#include "Resources/ResourceDefines.hpp"

#include "Containers/Vector.hpp"
#include "Math/Math.hpp"

struct VmaAllocation_T;

enum class NH_API BufferType
{
	Vertex,
	Index,
	Shader,
	Uniform,
	DrawIndirect
};

struct NH_API Buffer
{
public:
	bool Create(BufferType type, U64 size = 1024);
	void Destroy();

	bool UploadVertexData(const void* vertexData, U64 size, U64 offset, VkSemaphore waitSemaphore = nullptr);
	bool UploadIndexData(const void* indexData, U64 size, U64 offset);
	bool UploadShaderData(const void* shaderData, U64 size, U64 offset);
	bool UploadUniformData(const void* uniformData, U64 size, U64 offset);

	operator VkBuffer() const;

private:
	bool CheckForResize(U64 bufferSize);

	BufferType type;
	U64 bufferSize = 0;
	VkBuffer vkBuffer = VK_NULL_HANDLE;
	VmaAllocation_T* bufferAllocation = nullptr;
	VkBuffer vkBufferStaging = VK_NULL_HANDLE;
	VmaAllocation_T* stagingBufferAllocation = nullptr;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	friend class Renderer;
	friend class Resources;
};