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

void Scene::AddMesh(MeshInstance& instance) //TODO: version that take a struct that contains multiple instances of one mesh
{
	if (loaded)
	{
		instance.handle = UploadMesh(instance.mesh);

		MeshDraw& draw = meshDraws[instance.handle];
		Pipeline* pipeline = instance.material->meshPipeline;

		SetupDraw(pipeline, draw);

		//Instance
		instance.instanceOffset = draw.instanceCount * sizeof(InstanceData);

		instanceWrites.Push(CreateWrite(pipeline->instanceOffset + draw.instanceOffset + instance.instanceOffset, 0, sizeof(InstanceData), &instance.instanceData));

		++draw.instanceCount;
	}
}

void Scene::UploadInstances(MeshInstanceCluster& instances)
{
	if (loaded)
	{
		instances.handle = UploadMesh(instances.mesh);

		MeshDraw& draw = meshDraws[instances.handle];
		Pipeline* pipeline = instances.material->meshPipeline;

		SetupDraw(pipeline, draw);

		//Instance
		instances.instanceOffset = draw.instanceCount * pipeline->shader->instanceStride;

		instanceWrites.Push(CreateWrite(pipeline->instanceOffset + draw.instanceOffset + instances.instanceOffset, 0, pipeline->shader->instanceStride * instances.instances.Size(), instances.instances.Data()));

		draw.instanceCount += (U32)instances.instances.Size();
	}
}

HashHandle Scene::UploadMesh(ResourceRef<Mesh>& mesh)
{
	U64 handle = *handles.GetInsert(mesh->handle, meshDraws.Size());

	if (handle < meshDraws.Size()) { return handle; }

	MeshDraw& draw = meshDraws.Push({});

	if (!draw.indexCount)
	{
		//Indices
		indexWrites.Push(CreateWrite(indexOffset * sizeof(U32), 0, mesh->indicesSize, mesh->indices));

		draw.indexCount = mesh->indicesSize / sizeof(U32);
		draw.indexOffset = indexOffset;
		indexOffset += draw.indexCount;

		//Vertices
		draw.vertexOffset = vertexOffset / sizeof(Vector3);

		U32 verticesSize = 0;
		for (VertexBuffer& buffer : mesh->buffers)
		{
			verticesSize = Math::Max(verticesSize, buffer.size);
			vertexWrites[buffer.type].Push(CreateWrite(vertexOffset, 0, buffer.size, buffer.buffer));
		}

		vertexOffset += verticesSize;
	}

	return handle;
}

void Scene::SetupDraw(Pipeline* pipeline, MeshDraw& draw)
{
	if (!pipeline->drawCount) //TODO: This doesn't get reset with a new scene
	{
		pipeline->drawSets.Push({ drawOffset, countsOffset });
		drawOffset += sizeof(VkDrawIndexedIndirectCommand) * 128;
		countsOffset += sizeof(U32);
		pipeline->instanceOffset = instanceOffset;
		pipeline->bufferOffsets[pipeline->instanceBuffer] = instanceOffset;
		instanceOffset += INSTANCE_BUFFER_SIZE / 32;

		Renderer::currentRendergraph->AddPreprocessing(pipeline);
	}

	if (!draw.instanceCount)
	{
		draw.drawOffset = pipeline->drawSets.Front().drawOffset + sizeof(VkDrawIndexedIndirectCommand) * pipeline->drawCount;
		draw.instanceOffset = pipeline->drawCount * pipeline->shader->instanceStride * 1024;

		++pipeline->drawCount;

		countsWrites.Push(CreateWrite(pipeline->drawSets.Front().countOffset, 0, sizeof(U32), &pipeline->drawCount));
	}

	VkDrawIndexedIndirectCommand drawCommand{};
	drawCommand.indexCount = draw.indexCount;
	drawCommand.instanceCount = draw.instanceCount + 1;
	drawCommand.firstIndex = draw.indexOffset;
	drawCommand.vertexOffset = draw.vertexOffset;
	drawCommand.firstInstance = draw.instanceOffset / pipeline->shader->instanceStride;

	drawWrites.Push(CreateWrite(draw.drawOffset, 0, sizeof(VkDrawIndexedIndirectCommand), &drawCommand));
}

void Scene::UpdateMesh(MeshInstance& instance)
{
	MeshDraw& draw = meshDraws[instance.handle];
	Pipeline* pipeline = instance.material->meshPipeline;

	U64 offset = pipeline->instanceOffset + draw.instanceOffset + instance.instanceOffset;

	instanceWrites.Push(CreateWrite(offset, 0, pipeline->shader->instanceStride, &instance.instanceData));
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
	PostProcessData* postProcessData = Renderer::GetPostProcessData();
	postProcessData->saturation = 1.0f;

	lightVal += (F32)Time::DeltaTime();

	globalData->lightPos.x = Math::Cos(lightVal) * 0.2f;
	globalData->lightPos.y = 1.0f;
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