#include "Scene.hpp"
#include "Resources.hpp"
#include "Rendering\Renderer.hpp"
#include "Resources\Settings.hpp"

Scene::~Scene() { Destroy(); }

void Scene::Destroy()
{
	name.Destroy();
	meshDraws.Destroy();
	buffers.Destroy();
	textures.Destroy();
	samplers.Destroy();
}

void Scene::Update()
{
	float lightRange = 20.0f;
	float lightIntensity = 10.0f;

	if (Settings::Resized())
	{
		camera.SetAspectRatio((F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());
	}

	camera.Update();

	MapBufferParameters cbMap = { constantBuffer, 0, 0 };
	F32* cbData = (F32*)Renderer::MapBuffer(cbMap);
	if (cbData)
	{
		UniformData uniformData{ };
		uniformData.vp = camera.ViewProjection();
		uniformData.eye = camera.Eye();
		uniformData.light = Vector4{ 0.0f, 1.0f, 3.0f, 1.0f };
		uniformData.lightRange = lightRange;
		uniformData.lightIntensity = lightIntensity;

		Memory::Copy(cbData, &uniformData, sizeof(UniformData));

		Renderer::UnmapBuffer(cbMap);
	}

	for (MeshDraw& draw : meshDraws)
	{
		cbMap.buffer = draw.materialBuffer;
		MeshData* meshData = (MeshData*)Renderer::MapBuffer(cbMap);
		if (meshData)
		{
			UploadMaterial(*meshData, draw);

			Renderer::UnmapBuffer(cbMap);
		}
	}

	CommandBuffer* commands = Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	commands->Clear(0.3f, 0.3f, 0.3f, 1.0f);
	commands->ClearDepthStencil(1.0f, 0);
	commands->BindPass(Renderer::swapchain.RenderPass());
	commands->SetScissor(nullptr);
	commands->SetViewport(nullptr);

	Material* lastMaterial = nullptr;
	//TODO: Loop by material so that we can deal with multiple passes
	for (MeshDraw& draw : meshDraws)
	{
		if (draw.material != lastMaterial)
		{
			Pipeline* pipeline = draw.material->program->passes[0].pipeline;

			commands->BindPipeline(pipeline);
			lastMaterial = draw.material;
		}

		DrawMesh(commands, draw);
	}
}

void Scene::UploadMaterial(MeshData& meshData, const MeshDraw& meshDraw)
{
	meshData.textures[0] = meshDraw.diffuseTextureIndex;
	meshData.textures[1] = meshDraw.metalRoughOcclTextureIndex;
	meshData.textures[2] = meshDraw.normalTextureIndex;
	meshData.textures[3] = meshDraw.emissivityTextureIndex;
	meshData.baseColorFactor = meshDraw.baseColorFactor;
	meshData.metalRoughOcclFactor = meshDraw.metalRoughOcclFactor;
	meshData.emissiveFactor = meshDraw.emissiveFactor;
	meshData.alphaCutoff = meshDraw.alphaCutoff;
	meshData.flags = meshDraw.flags;

	Matrix4 model{ meshDraw.position, meshDraw.rotation, meshDraw.scale * Vector3{1.0f, 1.0f, -1.0f} };
	meshData.model = model;
	meshData.modelInv = model.Transposed().Inversed();
}

void Scene::DrawMesh(CommandBuffer* commands, MeshDraw& meshDraw)
{
	DescriptorSetCreation dsCreation{};
	dsCreation.SetBuffer(constantBuffer, 0).SetBuffer(meshDraw.materialBuffer, 1);
	dsCreation.SetLayout(meshDraw.material->program->passes[0].descriptorSetLayout);
	DescriptorSet* descriptorSet = commands->CreateDescriptorSet(dsCreation);

	commands->BindVertexBuffer(meshDraw.positionBuffer, 0);
	commands->BindVertexBuffer(meshDraw.tangentBuffer, 1);
	commands->BindVertexBuffer(meshDraw.normalBuffer, 2);
	commands->BindVertexBuffer(meshDraw.texcoordBuffer, 3);
	commands->BindIndexBuffer(meshDraw.indexBuffer);
	commands->BindDescriptorSet(&descriptorSet, 1, nullptr, 0);

	commands->DrawIndexed(TOPOLOGY_TYPE_TRIANGLE, meshDraw.primitiveCount, 1, 0, 0, 0);
}