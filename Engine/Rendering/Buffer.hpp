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
	Uniform
};

struct NH_API Buffer
{
private:
	bool Create(BufferType type, U64 size = 1024);
	void Destroy();

	bool UploadVertexData(const MeshData& vertexData);
	bool UploadVertexData(const Vector<Vector3>& vertexData);
	bool UploadIndexData(const MeshData& indexData);
	bool UploadShaderData(const Vector<Matrix4>& bufferData);
	bool UploadUniformData(const MatrixData& matrixData);

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