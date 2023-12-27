#include "Scene.hpp"

#include "Entity.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Rendering\RenderingDefines.hpp"

void Scene::Create(CameraType cameraType, Rendergraph* rendergraph)
{
	this->rendergraph = rendergraph;

	switch (cameraType)
	{
	case CAMERA_TYPE_PERSPECTIVE: {
		camera.SetPerspective(0.01f, 1000.0f, 45.0f, 1.7777778f);
#ifdef NH_DEBUG
		flyCamera.SetPerspective(0.01f, 1000.0f, 45.0f, 1.7777778f);
#endif
	} break;
	case CAMERA_TYPE_ORTHOGRAPHIC: {
		camera.SetOrthograpic(-100.0f, 100.0f, 240.0f, 135.0f, 1.0f);
#ifdef NH_DEBUG
		flyCamera.SetOrthograpic(-100.0f, 100.0f, 240.0f, 135.0f, 1.0f);
#endif
	} break;
	}

	vertexBuffer = Renderer::CreateBuffer(MEGABYTES(128), BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	instanceBuffer = Renderer::CreateBuffer(MEGABYTES(128), BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	indexBuffer = Renderer::CreateBuffer(MEGABYTES(128), BUFFER_USAGE_INDEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	drawBuffer = Renderer::CreateBuffer(MEGABYTES(32), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_INDIRECT_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	countsBuffer = Renderer::CreateBuffer(sizeof(U32) * 32, BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_INDIRECT_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);

	ForEach<RegisteredComponents>([this]<ComponentType Type>() { CreatePool<Type>(); });
}

void Scene::Destroy()
{
	name.Destroy();

	entities.Destroy();

	meshDraws.Destroy();
}

bool Scene::Render(CommandBuffer* commandBuffer)
{
	CameraData* cameraData = Renderer::GetCameraData();

#ifdef NH_DEBUG
	if (Settings::InEditor())
	{
		flyCamera.Update();
		cameraData->vp = flyCamera.ViewProjection();
		cameraData->eye = flyCamera.Eye();
	}
	else
#endif
	{
		camera.Update();
		cameraData->vp = camera.ViewProjection();
		cameraData->eye = camera.Eye();
	}

	rendergraph->Run(commandBuffer, vertexBuffer, instanceBuffer, indexBuffer, drawBuffer, countsBuffer);

	return true;
}

void Scene::Resize()
{
	rendergraph->Resize();
}

Entity* Scene::AddEntity()
{
	entities.Push({ this });

	return &entities.Back();
}

void Scene::AddMesh(MeshInstance& instance)
{
	if (loaded)
	{
		U64 handle = *handles.GetInsert(instance.mesh->handle, meshDraws.Size());

		if (handle >= meshDraws.Size()) { meshDraws.Push({}); }

		MeshDraw& draw = meshDraws[handle];

		RenderStageType type = instance.material->type;
		U32 index = instance.material->stageIndex;
		MeshLocation& location = draw.locations[type][index];
		U32 instanceSize = rendergraph->InstanceSize(type, index);
		U32 vertexSize = rendergraph->VertexSize(type, index);
		Pipeline* pipeline = rendergraph->GetPipeline(type, index);

		if (!pipeline->drawCount)
		{
			pipeline->drawOffset = drawOffset;
			drawOffset += sizeof(VkDrawIndexedIndirectCommand) * 128;
			pipeline->countOffset = countsOffset;
			countsOffset += sizeof(U32);
			pipeline->indexOffset = indexOffset;
			indexOffset += MEGABYTES(2);
			pipeline->vertexOffset = vertexOffset;
			vertexOffset += MEGABYTES(2);
			pipeline->instanceOffset = instanceOffset;
			instanceOffset += MEGABYTES(2);
		}

		if (!draw.indexCount)
		{
			//Indices
			VkBufferCopy region{};
			region.dstOffset = pipeline->indexOffset + pipeline->indexCount * sizeof(U32);
			region.size = instance.mesh->indicesSize;
			region.srcOffset = 0;

			Renderer::FillBuffer(indexBuffer, instance.mesh->indicesSize, instance.mesh->indices, 1, &region);

			draw.indexCount = instance.mesh->indicesSize / sizeof(U32);
			draw.indexOffset = pipeline->indexCount;
			pipeline->indexCount += draw.indexCount;

			//Vertices
			region.dstOffset = pipeline->vertexOffset + pipeline->vertexCount * vertexSize;
			region.size = instance.mesh->verticesSize;
			region.srcOffset = 0;

			Renderer::FillBuffer(vertexBuffer, instance.mesh->verticesSize, instance.mesh->vertices, 1, &region);

			draw.vertexOffset = pipeline->vertexCount;
			pipeline->vertexCount += instance.mesh->verticesSize / vertexSize;
		}

		if (!location.instanceCount)
		{
			location.drawOffset = pipeline->drawOffset + sizeof(VkDrawIndexedIndirectCommand) * pipeline->drawCount;
			location.instanceOffset = pipeline->instanceCount;
			pipeline->instanceCount += 1024;

			//Count
			VkBufferCopy region{};
			region.dstOffset = pipeline->countOffset;
			region.size = sizeof(U32);
			region.srcOffset = 0;

			++pipeline->drawCount;

			Renderer::FillBuffer(countsBuffer, sizeof(U32), &pipeline->drawCount, 1, &region);
		}

		//Draw
		VkDrawIndexedIndirectCommand drawCommand{};
		drawCommand.indexCount = draw.indexCount;
		drawCommand.instanceCount = location.instanceCount + 1;
		drawCommand.firstIndex = draw.indexOffset;
		drawCommand.vertexOffset = draw.vertexOffset;
		drawCommand.firstInstance = location.instanceOffset;

		VkBufferCopy region{};
		region.dstOffset = location.drawOffset;
		region.size = sizeof(VkDrawIndexedIndirectCommand);
		region.srcOffset = 0;

		Renderer::FillBuffer(drawBuffer, sizeof(VkDrawIndexedIndirectCommand), &drawCommand, 1, &region);

		//Instance
		region.dstOffset = pipeline->instanceOffset + (location.instanceOffset + location.instanceCount) * instanceSize;
		region.size = instanceSize;
		region.srcOffset = 0;

		Renderer::FillBuffer(instanceBuffer, instanceSize, &instance.instanceData, 1, &region);

		++location.instanceCount;
	}
}

void Scene::Load()
{
	loaded = true;

	for (ComponentPool* pool : componentPools)
	{
		pool->Load(this);
	}
}

void Scene::Unload()
{

}

void Scene::Update()
{
	for (ComponentPool* pool : componentPools)
	{
		pool->Update();
	}
}