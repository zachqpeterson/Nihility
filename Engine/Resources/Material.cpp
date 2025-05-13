#include "Material.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

bool Material::Create(const PipelineLayout& pipelineLayout, const Pipeline& pipeline, const Vector<VkDescriptorSet>& descriptorSets)
{
	this->sets = descriptorSets;
	this->pipelineLayout = pipelineLayout;
	this->pipeline = pipeline;

	vertexBuffer.Create(BufferType::Vertex, sizeof(SpriteVertex) * 4);
	indexBuffer.Create(BufferType::Index, sizeof(U32) * 6);

	for (U32 i = 0; i < Renderer::swapchain.ImageCount(); ++i)
	{
		instanceBuffers[i].Create(BufferType::Vertex, sizeof(SpriteInstance) * 10000);
	}

	SpriteVertex vertices[4] = {
	{ { -0.5f, -0.5f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f }, { 1.0f, 1.0f } }
	};

	U32 indices[6] = { 0, 1, 2, 2, 3, 0 };

	vertexBuffer.UploadVertexData(vertices, sizeof(SpriteVertex) * 4, 0);
	indexBuffer.UploadIndexData(indices, sizeof(U32) * 6, 0);

	return true;
}

void Material::Destroy()
{
	vertexBuffer.Destroy();
	indexBuffer.Destroy();

	for (U32 i = 0; i < Renderer::swapchain.ImageCount(); ++i)
	{
		instanceBuffers[i].Destroy();
	}

	pipeline.Destroy();
	pipelineLayout.Destroy();
}

void Material::Bind(CommandBuffer commandBuffer, U32 instanceCount) const
{
	commandBuffer.BindPipeline(pipeline);
	commandBuffer.BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, (U32)sets.Size(), sets.Data());
	commandBuffer.PushConstants(pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GlobalPushConstant), &Renderer::globalPushConstant);

	VkBuffer vertexBuffers[] = { vertexBuffer, instanceBuffers[Renderer::frameIndex] };
	U64 offsets[] = { 0, 0 };
	commandBuffer.BindVertexBuffers(2, vertexBuffers, offsets);
	commandBuffer.BindIndexBuffer(indexBuffer, 0);
	commandBuffer.DrawIndexed(6, instanceCount, 0, 0, 0);
}