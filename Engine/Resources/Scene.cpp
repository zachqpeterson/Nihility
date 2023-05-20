#include "Scene.hpp"
#include "Resources.hpp"
#include "Rendering\Renderer.hpp"

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

bool SetupScene()
{
	//glTF* scene = Resources::LoadScene("Avocado.gltf");
	//
	//Vector<Texture*> images{ scene->imageCount };
	//
	//for (U32 imageIndex = 0; imageIndex < scene->imageCount; ++imageIndex)
	//{
	//	Image& image = scene->images[imageIndex];
	//	Texture* tr = Resources::LoadTexture(image.uri);
	//
	//	images.Push(tr);
	//}
	//
	//Vector<Sampler*> samplers{ scene->samplerCount };
	//
	//for (U32 samplerIndex = 0; samplerIndex < scene->samplerCount; ++samplerIndex)
	//{
	//	SamplerRef& sampler = scene->samplers[samplerIndex];
	//
	//	String samplerName{ "sampler_{}", samplerIndex };
	//
	//	SamplerCreation creation;
	//	creation.minFilter = sampler.minFilter == SamplerRef::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	//	creation.magFilter = sampler.magFilter == SamplerRef::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	//	creation.name = samplerName;
	//
	//	Sampler* sr = Resources::CreateSampler(creation);
	//	ASSERT(sr != nullptr);
	//
	//	samplers.Push(sr);
	//}
	//
	//Vector<void*> buffersData{ scene->bufferCount };
	//
	//for (U32 bufferIndex = 0; bufferIndex < scene->bufferCount; ++bufferIndex)
	//{
	//	BufferRef& buffer = scene->buffers[bufferIndex];
	//
	//	void* result;
	//	Resources::LoadBinary(buffer.uri, &result);
	//	buffersData.Push(result);
	//}
	//
	//Vector<Buffer*> buffers{ scene->bufferViewCount };
	//
	//for (U32 bufferIndex = 0; bufferIndex < scene->bufferViewCount; ++bufferIndex)
	//{
	//	String bufferName{ NO_INIT };
	//	U32 bufferSize = 0;
	//	U8* data = GetBufferData(scene->bufferViews, bufferIndex, buffersData, bufferName, &bufferSize);
	//
	//	// NOTE(marco): the target attribute of a BufferView is not mandatory, so we prepare for both uses
	//	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	//
	//	if (bufferName.Blank())
	//	{
	//		bufferName = { "buffer_{}", bufferIndex };
	//	}
	//	else
	//	{
	//		// NOTE(marco): some buffers might have the same name, which causes issues in the renderer cache
	//		bufferName = { "{}_{}", bufferName, bufferIndex };
	//	}
	//
	//	BufferCreation bufferCreation{};
	//	bufferCreation.SetName(bufferName).Set(flags, RESOURCE_USAGE_IMMUTABLE, bufferSize).SetData(data);
	//
	//	Buffer* br = Resources::CreateBuffer(bufferCreation);
	//	ASSERT(br != nullptr);
	//
	//	buffers.Push(br);
	//}
	//
	//Vector<MeshDraw> meshDraws{ scene->meshCount };
	//
	//Vector<Buffer*> customMeshBuffers{ 8 };
	//
	//Scene& rootScene = scene->scenes[scene->scene];
	//
	//Vector<I32> nodeParents{ scene->nodeCount, -1 };
	//Vector<U32> nodeStack{ 8 };
	//Vector<Matrix4> nodeMatrix{ scene->nodeCount, Matrix4::Identity };
	//
	//for (U32 nodeIndex = 0; nodeIndex < rootScene.nodeCount; ++nodeIndex)
	//{
	//	U32 rootNode = rootScene.nodes[nodeIndex];
	//	nodeStack.Push(rootNode);
	//}
	//
	//while (nodeStack.Size())
	//{
	//	U32 nodeIndex;
	//	nodeStack.Pop(nodeIndex);
	//	Node& node = scene->nodes[nodeIndex];
	//
	//	Matrix4 localMatrix{ };
	//
	//	if (node.matrixCount)
	//	{
	//		Memory::Copy(&localMatrix, node.matrix, sizeof(Matrix4));
	//	}
	//	else
	//	{
	//		Vector3 nodeScale{ 1.0f, 1.0f, 1.0f };
	//		if (node.scaleCount != 0)
	//		{
	//			ASSERT(node.scaleCount == 3);
	//			nodeScale = Vector3{ node.scale[0], node.scale[1], node.scale[2] };
	//		}
	//
	//		Vector3 node_translation{ 0.f, 0.f, 0.f };
	//		if (node.translationCount)
	//		{
	//			ASSERT(node.translationCount == 3);
	//			node_translation = Vector3{ node.translation[0], node.translation[1], node.translation[2] };
	//		}
	//
	//		Quaternion3 nodeRotation{};
	//		if (node.rotationCount)
	//		{
	//			ASSERT(node.rotationCount == 4);
	//			nodeRotation.x = node.rotation[0];
	//			nodeRotation.y = node.rotation[1];
	//			nodeRotation.z = node.rotation[2];
	//			nodeRotation.w = node.rotation[3];
	//		}
	//
	//		Transform transform;
	//		transform.position = node_translation;
	//		transform.scale = nodeScale;
	//		transform.rotation = nodeRotation;
	//
	//		transform.CalculateMatrix(localMatrix);
	//	}
	//
	//	nodeMatrix[nodeIndex] = localMatrix;
	//
	//	for (U32 childIndex = 0; childIndex < node.childrenCount; ++childIndex)
	//	{
	//		U32 childNodeIndex = node.children[childIndex];
	//		nodeParents[childNodeIndex] = nodeIndex;
	//		nodeStack.Push(childNodeIndex);
	//	}
	//
	//	if (node.mesh == I32_MAX) { continue; }
	//
	//	Mesh& mesh = scene->meshes[node.mesh];
	//
	//	Matrix4 finalMatrix = localMatrix;
	//	I32 nodeParent = nodeParents[nodeIndex];
	//	while (nodeParent != -1)
	//	{
	//		finalMatrix = nodeMatrix[nodeParent] * finalMatrix;
	//		nodeParent = nodeParents[nodeParent];
	//	}
	//
	//	for (U32 primitiveIndex = 0; primitiveIndex < mesh.primitiveCount; ++primitiveIndex)
	//	{
	//		MeshDraw meshDraw{};
	//
	//		meshDraw.materialData.model = Matrix4::Identity;
	//
	//		MeshPrimitive& meshPrimitive = mesh.primitives[primitiveIndex];
	//
	//		Accessor& indicesAccessor = scene->accessors[meshPrimitive.indices];
	//		ASSERT(indicesAccessor.componentType == Accessor::UNSIGNED_INT || indicesAccessor.componentType == Accessor::UNSIGNED_SHORT);
	//		meshDraw.indexType = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
	//
	//		BufferView& indices_buffer_view = scene->bufferViews[indicesAccessor.bufferView];
	//		Buffer* indices_buffer_gpu = buffers[indicesAccessor.bufferView];
	//		meshDraw.indexBuffer = indices_buffer_gpu;
	//		meshDraw.indexOffset = indicesAccessor.byteOffset == I32_MAX ? 0 : indicesAccessor.byteOffset;
	//		meshDraw.count = indicesAccessor.count;
	//		ASSERT((meshDraw.count % 3) == 0);
	//
	//		//TODO:
	//		I32 position_accessor_index = -1;//gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "POSITION");
	//		I32 tangent_accessor_index = -1;//gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "TANGENT");
	//		I32 normal_accessor_index = -1;//gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "NORMAL");
	//		I32 texcoord_accessor_index = -1;//gltf_get_attribute_accessor_index(meshPrimitive.attributes, meshPrimitive.attributeCount, "TEXCOORD_0");
	//
	//		Vector3* position_data = nullptr;
	//		String name{ NO_INIT };
	//		U32* index_data_32 = (U32*)GetBufferData(scene->bufferViews, indicesAccessor.bufferView, buffersData, name);
	//		U16* index_data_16 = (U16*)index_data_32;
	//		U32 vertexCount = 0;
	//
	//		if (position_accessor_index != -1)
	//		{
	//			Accessor& position_accessor = scene->accessors[position_accessor_index];
	//			BufferView& position_buffer_view = scene->bufferViews[position_accessor.bufferView];
	//			Buffer* position_buffer_gpu = buffers[position_accessor.bufferView];
	//
	//			vertexCount = position_accessor.count;
	//
	//			meshDraw.positionBuffer = position_buffer_gpu;
	//			meshDraw.positionOffset = position_accessor.byteOffset == I32_MAX ? 0 : position_accessor.byteOffset;
	//
	//			position_data = (Vector3*)GetBufferData(scene->bufferViews, position_accessor.bufferView, buffersData, name);
	//		}
	//		else
	//		{
	//			Logger::Fatal("No position data found!");
	//			continue;
	//		}
	//
	//		if (normal_accessor_index != -1)
	//		{
	//			Accessor& normal_accessor = scene->accessors[normal_accessor_index];
	//			BufferView& normal_buffer_view = scene->bufferViews[normal_accessor.bufferView];
	//			Buffer* normal_buffer_gpu = buffers[normal_accessor.bufferView];
	//
	//			meshDraw.normalBuffer = normal_buffer_gpu;
	//			meshDraw.normalOffset = normal_accessor.byteOffset == I32_MAX ? 0 : normal_accessor.byteOffset;
	//		}
	//		else
	//		{
	//			// NOTE(marco): we could compute this at runtime
	//			Vector<Vector3> normalsArray{ vertexCount, {} };
	//
	//			U32 index_count = meshDraw.count;
	//			for (U32 index = 0; index < index_count; index += 3)
	//			{
	//				U32 i0 = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? index_data_32[index] : index_data_16[index];
	//				U32 i1 = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? index_data_32[index + 1] : index_data_16[index + 1];
	//				U32 i2 = indicesAccessor.componentType == Accessor::UNSIGNED_INT ? index_data_32[index + 2] : index_data_16[index + 2];
	//
	//				Vector3 p0 = position_data[i0];
	//				Vector3 p1 = position_data[i1];
	//				Vector3 p2 = position_data[i2];
	//
	//				Vector3 a = p1 - p0;
	//				Vector3 b = p2 - p0;
	//
	//				Vector3 normal = a.Cross(b);
	//
	//				normalsArray[i0] = normalsArray[i0] + normal;
	//				normalsArray[i1] = normalsArray[i1] + normal;
	//				normalsArray[i2] = normalsArray[i2] + normal;
	//			}
	//
	//			for (U32 vertex = 0; vertex < vertexCount; ++vertex)
	//			{
	//				normalsArray[vertex].Normalize();
	//			}
	//
	//			BufferCreation normals_creation{ };
	//			normals_creation.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, normalsArray.Size() * sizeof(Vector3)).SetName("normals").SetData(normalsArray.Data());
	//
	//			meshDraw.normalBuffer = Resources::CreateBuffer(normals_creation);
	//			meshDraw.normalOffset = 0;
	//
	//			customMeshBuffers.Push(meshDraw.normalBuffer);
	//
	//			normalsArray.Destroy();
	//		}
	//
	//		if (tangent_accessor_index != -1)
	//		{
	//			Accessor& tangent_accessor = scene->accessors[tangent_accessor_index];
	//			BufferView& tangent_buffer_view = scene->bufferViews[tangent_accessor.bufferView];
	//			Buffer* tangent_buffer_gpu = buffers[tangent_accessor.bufferView];
	//
	//			meshDraw.tangentBuffer = tangent_buffer_gpu;
	//			meshDraw.tangentOffset = tangent_accessor.byteOffset == I32_MAX ? 0 : tangent_accessor.byteOffset;
	//
	//			meshDraw.materialData.flags |= MaterialFeatures_TangentVertexAttribute;
	//		}
	//
	//		if (texcoord_accessor_index != -1)
	//		{
	//			Accessor& texcoord_accessor = scene->accessors[texcoord_accessor_index];
	//			BufferView& texcoord_buffer_view = scene->bufferViews[texcoord_accessor.bufferView];
	//			Buffer* texcoord_buffer_gpu = buffers[texcoord_accessor.bufferView];
	//
	//			meshDraw.texcoordBuffer = texcoord_buffer_gpu;
	//			meshDraw.texcoordOffset = texcoord_accessor.byteOffset == I32_MAX ? 0 : texcoord_accessor.byteOffset;
	//
	//			meshDraw.materialData.flags |= MaterialFeatures_TexcoordVertexAttribute;
	//		}
	//
	//		//ASSERTM(meshPrimitive.material != I32_MAX, "Mesh with no material is not supported!");
	//		Material& material = scene->materials[meshPrimitive.material];
	//
	//		// Descriptor Set
	//		DescriptorSetCreation dsCreation{};
	//		dsCreation.SetLayout(cubeDescriptorSetLayout).SetBuffer(cubeConstantBuffer, 0);
	//
	//		bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(MaterialData)).SetName("material");
	//		meshDraw.materialBuffer = Resources::CreateBuffer(bufferCreation);
	//		dsCreation.SetBuffer(meshDraw.materialBuffer, 1);
	//
	//		meshDraw.materialData.baseColorFactor = material.pbrMetallicRoughness.baseColorFactor;
	//
	//		if (material.pbrMetallicRoughness.baseColorTexture.index != I32_MAX)
	//		{
	//			TextureRef& diffuseTexture = scene->textures[material.pbrMetallicRoughness.baseColorTexture.index];
	//			Texture* diffuseTextureGpu = images[diffuseTexture.source];
	//
	//			Sampler* sampler = Resources::AccessDummySampler();
	//			if (diffuseTexture.sampler != U32_MAX)
	//			{
	//				sampler = samplers[diffuseTexture.sampler];
	//			}
	//
	//			dsCreation.SetTextureSampler(diffuseTextureGpu, sampler, 2);
	//
	//			meshDraw.materialData.flags |= MaterialFeatures_ColorTexture;
	//		}
	//		else
	//		{
	//			dsCreation.SetTextureSampler(Resources::AccessDummyTexture(), Resources::AccessDummySampler(), 2);
	//		}
	//
	//		if (material.pbrMetallicRoughness.metallicRoughnessTexture.index != I32_MAX)
	//		{
	//			TextureRef& roughnessTexture = scene->textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
	//			Texture* roughness_texture_gpu = images[roughnessTexture.source];
	//
	//			Sampler* sampler = Resources::AccessDummySampler();
	//			if (roughnessTexture.sampler != I32_MAX)
	//			{
	//				sampler = samplers[roughnessTexture.sampler];
	//			}
	//
	//			dsCreation.SetTextureSampler(roughness_texture_gpu, sampler, 3);
	//
	//			meshDraw.materialData.flags |= MaterialFeatures_RoughnessTexture;
	//		}
	//		else
	//		{
	//			dsCreation.SetTextureSampler(Resources::AccessDummyTexture(), Resources::AccessDummySampler(), 3);
	//		}
	//
	//		if (material.pbrMetallicRoughness.metallicFactor != F32_MAX)
	//		{
	//			meshDraw.materialData.metallicFactor = material.pbrMetallicRoughness.metallicFactor;
	//		}
	//		else
	//		{
	//			meshDraw.materialData.metallicFactor = 1.0f;
	//		}
	//
	//		if (material.pbrMetallicRoughness.roughnessFactor != F32_MAX)
	//		{
	//			meshDraw.materialData.roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
	//		}
	//		else
	//		{
	//			meshDraw.materialData.roughnessFactor = 1.0f;
	//		}
	//
	//		if (material.occlusionTexture.index != I32_MAX)
	//		{
	//			TextureRef& occlusionTexture = scene->textures[material.occlusionTexture.index];
	//
	//			// NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
	//			// texture
	//			Texture* occlusion_texture_gpu = images[occlusionTexture.source];
	//
	//			Sampler* sampler = Resources::AccessDummySampler();
	//			if (occlusionTexture.sampler != I32_MAX)
	//			{
	//				sampler = samplers[occlusionTexture.sampler];
	//			}
	//
	//			dsCreation.SetTextureSampler(occlusion_texture_gpu, sampler, 4);
	//
	//			meshDraw.materialData.occlusionFactor = material.occlusionTexture.strength != F32_MAX ? material.occlusionTexture.strength : 1.0f;
	//			meshDraw.materialData.flags |= MaterialFeatures_OcclusionTexture;
	//		}
	//		else
	//		{
	//			meshDraw.materialData.occlusionFactor = 1.0f;
	//			dsCreation.SetTextureSampler(Resources::AccessDummyTexture(), Resources::AccessDummySampler(), 4);
	//		}
	//
	//		if (material.emissiveFactorCount != 0)
	//		{
	//			meshDraw.materialData.emissiveFactor = {
	//				material.emissiveFactor[0],
	//				material.emissiveFactor[1],
	//				material.emissiveFactor[2],
	//			};
	//		}
	//
	//		if (material.emissiveTexture.index != I32_MAX)
	//		{
	//			TextureRef& emissiveTexture = scene->textures[material.emissiveTexture.index];
	//
	//			// NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
	//			// texture
	//			Texture* emissive_texture_gpu = images[emissiveTexture.source];
	//
	//			Sampler* sampler = Resources::AccessDummySampler();
	//			if (emissiveTexture.sampler != I32_MAX)
	//			{
	//				sampler = samplers[emissiveTexture.sampler];
	//			}
	//
	//			dsCreation.SetTextureSampler(emissive_texture_gpu, sampler, 5);
	//
	//			meshDraw.materialData.flags |= MaterialFeatures_EmissiveTexture;
	//		}
	//		else
	//		{
	//			dsCreation.SetTextureSampler(Resources::AccessDummyTexture(), Resources::AccessDummySampler(), 5);
	//		}
	//
	//		if (material.normalTexture.index != I32_MAX)
	//		{
	//			TextureRef& normalTexture = scene->textures[material.normalTexture.index];
	//			Texture* normal_texture_gpu = images[normalTexture.source];
	//
	//			Sampler* sampler = Resources::AccessDummySampler();
	//			if (normalTexture.sampler != I32_MAX)
	//			{
	//				sampler = samplers[normalTexture.sampler];
	//			}
	//
	//			dsCreation.SetTextureSampler(normal_texture_gpu, sampler, 6);
	//
	//			meshDraw.materialData.flags |= MaterialFeatures_NormalTexture;
	//		}
	//		else
	//		{
	//			dsCreation.SetTextureSampler(Resources::AccessDummyTexture(), Resources::AccessDummySampler(), 6);
	//		}
	//
	//		meshDraw.descriptorSet = Resources::CreateDescriptorSet(dsCreation);
	//
	//		meshDraws.Push(meshDraw);
	//	}
	//}

	return true;
}