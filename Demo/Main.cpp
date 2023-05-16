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
MeshDraw                   meshDraw{};

Vector3 eye = Vector3{ 0.0f, 2.5f, 2.0f };
Vector3 look = Vector3{ 0.0f, 0.0, -1.0f };
Vector3 right = Vector3{ 1.0f, 0.0, 0.0f };

F32 yaw = 0.0f;
F32 pitch = 0.0f;

enum MaterialFeatures {
	MaterialFeatures_ColorTexture = 1 << 0,
	MaterialFeatures_NormalTexture = 1 << 1,
	MaterialFeatures_RoughnessTexture = 1 << 2,
	MaterialFeatures_OcclusionTexture = 1 << 3,
	MaterialFeatures_EmissiveTexture = 1 << 4,

	MaterialFeatures_TangentVertexAttribute = 1 << 5,
	MaterialFeatures_TexcoordVertexAttribute = 1 << 6,
};

struct UniformData
{
	Matrix4 m;
	Matrix4 vp;
	Vector4 eye;
	Vector4 light;
};

static U8* GetBufferData(BufferView* bufferViews, U32 bufferIndex, Vector<void*>& buffersData, String& bufferName, U32* bufferSize = nullptr)
{
	BufferView& buffer = bufferViews[bufferIndex];

	I32 offset = buffer.byteOffset;
	if (offset == I32_MAX) { offset = 0; }

	bufferName = buffer.name;

	if (bufferSize != nullptr)
	{
		*bufferSize = buffer.byteLength;
	}

	U8* data = (U8*)buffersData[buffer.buffer] + offset;

	return data;
}

bool Init()
{
	glTF scene; //TODO: Load Scene file

	Vector<Texture*> images{ scene.imagesCount };

	for (U32 imageIndex = 0; imageIndex < scene.imagesCount; ++imageIndex)
	{
		Image& image = scene.images[imageIndex];
		Texture* tr = Resources::LoadTexture(image.uri);

		images.Push(tr);
	}

	Vector<Sampler*> samplers{ scene.samplersCount };

	for (U32 sampler_index = 0; sampler_index < scene.samplersCount; ++sampler_index)
	{
		SamplerScene& sampler = scene.samplers[sampler_index];

		String samplerName{ "sampler_{}", sampler_index };

		SamplerCreation creation;
		creation.minFilter = sampler.minFilter == SamplerScene::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		creation.magFilter = sampler.magFilter == SamplerScene::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		creation.name = samplerName;

		Sampler* sr = Resources::CreateSampler(creation);
		ASSERT(sr != nullptr);

		samplers.Push(sr);
	}

	Vector<void*> buffersData{ scene.buffersCount };

	for (U32 buffer_index = 0; buffer_index < scene.buffersCount; ++buffer_index)
	{
		BufferScene& buffer = scene.buffers[buffer_index];

		void* result;
		Resources::LoadBinary(buffer.uri, &result);
		buffersData.Push(result);
	}

	Vector<Buffer*> buffers{ scene.bufferViewsCount };

	for (U32 bufferIndex = 0; bufferIndex < scene.bufferViewsCount; ++bufferIndex)
	{
		String bufferName{ NO_INIT };
		U32 bufferSize = 0;
		U8* data = GetBufferData(scene.bufferViews, bufferIndex, buffersData, bufferName, &bufferSize);

		// NOTE(marco): the target attribute of a BufferView is not mandatory, so we prepare for both uses
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		if (bufferName.Blank())
		{
			bufferName = { "buffer_{}", bufferIndex };
		}
		else
		{
			// NOTE(marco); some buffers might have the same name, which causes issues in the renderer cache
			bufferName = { "{}_{}", bufferName, bufferIndex };
		}

		BufferCreation bufferCreation{};
		bufferCreation.SetName(bufferName).Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);

		Buffer* br = Resources::CreateBuffer(bufferCreation);
		ASSERT(br != nullptr);

		buffers.Push(br);
	}

	Vector<MeshDraw> meshDraws{ scene.meshesCount };

	Vector<BufferHandle> customMeshBuffers{ 8 };

	Vector4 dummyData[3]{ };
	BufferCreation bufferCreation{ };
	bufferCreation.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, sizeof(Vector4) * 3).SetData(dummyData).SetName("dummy_attribute_buffer");

	Buffer* dummyAttributeBuffer = Resources::CreateBuffer(bufferCreation);

	// Create pipeline state
	PipelineCreation pipelineCreation;

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
	cubeRllCreation.AddBinding({ "LocalConstants", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1 });
	cubeRllCreation.AddBinding({ "MaterialConstant", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1 });
	cubeRllCreation.AddBinding({ "diffuseTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, 1 });
	cubeRllCreation.AddBinding({ "roughnessMetalnessTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, 1 });
	cubeRllCreation.AddBinding({ "roughnessMetalnessTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 1 });
	cubeRllCreation.AddBinding({ "emissiveTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 1 });
	cubeRllCreation.AddBinding({ "occlusionTexture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 1 });
	// Setting it into pipeline
	cubeDescriptorSetLayout = Resources::CreateDescriptorSetLayout(cubeRllCreation);
	pipelineCreation.AddDescriptorSetLayout(cubeDescriptorSetLayout);

	// Constant buffer
	BufferCreation bufferCreation;
	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(UniformData)).SetName("cube_cb");
	cubeConstantBuffer = Resources::CreateBuffer(bufferCreation);

	cubePipeline = Renderer::CreatePipeline(pipelineCreation);

	Vector<MeshDraw> meshDraws;

	Mesh mesh;

	for (U32 primitiveIndex = 0; primitiveIndex < mesh.primitivesCount; ++primitiveIndex)
	{
		meshDraw.materialData.model = Matrix4::Identity;

		MeshPrimitive& meshPrimitive = mesh.primitives[primitiveIndex];

		Accessor& indicesAccessor = scene.accessors[meshPrimitive.indices];
		ASSERT(indicesAccessor.componentType == Accessor::UNSIGNED_INT || indicesAccessor.componentType == Accessor::UNSIGNED_SHORT);
		meshDraw.indexType = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;

		BufferView& indices_buffer_view = scene.bufferViews[indicesAccessor.bufferView];
		BufferResource& indices_buffer_gpu = buffers[indicesAccessor.bufferView];
		meshDraw.indexBuffer = indices_buffer_gpu.handle;
		meshDraw.indexOffset = indicesAccessor.byteOffset == I32_MAX ? 0 : indicesAccessor.byteOffset;
		meshDraw.count = indicesAccessor.count;
		ASSERT((meshDraw.count % 3) == 0);

		I32 position_accessor_index = gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "POSITION");
		I32 tangent_accessor_index = gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "TANGENT");
		I32 normal_accessor_index = gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "NORMAL");
		I32 texcoord_accessor_index = gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "TEXCOORD_0");

		Vector3* position_data = nullptr;
		U32* index_data_32 = (U32*)get_buffer_data(scene.bufferViews, indicesAccessor.bufferView, buffers_data);
		U16* index_data_16 = (U16*)index_data_32;
		U32 vertex_count = 0;

		if (position_accessor_index != -1)
		{
			Accessor& position_accessor = scene.accessors[position_accessor_index];
			BufferView& position_buffer_view = scene.bufferViews[position_accessor.bufferView];
			BufferResource& position_buffer_gpu = buffers[position_accessor.bufferView];

			vertex_count = position_accessor.count;

			meshDraw.positionBuffer = position_buffer_gpu.handle;
			meshDraw.positionOffset = position_accessor.byteOffset == I32_MAX ? 0 : position_accessor.byteOffset;

			position_data = (vec3s*)get_buffer_data(scene.bufferViews, position_accessor.bufferView, buffers_data);
		}
		else
		{
			Logger::Fatal("No position data found!");
			continue;
		}

		if (normal_accessor_index != -1)
		{
			Accessor& normal_accessor = scene.accessors[normal_accessor_index];
			BufferView& normal_buffer_view = scene.bufferViews[normal_accessor.bufferView];
			BufferResource& normal_buffer_gpu = buffers[normal_accessor.bufferView];

			meshDraw.normalBuffer = normal_buffer_gpu.handle;
			meshDraw.normalOffset = normal_accessor.byteOffset == I32_MAX ? 0 : normal_accessor.byteOffset;
		}
		else
		{
			// NOTE(marco): we could compute this at runtime
			Vector<Vector3> normalsArray{ vertex_count, {} };

			U32 index_count = meshDraw.count;
			for (U32 index = 0; index < index_count; index += 3)
			{
				U32 i0 = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? index_data_32[index] : index_data_16[index];
				U32 i1 = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? index_data_32[index + 1] : index_data_16[index + 1];
				U32 i2 = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? index_data_32[index + 2] : index_data_16[index + 2];

				Vector3 p0 = position_data[i0];
				Vector3 p1 = position_data[i1];
				Vector3 p2 = position_data[i2];

				Vector3 a = p1 - p0;
				Vector3 b = p2 - p0;

				Vector3 normal = a.Cross(b);

				normalsArray[i0] = normalsArray[i0] + normal;
				normalsArray[i1] = normalsArray[i1] + normal;
				normalsArray[i2] = normalsArray[i2] + normal;
			}

			for (U32 vertex = 0; vertex < vertex_count; ++vertex)
			{
				normalsArray[vertex].Normalize();
			}

			BufferCreation normals_creation{ };
			normals_creation.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, normalsArray.Size() * sizeof(Vector3)).SetName("normals").SetData(normalsArray.Data());

			meshDraw.normalBuffer = Renderer::CreateBuffer(normals_creation);
			meshDraw.normalOffset = 0;

			custom_mesh_buffers.push(meshDraw.normalBuffer);

			normalsArray.Destroy();
		}

		if (tangent_accessor_index != -1)
		{
			Accessor& tangent_accessor = scene.accessors[tangent_accessor_index];
			BufferView& tangent_buffer_view = scene.bufferViews[tangent_accessor.bufferView];
			BufferResource& tangent_buffer_gpu = buffers[tangent_accessor.bufferView];

			meshDraw.tangentBuffer = tangent_buffer_gpu.handle;
			meshDraw.tangentOffset = tangent_accessor.byteOffset == I32_MAX ? 0 : tangent_accessor.byteOffset;

			meshDraw.materialData.flags |= MaterialFeatures_TangentVertexAttribute;
		}

		if (texcoord_accessor_index != -1)
		{
			Accessor& texcoord_accessor = scene.accessors[texcoord_accessor_index];
			BufferView& texcoord_buffer_view = scene.bufferViews[texcoord_accessor.bufferView];
			BufferResource& texcoord_buffer_gpu = buffers[texcoord_accessor.bufferView];

			meshDraw.texcoordBuffer = texcoord_buffer_gpu.handle;
			meshDraw.texcoordOffset = texcoord_accessor.byteOffset == I32_MAX ? 0 : texcoord_accessor.byteOffset;

			meshDraw.materialData.flags |= MaterialFeatures_TexcoordVertexAttribute;
		}

		//ASSERTM(meshPrimitive.material != I32_MAX, "Mesh with no material is not supported!");
		Material& material = scene.materials[meshPrimitive.material];

		// Descriptor Set
		DescriptorSetCreation ds_creation{};
		ds_creation.SetLayout(cubeDescriptorSetLayout).Buffer(cubeConstantBuffer, 0);

		bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(MaterialData)).SetName("material");
		meshDraw.materialBuffer = Renderer::CreateBuffer(bufferCreation);
		ds_creation.Buffer(meshDraw.materialBuffer, 1);

		if (material.pbrMetallicRoughness != nullptr)
		{
			if (material.pbrMetallicRoughness->baseColorFactorCount != 0)
			{
				ASSERT(material.pbrMetallicRoughness->baseColorFactorCount == 4);

				meshDraw.materialData.baseColorFactor = {
					material.pbrMetallicRoughness->baseColorFactor[0],
					material.pbrMetallicRoughness->baseColorFactor[1],
					material.pbrMetallicRoughness->baseColorFactor[2],
					material.pbrMetallicRoughness->baseColorFactor[3],
				};
			}
			else
			{
				meshDraw.materialData.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
			}

			if (material.pbrMetallicRoughness->baseColorTexture != nullptr)
			{
				Texture& diffuseTexture = scene.textures[material.pbrMetallicRoughness->baseColorTexture->index];
				TextureResource& diffuseTextureGpu = images[diffuseTexture.source];

				SamplerHandle sampler_handle = dummySampler;
				if (diffuseTexture.sampler != I32_MAX)
				{
					sampler_handle = samplers[diffuseTexture.sampler].handle;
				}

				ds_creation.TextureSampler(diffuseTextureGpu.handle, sampler_handle, 2);

				meshDraw.materialData.flags |= MaterialFeatures_ColorTexture;
			}
			else
			{
				ds_creation.TextureSampler(dummyTexture, dummySampler, 2);
			}

			if (material.pbrMetallicRoughness->metallicRoughnessTexture != nullptr)
			{
				Texture& roughnessTexture = scene.textures[material.pbrMetallicRoughness->metallicRoughnessTexture->index];
				TextureResource& roughness_texture_gpu = images[roughnessTexture.source];

				SamplerHandle sampler_handle = dummySampler;
				if (roughnessTexture.sampler != I32_MAX)
				{
					sampler_handle = samplers[roughnessTexture.sampler].handle;
				}

				ds_creation.TextureSampler(roughness_texture_gpu.handle, sampler_handle, 3);

				meshDraw.materialData.flags |= MaterialFeatures_RoughnessTexture;
			}
			else
			{
				ds_creation.TextureSampler(dummyTexture, dummySampler, 3);
			}

			if (material.pbrMetallicRoughness->metallicFactor != F32_MAX)
			{
				meshDraw.materialData.metallicFactor = material.pbrMetallicRoughness->metallicFactor;
			}
			else
			{
				meshDraw.materialData.metallicFactor = 1.0f;
			}

			if (material.pbrMetallicRoughness->roughnessFactor != F32_MAX)
			{
				meshDraw.materialData.roughnessFactor = material.pbrMetallicRoughness->roughnessFactor;
			}
			else
			{
				meshDraw.materialData.roughnessFactor = 1.0f;
			}
		}

		if (material.occlusionTexture != nullptr)
		{
			Texture& occlusionTexture = scene.textures[material.occlusionTexture->index];

			// NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
			// texture
			TextureResource& occlusion_texture_gpu = images[occlusionTexture.source];

			SamplerHandle sampler_handle = dummySampler;
			if (occlusionTexture.sampler != I32_MAX)
			{
				sampler_handle = samplers[occlusionTexture.sampler].handle;
			}

			ds_creation.TextureSampler(occlusion_texture_gpu.handle, sampler_handle, 4);

			meshDraw.materialData.occlusionFactor = material.occlusionTexture->strength != F32_MAX ? material.occlusionTexture->strength : 1.0f;
			meshDraw.materialData.flags |= MaterialFeatures_OcclusionTexture;
		}
		else
		{
			meshDraw.materialData.occlusionFactor = 1.0f;
			ds_creation.TextureSampler(dummyTexture, dummySampler, 4);
		}

		if (material.emissiveFactorCount != 0)
		{
			meshDraw.materialData.emissiveFactor = {
				material.emissiveFactor[0],
				material.emissiveFactor[1],
				material.emissiveFactor[2],
			};
		}

		if (material.emissiveTexture != nullptr)
		{
			Texture& emissiveTexture = scene.textures[material.emissiveTexture->index];

			// NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
			// texture
			TextureResource& emissive_texture_gpu = images[emissiveTexture.source];

			SamplerHandle sampler_handle = dummySampler;
			if (emissiveTexture.sampler != I32_MAX)
			{
				sampler_handle = samplers[emissiveTexture.sampler].handle;
			}

			ds_creation.TextureSampler(emissive_texture_gpu.handle, sampler_handle, 5);

			meshDraw.materialData.flags |= MaterialFeatures_EmissiveTexture;
		}
		else
		{
			ds_creation.TextureSampler(dummyTexture, dummySampler, 5);
		}

		if (material.normalTexture != nullptr)
		{
			Texture& normalTexture = scene.textures[material.normalTexture->index];
			TextureResource& normal_texture_gpu = images[normal_texture.source];

			SamplerHandle sampler_handle = dummySampler;
			if (normalTexture.sampler != I32_MAX)
			{
				sampler_handle = samplers[normalTexture.sampler].handle;
			}

			ds_creation.TextureSampler(normal_texture_gpu.handle, sampler_handle, 6);

			meshDraw.materialData.flags |= MaterialFeatures_NormalTexture;
		}
		else
		{
			ds_creation.TextureSampler(dummyTexture, dummySampler, 6);
		}

		meshDraw.descriptorSet = Renderer::CreateDescriptorSet(ds_creation);

		meshDraws.Push(meshDraw);
	}

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
				pitch += (y - prevY) * 0.3f;
				yaw += (x - prevX) * 0.3f;

				pitch = Math::Clamp(pitch, -60.0f, 60.0f);

				if (yaw > 360.0f) { yaw -= 360.0f; }

				Quaternion3 rx{ Vector3::Right, -pitch };
				Quaternion3 ry{ Vector3::Up, -yaw };

				look = rx.ToMatrix3() * Vector3::Back;
				look = ry.ToMatrix3() * look;

				right = look.Cross(Vector3::Up);
			}

			F32 speed = 5.0f * Time::DeltaTime();

			eye += look * speed * Input::ButtonDown(BUTTON_CODE_W);
			eye -= look * speed * Input::ButtonDown(BUTTON_CODE_S);
			eye += right * speed * Input::ButtonDown(BUTTON_CODE_D);
			eye -= right * speed * Input::ButtonDown(BUTTON_CODE_A);

			Matrix4 view;
			view.LookAt(eye, eye + look, Vector3::Up);
			Matrix4 projection;
			projection.SetPerspective(60.0f, Settings::WindowWidth() / Settings::WindowHeight(), 0.01f, 1000.0f);

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

	MeshDraw meshDraw{};
	meshDraw.materialData.modelInv = (globalModel * meshDraw.materialData.model).Transposed().Inverse();

	MapBufferParameters material_map = { meshDraw.materialBuffer, 0, 0 };
	MaterialData* material_buffer_data = (MaterialData*)Renderer::MapBuffer(material_map);

	memcpy(material_buffer_data, &meshDraw.materialData, sizeof(MaterialData));

	Renderer::UnmapBuffer(material_map);

	commands->BindVertexBuffer(meshDraw.positionBuffer, 0, meshDraw.positionOffset);
	commands->BindVertexBuffer(meshDraw.normalBuffer, 2, meshDraw.normalOffset);

	if (meshDraw.materialData.flags & MaterialFeatures_TangentVertexAttribute)
	{
		commands->BindVertexBuffer(meshDraw.tangentBuffer, 1, meshDraw.tangentOffset);
	}
	else
	{
		commands->BindVertexBuffer(dummyAttributeBuffer, 1, 0);
	}

	if (meshDraw.materialData.flags & MaterialFeatures_TexcoordVertexAttribute)
	{
		commands->BindVertexBuffer(meshDraw.texcoordBuffer, 3, meshDraw.texcoordOffset);
	}
	else
	{
		commands->BindVertexBuffer(dummyAttributeBuffer, 3, 0);
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