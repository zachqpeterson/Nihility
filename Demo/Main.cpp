#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Time.hpp"

Buffer* sceneConstantBuffer;
DescriptorSetLayout* cubeDescriptorSetLayout;
MeshDraw meshDraw{};
Camera camera{ true, 20.0f, 6.0f, 0.1f };

bool Init()
{
	camera.SetPerspective(0.1f, 4000.0f, 60.0f, (F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());

	PipelineCreation pipelineCreation{};
	pipelineCreation.renderPass = Renderer::GetSwapchainOutput();
	pipelineCreation.depthStencil.SetDepth(true, VK_COMPARE_OP_LESS_OR_EQUAL);
	pipelineCreation.blendState.AddBlendState().SetColor(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD);

	pipelineCreation.shaders.SetName("PBR").AddStage("Pbr.vert", VK_SHADER_STAGE_VERTEX_BIT).AddStage("Pbr.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Constant buffer
	BufferCreation bufferCreation;
	bufferCreation.Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(UniformData)).SetName("scene_cb");
	sceneConstantBuffer = Resources::CreateBuffer(bufferCreation);

	RasterizationCreation rasterization{};
	rasterization.cullMode = VK_CULL_MODE_NONE;
	rasterization.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization.fill = FILL_MODE_SOLID;

	pipelineCreation.rasterization = rasterization;

	pipelineCreation.name = "main_no_cull";
	Program* programNoCull = Resources::CreateProgram({ pipelineCreation });

	pipelineCreation.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;

	pipelineCreation.name = "main_cull";
	Program* programCull = Resources::CreateProgram({ pipelineCreation });

	MaterialCreation materialCreation;

	materialCreation.SetName("material_no_cull_opaque").SetProgram(programNoCull).SetRenderIndex(0);
	Material* materialNoCullOpaque = Resources::CreateMaterial(materialCreation);

	materialCreation.SetName("material_cull_opaque").SetProgram(programCull).SetRenderIndex(1);
	Material* materialCullOpaque = Resources::CreateMaterial(materialCreation);

	materialCreation.SetName("material_no_cull_transparent").SetProgram(programNoCull).SetRenderIndex(2);
	Material* materialNoCullTransparent = Resources::CreateMaterial(materialCreation);

	materialCreation.SetName("material_cull_transparent").SetProgram(programCull).SetRenderIndex(3);
	Material* materialCullTransparent = Resources::CreateMaterial(materialCreation);

	Vector<Texture*> textures{ 4 };

	Texture* texture = Resources::LoadTexture("BoomBox_baseColor.bmp", true);
	textures.Push(texture);
	texture = Resources::LoadTexture("BoomBox_occlusionRoughnessMetallic.bmp", true);
	textures.Push(texture);
	texture = Resources::LoadTexture("BoomBox_normal.bmp", true);
	textures.Push(texture);
	texture = Resources::LoadTexture("BoomBox_emissive.bmp", true);
	textures.Push(texture);

	void* bufferData{ nullptr };
	U32 bufferLength = Resources::LoadBinary("BoomBox.bin", &bufferData);

	Vector<Buffer*> buffers{ 5 };

	U32 bufferOffset = 0;
	U32 bufferSize = 28600;
	U8* data = (U8*)bufferData + bufferOffset;

	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	bufferCreation.Reset().SetName("buffer_0").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	Buffer* br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 42900;
	bufferOffset = 28600;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_1").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 57200;
	bufferOffset = 71500;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_2").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 42900;
	bufferOffset = 128700;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_3").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 36216;
	bufferOffset = 171600;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_4").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	Vector3 nodeScale = Vector3::One * 3.0f;

	Vector3 nodeTranslation = Vector3::Zero;

	Quaternion3 nodeRotation = Quaternion3::Identity;

	Transform transform;
	transform.position = nodeTranslation;
	transform.scale = nodeScale;
	transform.rotation = nodeRotation;

	Matrix4 finalMatrix{};
	transform.CalculateMatrix(finalMatrix);

	meshDraw.indexBuffer = buffers[4];
	meshDraw.indexOffset = 0;
	meshDraw.primitiveCount = 18108;

	I32 positionIndex = 3;
	I32 tangentIndex = 2;
	I32 normalIndex = 1;
	I32 texcoordIndex = 0;

	meshDraw.positionBuffer = buffers[positionIndex];
	meshDraw.positionOffset = 0;

	meshDraw.normalBuffer = buffers[normalIndex];
	meshDraw.normalOffset = 0;

	meshDraw.tangentBuffer = buffers[tangentIndex];
	meshDraw.tangentOffset = 0;

	meshDraw.texcoordBuffer = buffers[texcoordIndex];
	meshDraw.texcoordOffset = 0;

	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(MeshData)).SetName("material"); //TODO: Unique name
	meshDraw.materialBuffer = Resources::CreateBuffer(bufferCreation);

	meshDraw.diffuseTextureIndex = (U16)textures[0]->handle;
	meshDraw.metalRoughOcclTextureIndex = (U16)textures[1]->handle;
	meshDraw.normalTextureIndex = (U16)textures[2]->handle;
	meshDraw.emissivityTextureIndex = (U16)textures[3]->handle;

	meshDraw.baseColorFactor = Vector4::One;
	meshDraw.metalRoughOcclFactor = Vector4::One;
	meshDraw.emissiveFactor = Vector4::Zero;
	meshDraw.material = materialCullOpaque;
	meshDraw.scale = Vector3::One * 100.0f;

	//TODO: Load Multiple Meshes
	//TODO: Sort Meshes

	return true;
}

static void UploadMaterial(MeshData& meshData, const MeshDraw& meshDraw, const F32 globalScale)
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

	//For left-handed systems need to invert positive and negative Z.
	Matrix4 model{ Vector3::Zero, Quaternion3::Identity, meshDraw.scale * Vector3{globalScale, globalScale, -globalScale} };
	meshData.model = model;
	meshData.modelInv = model.Transposed().Inversed();
}

static void DrawMesh(CommandBuffer* commands, MeshDraw& meshDraw)
{
	DescriptorSetCreation dsCreation{};
	dsCreation.SetBuffer(sceneConstantBuffer, 0).SetBuffer(meshDraw.materialBuffer, 1);
	dsCreation.SetLayout(meshDraw.material->program->passes[0].descriptorSetLayout);
	DescriptorSet* descriptorSet = commands->CreateDescriptorSet(dsCreation);

	commands->BindVertexBuffer(meshDraw.positionBuffer, 0, meshDraw.positionOffset);
	commands->BindVertexBuffer(meshDraw.tangentBuffer, 1, meshDraw.tangentOffset);
	commands->BindVertexBuffer(meshDraw.normalBuffer, 2, meshDraw.normalOffset);
	commands->BindVertexBuffer(meshDraw.texcoordBuffer, 3, meshDraw.texcoordOffset);
	commands->BindIndexBuffer(meshDraw.indexBuffer, meshDraw.indexOffset);
	commands->BindDescriptorSet(&descriptorSet, 1, nullptr, 0);

	commands->DrawIndexed(TOPOLOGY_TYPE_TRIANGLE, meshDraw.primitiveCount, 1, 0, 0, 0);
}

void Update()
{
	float lightRange = 20.0f;
	float lightIntensity = 10.0f;

	if (Settings::Resized())
	{
		camera.SetAspectRatio((F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());
	}

	camera.Update();

	Matrix4 globalModel{};

	{
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

			Memory::Copy(cbData, &uniformData, sizeof(UniformData));

			Renderer::UnmapBuffer(cbMap);
		}

		//TODO: For each mesh
		cbMap.buffer = meshDraw.materialBuffer;
		MeshData* meshData = (MeshData*)Renderer::MapBuffer(cbMap);
		if (meshData)
		{
			UploadMaterial(*meshData, meshDraw, 1.0f);

			Renderer::UnmapBuffer(cbMap);
		}
	}

	CommandBuffer* commands = Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	commands->Clear(0.3f, 0.3f, 0.3f, 1.0f);
	commands->ClearDepthStencil(1.0f, 0);
	commands->BindPass(Renderer::GetSwapchainPass());
	commands->SetScissor(nullptr);
	commands->SetViewport(nullptr);
	
	Material* lastMaterial = nullptr;
	//TODO: Loop by material so that we can deal with multiple passes
	//TODO: For each mesh
	{
		if (meshDraw.material != lastMaterial)
		{
			Pipeline* pipeline = meshDraw.material->program->passes[0].pipeline;

			commands->BindPipeline(pipeline);
			lastMaterial = meshDraw.material;
		}

		DrawMesh(commands, meshDraw);
	}
}

void Shutdown()
{

}

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}