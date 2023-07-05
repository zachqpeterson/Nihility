#include "Scene.hpp"
#include "Resources.hpp"
#include "Rendering\Renderer.hpp"
#include "Resources\Settings.hpp"

void Scene::Create()
{
	
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

	Renderer::RenderSkybox(skybox, constantBuffer);

	CommandBuffer* commands = Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	for (Mesh& mesh : meshes)
	{
		cbMap.buffer = mesh.materialBuffer;
		MeshData* meshData = (MeshData*)Renderer::MapBuffer(cbMap);
		if (meshData)
		{
			UploadMaterial(*meshData, mesh);

			Renderer::UnmapBuffer(cbMap);

			mesh.material->program->DrawMesh(commands, mesh, constantBuffer);
		}
	}
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