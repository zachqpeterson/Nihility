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

	MapBufferParameters cbMap = { postProcessConstantBuffer, 0, 0 };
	F32* cbData = (F32*)Renderer::MapBuffer(cbMap);
	if (cbData)
	{
		Memory::Copy(cbData, &postProcessData, sizeof(PostProcessData));
		Renderer::UnmapBuffer(cbMap);
	}

	updatePostProcess = false;

	PipelineConnection connection{};
	connection.type = CONNECTION_TYPE_BUFFER;
	connection.buffer = postProcessConstantBuffer;
	connection.set = 0;
	connection.binding = 0;

	Resources::postProcessProgram->passes[0]->AddConnection(connection);
}

Scene::~Scene() { Destroy(); }

void Scene::Destroy()
{
	name.Destroy();
}

void Scene::Update()
{
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
		uniformData.directionalLight = Vector4{ 0.0f, 3.0f, 1.0f, 1.0f };
		uniformData.directionalLightColor = Vector4{ 1.0f, 0.882352941f, 0.63921568627f, 1.0f};
		uniformData.ambientLight = Vector4{ 0.3f, 0.3f, 0.3f, 1.0f }; //brg
		uniformData.lightIntensity = 10.0f;
		if (drawSkybox) { uniformData.skyboxIndex = (U32)skybox->texture->handle; }

		Memory::Copy(cbData, &uniformData, sizeof(UniformData));

		Renderer::UnmapBuffer(cbMap);
	}

	CommandBuffer* commands = Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	for (Model* model : models)
	{
		for (U32 i = 0; i < model->meshCount; ++i)
		{
			Mesh* mesh = model->meshes[i];

			cbMap.buffer = mesh->material->materialBuffer;
			MeshData* meshData = (MeshData*)Renderer::MapBuffer(cbMap);
			if (meshData)
			{
				UploadMaterial(*meshData, mesh);

				Renderer::UnmapBuffer(cbMap);

				mesh->material->program->DrawMesh(commands, mesh, sceneConstantBuffer);
			}
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
	}

	Resources::postProcessProgram->RunPasses(commands);
}

void Scene::AddModel(Model* model)
{
	models.Push(model);
}

void Scene::SetSkybox(const String& name)
{
	skybox = Resources::LoadSkybox(name);
	drawSkybox = true;
}

void Scene::UploadMaterial(MeshData& meshData, Mesh* mesh)
{
	meshData.textures[0] = mesh->material->diffuseTextureIndex;
	meshData.textures[1] = mesh->material->metalRoughOcclTextureIndex;
	meshData.textures[2] = mesh->material->normalTextureIndex;
	meshData.textures[3] = mesh->material->emissivityTextureIndex;
	meshData.baseColorFactor = mesh->material->baseColorFactor;
	meshData.metalRoughOcclFactor = mesh->material->metalRoughOcclFactor;
	meshData.emissiveFactor = mesh->material->emissiveFactor;
	meshData.alphaCutoff = mesh->material->alphaCutoff;
	meshData.flags = mesh->material->flags;
	
	Matrix4 model{ Vector3::Zero, Vector3::Zero, Vector3{1.0f, 1.0f, -1.0f} };
	meshData.model = model;
	meshData.modelInv = model.Transposed().Inversed();
}