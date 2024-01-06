#include "Scene.hpp"

#include "Entity.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Rendering\RenderingDefines.hpp"
#include "Core\Time.hpp"

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

	for (U32 i = 0; i < VERTEX_TYPE_COUNT - 1; ++i)
	{
		buffers.vertexBuffers.Push(Renderer::CreateBuffer(MEGABYTES(32), BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL));
	}

	buffers.instanceBuffer = Renderer::CreateBuffer(MEGABYTES(128), BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	buffers.indexBuffer = Renderer::CreateBuffer(MEGABYTES(128), BUFFER_USAGE_INDEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	buffers.drawBuffer = Renderer::CreateBuffer(MEGABYTES(32), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_INDIRECT_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	buffers.countsBuffer = Renderer::CreateBuffer(sizeof(U32) * 32, BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_INDIRECT_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);

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
	ShadowData* shadowData = Renderer::GetShadowData();
	CameraData* cameraData = Renderer::GetCameraData();
	SkyboxData* skyboxData = Renderer::GetSkyboxData();

	Vector3 lightPos;

	lightPos.x = Math::Cos(Time::AbsoluteTime() * 360.0f * DEG_TO_RAD_F) * 40.0f;
	lightPos.y = -50.0f + Math::Sin(Time::AbsoluteTime() * 360.0f * DEG_TO_RAD_F) * 20.0f;
	lightPos.z = 25.0f + Math::Sin(Time::AbsoluteTime() * 360.0f * DEG_TO_RAD_F) * 5.0f;

	shadowData->depthMVP = Math::Perspective(45.0f * DEG_TO_RAD_F, 1.0f, 0.01f, 1000.0f) * Math::LookAt(lightPos, Vector3Zero, Vector3Up);

#ifdef NH_DEBUG
	if (Settings::InEditor())
	{
		flyCamera.Update();
		cameraData->vp = flyCamera.ViewProjection();
		cameraData->eye = flyCamera.Eye();

		skyboxData->invViewProjection = flyCamera.ViewProjection().Inverse();
	}
	else
#endif
	{
		camera.Update();
		cameraData->vp = camera.ViewProjection();
		cameraData->eye = camera.Eye();

		skyboxData->invViewProjection = camera.ViewProjection().Inverse();
	}

	rendergraph->Run(commandBuffer, groups);

	return true;
}

void Scene::Resize()
{
	rendergraph->Resize();
}

Entity* Scene::AddEntity()
{
	entities.Push({ this, (U32)entities.Size() });

	return &entities.Back();
}

Entity* Scene::GetEntity(U32 id)
{
	return &entities[id];
}

void Scene::AddMesh(MeshInstance& instance)
{
	if (loaded)
	{
		U64 handle = *handles.GetInsert(instance.mesh->handle, meshDraws.Size());

		if (handle >= meshDraws.Size()) { meshDraws.Push({}); instance.handle = handle; }

		MeshDraw& draw = meshDraws[handle];

		PipelineGroup& group = groups[instance.material->type];

		if (!group.drawCount) //TODO: This doesn't get reset with a new scene
		{
			group.drawOffset = drawOffset;
			drawOffset += sizeof(VkDrawIndexedIndirectCommand) * 128;
			group.countOffset = countsOffset;
			countsOffset += sizeof(U32);
		}

		if (!draw.indexCount)
		{
			//Indices
			VkBufferCopy region{};
			region.dstOffset = indexOffset * sizeof(U32);
			region.size = instance.mesh->indicesSize;
			region.srcOffset = 0;

			Renderer::FillBuffer(buffers.indexBuffer, instance.mesh->indicesSize, instance.mesh->indices, 1, &region);

			draw.indexCount = instance.mesh->indicesSize / sizeof(U32);
			draw.indexOffset = indexOffset;
			indexOffset += draw.indexCount;

			//Vertices
			draw.vertexOffset = vertexOffset / sizeof(Vector3);

			U32 verticesSize = 0;
			for (VertexBuffer& buffer : instance.mesh->buffers)
			{
				verticesSize = Math::Max(verticesSize, buffer.size);
				region.dstOffset = vertexOffset;
				region.size = buffer.size;
				region.srcOffset = 0;

				Renderer::FillBuffer(buffers.vertexBuffers[buffer.type], region.size, buffer.buffer, 1, &region);
			}

			vertexOffset += verticesSize;
		}

		if (!draw.instanceCount)
		{
			draw.drawOffset = group.drawOffset + sizeof(VkDrawIndexedIndirectCommand) * group.drawCount;
			group.instanceOffset = instanceOffset;
			draw.instanceOffset = 0;
			instanceOffset += sizeof(InstanceData) * 1024;

			//Count
			VkBufferCopy region{};
			region.dstOffset = group.countOffset;
			region.size = sizeof(U32);
			region.srcOffset = 0;

			++group.drawCount;

			Renderer::FillBuffer(buffers.countsBuffer, sizeof(U32), &group.drawCount, 1, &region);
		}

		//TODO: Write drawcall 
		//Draw
		VkDrawIndexedIndirectCommand drawCommand{};
		drawCommand.indexCount = draw.indexCount;
		drawCommand.instanceCount = draw.instanceCount + 1;
		drawCommand.firstIndex = draw.indexOffset;
		drawCommand.vertexOffset = draw.vertexOffset;
		drawCommand.firstInstance = (group.instanceOffset + draw.instanceOffset) / sizeof(InstanceData);

		VkBufferCopy region{};
		region.dstOffset = draw.drawOffset;
		region.size = sizeof(VkDrawIndexedIndirectCommand);
		region.srcOffset = 0;

		Renderer::FillBuffer(buffers.drawBuffer, sizeof(VkDrawIndexedIndirectCommand), &drawCommand, 1, &region);

		//Instance
		instance.instanceOffset = draw.instanceCount * sizeof(InstanceData);

		region.dstOffset = group.instanceOffset + draw.instanceOffset + instance.instanceOffset;
		region.size = sizeof(InstanceData);
		region.srcOffset = 0;

		Renderer::FillBuffer(buffers.instanceBuffer, sizeof(InstanceData), &instance.instanceData, 1, &region);

		++draw.instanceCount;
	}
}

void Scene::UpdateMesh(MeshInstance& instance)
{
	MeshDraw& draw = meshDraws[instance.handle];

	PipelineGroup& group = groups[instance.material->type];

	VkBufferCopy region{};
	region.dstOffset = group.instanceOffset + draw.instanceOffset + instance.instanceOffset;
	region.size = sizeof(InstanceData);
	region.srcOffset = 0;

	Renderer::FillBuffer(buffers.instanceBuffer, sizeof(InstanceData), &instance.instanceData, 1, &region);
}

void Scene::SetSkybox(Skybox* skybox)
{
	SkyboxData* skyboxData = Renderer::GetSkyboxData();
	CameraData* cameraData = Renderer::GetCameraData();

	skyboxData->skyboxIndex = (U32)skybox->texture->handle;
	cameraData->skyboxIndex = (U32)skybox->texture->handle;
}

void Scene::Load()
{
	if (!loaded)
	{
		loaded = true;

		rendergraph->SetBuffers(buffers);

		for (ComponentPool* pool : componentPools)
		{
			pool->Load(this);
		}
	}
}

void Scene::Unload()
{
	if (loaded)
	{
		loaded = false;
	}
}

void Scene::Update()
{
	for (ComponentPool* pool : componentPools)
	{
		pool->Update(this);
	}
}