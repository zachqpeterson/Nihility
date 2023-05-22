#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Time.hpp"

Buffer* cubeVertexBuffer;
Buffer* cubeIndexBuffer;
Pipeline* cubePipeline;
Buffer* cubeConstantBuffer;
DescriptorSetLayout* cubeDescriptorSetLayout;
MeshDraw meshDraw{};

Vector3 eye = Vector3{ 0.0f, 0.0f, 0.5f };
Vector3 look = Vector3{ 0.0f, 0.0, -1.0f };
Vector3 right = Vector3{ 1.0f, 0.0, 0.0f };

F32 yaw = 0.0f;
F32 pitch = 0.0f;

bool Init()
{
	// Create pipeline state
	PipelineCreation pipelineCreation{};

	// Vertex input
	// TODO(marco): component format should be based on buffer view type
	pipelineCreation.vertexInput.AddVertexAttribute({ 0, 0, 0, VERTEX_COMPONENT_FLOAT3 }); // position
	pipelineCreation.vertexInput.AddVertexStream({ 0, 12, VERTEX_INPUT_RATE_VERTEX });

	pipelineCreation.vertexInput.AddVertexAttribute({ 1, 1, 0, VERTEX_COMPONENT_FLOAT4 }); // tangent
	pipelineCreation.vertexInput.AddVertexStream({ 1, 16, VERTEX_INPUT_RATE_VERTEX });

	pipelineCreation.vertexInput.AddVertexAttribute({ 2, 2, 0, VERTEX_COMPONENT_FLOAT3 }); // normal
	pipelineCreation.vertexInput.AddVertexStream({ 2, 12, VERTEX_INPUT_RATE_VERTEX });

	pipelineCreation.vertexInput.AddVertexAttribute({ 3, 3, 0, VERTEX_COMPONENT_FLOAT2 }); // texcoord
	pipelineCreation.vertexInput.AddVertexStream({ 3, 8, VERTEX_INPUT_RATE_VERTEX });

	// Render pass
	pipelineCreation.renderPass = Renderer::GetSwapchainOutput();
	// Depth
	pipelineCreation.depthStencil.SetDepth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

	pipelineCreation.shaders.
		SetName("Cube").
		AddStage("Cube.vert", VK_SHADER_STAGE_VERTEX_BIT).
		AddStage("Cube.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Descriptor set layout
	DescriptorSetLayoutCreation cubeRllCreation{};
	cubeRllCreation.AddBinding({ "LocalConstants", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, 0 });
	cubeRllCreation.AddBinding({ "MaterialConstant", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, 0 });
	cubeRllCreation.AddBinding({ "diffuseTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, 1 });
	cubeRllCreation.AddBinding({ "roughnessMetalnessTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, 1, 0 });
	cubeRllCreation.AddBinding({ "roughnessMetalnessTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 1, 0 });
	cubeRllCreation.AddBinding({ "emissiveTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 1, 0 });
	cubeRllCreation.AddBinding({ "occlusionTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 1, 0 });
	// Setting it into pipeline
	cubeDescriptorSetLayout = Resources::CreateDescriptorSetLayout(cubeRllCreation);
	pipelineCreation.AddDescriptorSetLayout(cubeDescriptorSetLayout);

	// Constant buffer
	BufferCreation bufferCreation;
	bufferCreation.Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(UniformData)).SetName("cube_cb");
	cubeConstantBuffer = Resources::CreateBuffer(bufferCreation);

	RasterizationCreation rasterization{};
	rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization.fill = FILL_MODE_SOLID;

	pipelineCreation.rasterization = rasterization;

	cubePipeline = Resources::CreatePipeline(pipelineCreation);

	Vector<Texture*> textures{ 3 };

	Texture* texture = Resources::LoadTexture("Avocado_baseColor.bmp");
	textures.Push(texture);
	texture = Resources::LoadTexture("Avocado_roughnessMetallic.bmp");
	textures.Push(texture);
	texture = Resources::LoadTexture("Avocado_normal.bmp");
	textures.Push(texture);

	void* bufferData{ nullptr };
	U32 bufferLength = Resources::LoadBinary("Avocado.bin", &bufferData);

	Vector<Buffer*> buffers{ 3 };
	U32 bufferSize = 3248;
	U32 bufferOffset = 0;
	U8* data = (U8*)bufferData + bufferOffset;

	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	bufferCreation.Reset().SetName("buffer_0").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	Buffer* br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 4872;
	bufferOffset = 3248;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_1").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 6496;
	bufferOffset = 8120;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_2").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 4872;
	bufferOffset = 14616;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_3").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	bufferSize = 4092;
	bufferOffset = 19488;
	data = (U8*)bufferData + bufferOffset;

	bufferCreation.Reset().SetName("buffer_4").Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

	br = Resources::CreateBuffer(bufferCreation);

	buffers.Push(br);

	Vector3 nodeScale = Vector3::One;

	Vector3 nodeTranslation = Vector3::Zero;

	Quaternion3 nodeRotation = Quaternion3::Identity;

	Transform transform;
	transform.position = nodeTranslation;
	transform.scale = nodeScale;
	transform.rotation = nodeRotation;

	Matrix4 finalMatrix{};
	transform.CalculateMatrix(finalMatrix);

	meshDraw.materialData.model = Matrix4::Identity;
	meshDraw.indexType = VK_INDEX_TYPE_UINT16;
	Buffer* indexBuffer = buffers[4];
	meshDraw.indexBuffer = indexBuffer;
	meshDraw.indexOffset = 0;
	meshDraw.count = 2046;

	I32 positionIndex = 3;
	I32 tangentIndex = 2;
	I32 normalIndex = 1;
	I32 texcoordIndex = 0;

	U16* indexData = (U16*)((U8*)bufferData + 19488);

	U32 vertexCount = 406;

	meshDraw.positionBuffer = buffers[positionIndex];
	meshDraw.positionOffset = 0;

	Vector3* positionData = (Vector3*)((U8*)bufferData + 14616);

	meshDraw.normalBuffer = buffers[normalIndex];
	meshDraw.normalOffset = 0;

	meshDraw.tangentBuffer = buffers[tangentIndex];
	meshDraw.tangentOffset = 0;

	meshDraw.materialData.flags |= MaterialFeatures_TangentVertexAttribute;

	meshDraw.texcoordBuffer = buffers[texcoordIndex];
	meshDraw.texcoordOffset = 0;

	meshDraw.materialData.flags |= MaterialFeatures_TexcoordVertexAttribute;

	// Descriptor Set
	DescriptorSetCreation dsCreation{};
	dsCreation.SetLayout(cubeDescriptorSetLayout).SetBuffer(cubeConstantBuffer, 0);

	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(MaterialData)).SetName("material");
	meshDraw.materialBuffer = Resources::CreateBuffer(bufferCreation);
	dsCreation.SetBuffer(meshDraw.materialBuffer, 1);

	meshDraw.materialData.baseColorFactor = Vector4::One;

	Texture* diffuseTexture = textures[0];
	Sampler* dummySampler = Resources::AccessDummySampler();

	dsCreation.SetTextureSampler(diffuseTexture, dummySampler, 2);

	meshDraw.materialData.flags |= MaterialFeatures_ColorTexture;

	Texture* roughnessTexture = textures[1];

	dsCreation.SetTextureSampler(roughnessTexture, dummySampler, 3);

	meshDraw.materialData.flags |= MaterialFeatures_RoughnessTexture;

	meshDraw.materialData.metallicFactor = 1.0f;
	meshDraw.materialData.roughnessFactor = 1.0f;

	meshDraw.materialData.occlusionFactor = 1.0f;
	Texture* dummyTexture = Resources::AccessDummyTexture();
	dsCreation.SetTextureSampler(dummyTexture, dummySampler, 4);
	dsCreation.SetTextureSampler(dummyTexture, dummySampler, 5);

	Texture* normalTexture = textures[2];

	dsCreation.SetTextureSampler(normalTexture, dummySampler, 6);

	meshDraw.materialData.flags |= MaterialFeatures_NormalTexture;

	meshDraw.descriptorSet = Resources::CreateDescriptorSet(dsCreation);

	return true;
}

void Update()
{
	Matrix4 globalModel{};

	{
		// Update rotating cube gpu data
		MapBufferParameters cbMap = { cubeConstantBuffer, 0, 0 };
		F32* cbData = (F32*)Renderer::MapBuffer(cbMap);
		if (cbData)
		{
			if (Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE))
			{
				U32 x, y;
				U32 prevX, prevY;
				Input::MousePos(x, y);
				Input::PreviousMousePos(prevX, prevY);
				pitch += (I32)(y - prevY) * 0.015f;
				yaw += (I32)(x - prevX) * 0.015f;

				pitch = Math::Clamp(pitch, -90.0f, 90.0f);

				if (yaw > 360.0f) { yaw -= 360.0f; }

				Quaternion3 rx{ Vector3::Right, -pitch };
				Quaternion3 ry{ Vector3::Up, -yaw };

				look = rx.ToMatrix3() * Vector3::Back;
				look = ry.ToMatrix3() * look;

				right = look.Cross(Vector3::Up);
			}

			F32 speed = (F32)(0.5f * Time::DeltaTime());
			if (Input::ButtonDown(BUTTON_CODE_SHIFT)) { speed *= 2.0f; }

			eye += look * speed * Input::ButtonDown(BUTTON_CODE_W);
			eye -= look * speed * Input::ButtonDown(BUTTON_CODE_S);
			eye += right * speed * Input::ButtonDown(BUTTON_CODE_D);
			eye -= right * speed * Input::ButtonDown(BUTTON_CODE_A);

			Matrix4 view;
			view.LookAt(eye, eye + look, Vector3::Up);
			Matrix4 projection;
			projection.SetPerspective(60.0f, Settings::WindowWidth() / (F32)Settings::WindowHeight(), 0.01f, 1000.0f);

			//F32 camHeight = 0.9375f / 20.0f;
			//F32 camWidth = 1.66666666667f / 20.0f;
			//projection.SetOrthographic(-camWidth, camWidth, -camHeight, camHeight, 0.1f, 1000.0f);

			// Calculate view projection matrix
			Matrix4 viewProjection = projection * view;

			UniformData uniformData{ };
			uniformData.vp = viewProjection;
			uniformData.m = globalModel;
			uniformData.eye = Vector4{ eye.x, eye.y, eye.z, 1.0f };
			uniformData.light = Vector4{ 2.0f, 2.0f, 0.0f, 1.0f };

			Memory::Copy(cbData, &uniformData, sizeof(UniformData));

			Renderer::UnmapBuffer(cbMap);
		}
	}

	CommandBuffer* commands = Renderer::GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	commands->Clear(0.3f, 0.9f, 0.3f, 1.0f);
	commands->ClearDepthStencil(1.0f, 0);
	commands->BindPass(Renderer::GetSwapchainPass());
	commands->BindPipeline(cubePipeline);
	commands->SetScissor(nullptr);
	commands->SetViewport(nullptr);

	meshDraw.materialData.modelInv = (globalModel * meshDraw.materialData.model).Transposed().Inverse();

	MapBufferParameters materialMap = { meshDraw.materialBuffer, 0, 0 };
	MaterialData* materialBufferData = (MaterialData*)Renderer::MapBuffer(materialMap);

	memcpy(materialBufferData, &meshDraw.materialData, sizeof(MaterialData));

	Renderer::UnmapBuffer(materialMap);

	commands->BindVertexBuffer(meshDraw.positionBuffer, 0, meshDraw.positionOffset);
	commands->BindVertexBuffer(meshDraw.normalBuffer, 2, meshDraw.normalOffset);

	if (meshDraw.materialData.flags & MaterialFeatures_TangentVertexAttribute)
	{
		commands->BindVertexBuffer(meshDraw.tangentBuffer, 1, meshDraw.tangentOffset);
	}
	else
	{
		commands->BindVertexBuffer(Resources::AccessDummyAttributeBuffer(), 1, 0);
	}

	if (meshDraw.materialData.flags & MaterialFeatures_TexcoordVertexAttribute)
	{
		commands->BindVertexBuffer(meshDraw.texcoordBuffer, 3, meshDraw.texcoordOffset);
	}
	else
	{
		commands->BindVertexBuffer(Resources::AccessDummyAttributeBuffer(), 3, 0);
	}

	commands->BindIndexBuffer(meshDraw.indexBuffer, meshDraw.indexOffset, meshDraw.indexType);
	commands->BindDescriptorSet(&meshDraw.descriptorSet, 1, nullptr, 0);

	commands->DrawIndexed(TOPOLOGY_TYPE_TRIANGLE, meshDraw.count, 1, 0, 0, 0);
}

void Shutdown()
{

}

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}