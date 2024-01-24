#include "Scene.hpp"

#include "Entity.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Rendering\RenderingDefines.hpp"
#include "Core\Time.hpp"

#define INSTANCE_BUFFER_SIZE sizeof(InstanceData) * MEGABYTES(1)

void Scene::Create(CameraType cameraType)
{
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
		buffers.vertexBuffers[i] = Renderer::CreateBuffer(MEGABYTES(32), BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	}

	stagingBuffer = Renderer::CreateBuffer(MEGABYTES(32), BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_TYPE_CPU_VISIBLE | BUFFER_MEMORY_TYPE_CPU_COHERENT);
	buffers.instanceBuffer = Renderer::CreateBuffer(INSTANCE_BUFFER_SIZE, BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	buffers.indexBuffer = Renderer::CreateBuffer(MEGABYTES(64), BUFFER_USAGE_INDEX_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	buffers.drawBuffer = Renderer::CreateBuffer(MEGABYTES(32), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_INDIRECT_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	buffers.countsBuffer = Renderer::CreateBuffer(sizeof(U32) * 32, BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_INDIRECT_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);

	ForEach<RegisteredComponents>([this]<ComponentType Type>() { CreatePool<Type>(); }); //TODO: Create runtime typelist
}

void Scene::Destroy()
{
	name.Destroy();

	entities.Destroy();

	meshDraws.Destroy();
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

		instance.handle = handle;

		if (handle >= meshDraws.Size()) { meshDraws.Push({}); }

		MeshDraw& draw = meshDraws[handle];
		Pipeline* pipeline = instance.material->meshPipeline;

		if (!pipeline->drawCount) //TODO: This doesn't get reset with a new scene
		{
			pipeline->drawSets.Push({ drawOffset, countsOffset });
			drawOffset += sizeof(VkDrawIndexedIndirectCommand) * 128;
			countsOffset += sizeof(U32);
			pipeline->instanceOffset = instanceOffset;
			instanceOffset += INSTANCE_BUFFER_SIZE / 32;

			Renderer::currentRendergraph->AddPreprocessing(pipeline);
		}

		if (!draw.indexCount)
		{
			//Indices
			indexWrites.Push(CreateWrite(indexOffset * sizeof(U32), 0, instance.mesh->indicesSize, instance.mesh->indices));

			draw.indexCount = instance.mesh->indicesSize / sizeof(U32);
			draw.indexOffset = indexOffset;
			indexOffset += draw.indexCount;

			//Vertices
			draw.vertexOffset = vertexOffset / sizeof(Vector3);

			U32 verticesSize = 0;
			for (VertexBuffer& buffer : instance.mesh->buffers)
			{
				verticesSize = Math::Max(verticesSize, buffer.size);
				vertexWrites[buffer.type].Push(CreateWrite(vertexOffset, 0, buffer.size, buffer.buffer));
			}

			vertexOffset += verticesSize;
		}

		if (!draw.instanceCount)
		{
			draw.drawOffset = pipeline->drawSets.Front().drawOffset + sizeof(VkDrawIndexedIndirectCommand) * pipeline->drawCount;
			draw.instanceOffset = pipeline->drawCount * sizeof(InstanceData) * 1024;

			//Count
			++pipeline->drawCount;

			countsWrites.Push(CreateWrite(pipeline->drawSets.Front().countOffset, 0, sizeof(U32), &pipeline->drawCount));
		}

		//Draw
		VkDrawIndexedIndirectCommand drawCommand{};
		drawCommand.indexCount = draw.indexCount;
		drawCommand.instanceCount = draw.instanceCount + 1;
		drawCommand.firstIndex = draw.indexOffset;
		drawCommand.vertexOffset = draw.vertexOffset;
		drawCommand.firstInstance = (pipeline->instanceOffset + draw.instanceOffset) / sizeof(InstanceData);

		drawWrites.Push(CreateWrite(draw.drawOffset, 0, sizeof(VkDrawIndexedIndirectCommand), &drawCommand));

		//Instance
		instance.instanceOffset = draw.instanceCount * sizeof(InstanceData);

		instanceWrites.Push(CreateWrite(pipeline->instanceOffset + draw.instanceOffset + instance.instanceOffset, 0, sizeof(InstanceData), &instance.instanceData));

		++draw.instanceCount;
	}
}

void Scene::UpdateMesh(MeshInstance& instance)
{
	MeshDraw& draw = meshDraws[instance.handle];
	Pipeline* pipeline = instance.material->meshPipeline;

	U64 offset = pipeline->instanceOffset + draw.instanceOffset + instance.instanceOffset;

	instanceWrites.Push(CreateWrite(offset, 0, sizeof(InstanceData), &instance.instanceData));
}

void Scene::SetSkybox(const ResourceRef<Skybox>& skybox)
{
	SkyboxData* skyboxData = Renderer::GetSkyboxData();
	GlobalData* globalData = Renderer::GetGlobalData();

	skyboxData->skyboxIndex = (U32)skybox->texture->handle;
	globalData->skyboxIndex = (U32)skybox->texture->handle;
}

void Scene::Load(ResourceRef<Rendergraph>& rendergraph)
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

F32 lightVal = 0.0f;

void Scene::Update()
{
	for (ComponentPool* pool : componentPools)
	{
		pool->Update(this);
	}

	ShadowData* shadowData = Renderer::GetShadowData();
	SkyboxData* skyboxData = Renderer::GetSkyboxData();
	GlobalData* globalData = Renderer::GetGlobalData();

	lightVal += (F32)Time::DeltaTime();

	globalData->lightPos.x = Math::Cos(lightVal) * 0.2f;
	globalData->lightPos.y = 0.3f;
	globalData->lightPos.z = Math::Sin(lightVal) * 0.2f;

	shadowData->depthViewProjection = Math::Perspective(45.0f, 1.0f, 0.01f, 1000.0f) * Math::LookAt(globalData->lightPos, Vector3Zero, Vector3Up);

	globalData->lightSpace = shadowData->depthViewProjection;

#ifdef NH_DEBUG
	if (Settings::InEditor())
	{
		flyCamera.Update();
		globalData->viewProjection = flyCamera.ViewProjection();
		globalData->eye = flyCamera.Eye();

		skyboxData->invViewProjection = flyCamera.ViewProjection().Inverse();
	}
	else
#endif
	{
		camera.Update();
		globalData->viewProjection = camera.ViewProjection();
		globalData->eye = camera.Eye();

		skyboxData->invViewProjection = camera.ViewProjection().Inverse();
	}

	VkBufferCopy region{};
	region.dstOffset = 0;
	region.size = sizeof(GlobalData);
	region.srcOffset = 0;

	Renderer::FillBuffer(Renderer::globalsBuffer, sizeof(GlobalData), globalData, 1, &region);

	if (indexWrites.Size())
	{
		Renderer::FillBuffer(buffers.indexBuffer, stagingBuffer, (U32)indexWrites.Size(), indexWrites.Data());
		indexWrites.Clear();
	}

	for (U32 i = 0; i < VERTEX_TYPE_COUNT - 1; ++i)
	{
		Vector<VkBufferCopy>& writes = vertexWrites[i];

		if (writes.Size())
		{
			Renderer::FillBuffer(buffers.vertexBuffers[i], stagingBuffer, (U32)writes.Size(), writes.Data());
			writes.Clear();
		}
	}

	if (countsWrites.Size())
	{
		Renderer::FillBuffer(buffers.countsBuffer, stagingBuffer, (U32)countsWrites.Size(), countsWrites.Data());
		countsWrites.Clear();
	}

	if (drawWrites.Size())
	{
		Renderer::FillBuffer(buffers.drawBuffer, stagingBuffer, (U32)drawWrites.Size(), drawWrites.Data());
		drawWrites.Clear();
	}

	if (instanceWrites.Size())
	{
		Renderer::FillBuffer(buffers.instanceBuffer, stagingBuffer, (U32)instanceWrites.Size(), instanceWrites.Data());
		instanceWrites.Clear();
	}

	stagingBuffer.allocationOffset = 0;
}

VkBufferCopy Scene::CreateWrite(U64 dstOffset, U64 srcOffset, U64 size, void* data)
{
	VkBufferCopy region{};
	region.dstOffset = dstOffset;
	region.size = size;
	region.srcOffset = stagingBuffer.allocationOffset;
	stagingBuffer.allocationOffset += size;

	Memory::Copy((U8*)stagingBuffer.data + region.srcOffset, (U8*)data + srcOffset, size);

	return region;
}