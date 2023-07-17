#include "Scene.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"

void Scene::Create()
{
	BufferCreation bufferCreation{};
	bufferCreation.Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(UniformData)).SetName(name + "_scene_cb");
	sceneConstantBuffer = Resources::CreateBuffer(bufferCreation);

	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(Matrix4)).SetName(name + "_skybox_cb");
	skyboxConstantBuffer = Resources::CreateBuffer(bufferCreation);

	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(PostProcessData)).SetName(name + "_postProcess_cb");
	postProcessConstantBuffer = Resources::CreateBuffer(bufferCreation);

	postProcessData.contrast = 1.0f;
	postProcessData.brightness = 0.0f;
	postProcessData.saturation = 1.0f;
	postProcessData.gammaCorrection = 0.8f;
}

Scene::~Scene() { Destroy(); }

void Scene::Destroy()
{
	name.Destroy();
	meshes.Destroy();
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

	MapBufferParameters cbMap = { sceneConstantBuffer, 0, 0 };
	F32* cbData = (F32*)Renderer::MapBuffer(cbMap);
	if (cbData)
	{
		UniformData uniformData{ };
		uniformData.vp = camera.ViewProjection();
		uniformData.eye = camera.Eye();
		uniformData.light = Vector4{ 0.0f, 1.0f, 3.0f, 1.0f };
		uniformData.lightRange = lightRange;
		uniformData.lightIntensity = lightIntensity;
		uniformData.skymapIndex = (U32)skybox->texture->handle;

		Memory::Copy(cbData, &uniformData, sizeof(UniformData));

		Renderer::UnmapBuffer(cbMap);
	}

	CommandBuffer* commands = Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	for (Mesh& mesh : meshes)
	{
		cbMap.buffer = mesh.materialBuffer;
		MeshData* meshData = (MeshData*)Renderer::MapBuffer(cbMap);
		if (meshData)
		{
			UploadMaterial(*meshData, mesh);

			Renderer::UnmapBuffer(cbMap);

			mesh.material->program->DrawMesh(commands, mesh, sceneConstantBuffer);
		}
	}

	if(drawSkybox)
	{
		MapBufferParameters cbMap = { skyboxConstantBuffer, 0, 0 };
		F32* cbData = (F32*)Renderer::MapBuffer(cbMap);
		if (cbData)
		{
			Matrix4 vp = camera.ViewProjectionNoTranslation();

			Memory::Copy(cbData, &vp, sizeof(Matrix4));

			Renderer::UnmapBuffer(cbMap);
		}

		Pipeline* skyboxPipeline = Resources::skyboxProgram->passes[0];

		Resources::UpdateDescriptorSet(skyboxPipeline->descriptorSets[Renderer::GetFrameIndex()][0], &skybox->texture, &skyboxConstantBuffer);

		skyboxPipeline->vertexBuffers[0] = skybox->buffer;
		skyboxPipeline->vertexBufferCount = 1;

		Resources::skyboxProgram->RunPasses(Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false));
	}

	if (updatePostProcess)
	{
		updatePostProcess = false;
		MapBufferParameters cbMap = { postProcessConstantBuffer, 0, 0 };
		F32* cbData = (F32*)Renderer::MapBuffer(cbMap);
		if (cbData)
		{
			Memory::Copy(cbData, &postProcessData, sizeof(PostProcessData));
			Renderer::UnmapBuffer(cbMap);
		}

		Resources::postProcessProgram->passes[0]->SetInput(postProcessConstantBuffer, 0);
	}

	Resources::postProcessProgram->RunPasses(commands);
}

void Scene::UploadMaterial(MeshData& meshData, const Mesh& mesh)
{
	meshData.textures[0] = mesh.diffuseTextureIndex;
	meshData.textures[1] = mesh.metalRoughOcclTextureIndex;
	meshData.textures[2] = mesh.normalTextureIndex;
	meshData.textures[3] = mesh.emissivityTextureIndex;
	meshData.baseColorFactor = mesh.baseColorFactor;
	meshData.metalRoughOcclFactor = mesh.metalRoughOcclFactor;
	meshData.emissiveFactor = mesh.emissiveFactor;
	meshData.alphaCutoff = mesh.alphaCutoff;
	meshData.flags = mesh.flags;

	Matrix4 model{ mesh.position, mesh.rotation, mesh.scale * Vector3{1.0f, 1.0f, -1.0f} };
	meshData.model = model;
	meshData.modelInv = model.Transposed().Inversed();
}