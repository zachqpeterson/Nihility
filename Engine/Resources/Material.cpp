#include "Material.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

bool Material::Create(const PipelineLayout& pipelineLayout, const Pipeline& pipeline, const Vector<VkDescriptorSet>& descriptorSets, const Vector<PushConstant>& pushConstants)
{
	this->pipelineLayout = pipelineLayout;
	this->pipeline = pipeline;
	this->sets = descriptorSets;
	this->pushConstants = pushConstants;

	if (pipeline.VertexSize())
	{
		vertexBuffer.Create(BufferType::Vertex, pipeline.VertexSize() * 4);
		indexBuffer.Create(BufferType::Index, sizeof(U32) * 6);
	}

	if (pipeline.InstanceSize())
	{
		for (U32 i = 0; i < Renderer::swapchain.ImageCount(); ++i)
		{
			instanceBuffers[i].Create(BufferType::Vertex, pipeline.InstanceSize() * 10000);
		}
	}

	if (pipeline.VertexSize() && pipeline.InstanceSize()) { vertexUsage = VertexUsage::VerticesAndInstances; }
	else if (pipeline.VertexSize()) { vertexUsage = VertexUsage::Vertices; }
	else if (pipeline.InstanceSize()) { vertexUsage = VertexUsage::Instances; }
	else { vertexUsage = VertexUsage::None; }

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

void Material::Bind(CommandBuffer commandBuffer) const
{
	if ((vertexUsage == VertexUsage::VerticesAndInstances || vertexUsage == VertexUsage::Instances) && 
		instanceBuffers[Renderer::frameIndex].Offset() == U64_MAX) { return; }

	commandBuffer.BindPipeline(pipeline);
	if (sets.Size()) { commandBuffer.BindDescriptorSets(BindPoint::Graphics, pipelineLayout, 0, (U32)sets.Size(), sets.Data()); }
	for (const PushConstant& pc : pushConstants)
	{
		commandBuffer.PushConstants(pipelineLayout, pc.stages, pc.offset, pc.size, pc.data);
	}

	switch (vertexUsage)
	{
	case VertexUsage::VerticesAndInstances: {
		VkBuffer vertexBuffers[] = { vertexBuffer, instanceBuffers[Renderer::frameIndex] };
		U64 offsets[] = { vertexBuffer.Offset(), instanceBuffers[Renderer::frameIndex].Offset() };
		commandBuffer.BindVertexBuffers(CountOf32(vertexBuffers), vertexBuffers, offsets);

		commandBuffer.BindIndexBuffer(indexBuffer, indexBuffer.Offset());

		commandBuffer.DrawIndexed(indexBuffer.Size() / sizeof(U32), instanceBuffers[Renderer::frameIndex].Size() / pipeline.InstanceSize(), 0, 0, 0);
	} break;
	case VertexUsage::Vertices: {
		VkBuffer vertexBuffers[] = { vertexBuffer };
		U64 offsets[] = { vertexBuffer.Offset() };
		commandBuffer.BindVertexBuffers(CountOf32(vertexBuffers), vertexBuffers, offsets);

		commandBuffer.BindIndexBuffer(indexBuffer, indexBuffer.Offset());

		commandBuffer.DrawIndexed(indexBuffer.Size() / sizeof(U32), 1, 0, 0, 0);
	} break;
	case VertexUsage::Instances: {
		VkBuffer vertexBuffers[] = { instanceBuffers[Renderer::frameIndex] };
		U64 offsets[] = { instanceBuffers[Renderer::frameIndex].Offset() };
		commandBuffer.BindVertexBuffers(CountOf32(vertexBuffers), vertexBuffers, offsets);

		commandBuffer.Draw(0, 3, 0, instanceBuffers[Renderer::frameIndex].Size() / pipeline.InstanceSize());
	} break;
	case VertexUsage::None: {
		commandBuffer.Draw(0, 3, 0, 1);
	} break;
	}
}

void Material::UploadVertices(void* data, U32 size, U32 offset)
{
	if (pipeline.VertexSize()) { vertexBuffer.UploadVertexData(data, size, offset); }
	else { Logger::Error("This Material Does Not Use Vertices!"); }
}

void Material::UploadInstances(void* data, U32 size, U32 offset)
{
	if (pipeline.InstanceSize()) { instanceBuffers[Renderer::frameIndex].UploadVertexData(data, size, offset); }
	else { Logger::Error("This Material Does Not Use Instances!"); }
}

void Material::UploadIndices(void* data, U32 size, U32 offset)
{
	if (pipeline.VertexSize()) { indexBuffer.UploadIndexData(data, size, offset); }
	else { Logger::Error("This Material Does Not Use Indices!"); }
}

const PipelineLayout& Material::GetPipelineLayout() const
{
	return pipelineLayout;
}