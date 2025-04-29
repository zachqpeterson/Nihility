#include "Resources.hpp"

#include "Core/Logger.hpp"
#include "Rendering/Renderer.hpp"
#include "Containers/Stack.hpp"
#include "Containers/Pair.hpp"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vma/vk_mem_alloc.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

ResourceRef<Texture> Resources::whiteTexture;
ResourceRef<Texture> Resources::placeholderTexture;

Hashmap<String, Resource<Model>> Resources::models(1024);
Hashmap<String, Resource<Texture>> Resources::textures(1024);

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	whiteTexture = LoadTexture("textures/white.png");
	placeholderTexture = LoadTexture("textures/missing_texture.png");

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	DestroyResources(models, DestroyModel);
	DestroyResources(textures, Renderer::DestroyTexture);
}

ResourceRef<Texture> Resources::LoadTexture(const aiTexture* textureData, bool generateMipmaps, bool flipImage)
{
	if (!textureData) { return nullptr; }

	I32 texWidth;
	I32 texHeight;
	I32 numberOfChannels;

	stbi_set_flip_vertically_on_load(flipImage);

	U8* data = nullptr;
	if (textureData->mHeight == 0) { data = stbi_load_from_memory((const U8*)textureData, textureData->mWidth, &texWidth, &texHeight, &numberOfChannels, STBI_rgb_alpha); }
	else { data = stbi_load_from_memory((const U8*)textureData, textureData->mWidth * textureData->mHeight, &texWidth, &texHeight, &numberOfChannels, STBI_rgb_alpha); }

	if (!data)
	{
		Logger::Error("Failed To Load Texture Data!");
		stbi_image_free(data);
		return nullptr;
	}

	U64 handle;
	Resource<Texture>& texture = *textures.Request(textureData->mFilename.C_Str(), handle);

	texture->width = texWidth;
	texture->height = texHeight;
	texture->depth = 1;
	texture->size = texWidth * texHeight * 4;
	texture->mipmapLevels = generateMipmaps ? (U32)Math::Floor(Math::Log2((F32)Math::Max(texWidth, texHeight))) + 1 : 1;

	stbi_image_free(data);

	if (!Renderer::UploadTexture(texture, data))
	{
		Logger::Error("Failed To Upload Texture Data!");
		return nullptr;
	}

	return { texture, handle };
}

ResourceRef<Texture> Resources::LoadTexture(const String& path, bool generateMipmaps, bool flipImage)
{
	if (path.Blank()) { Logger::Error("Blank Path Passed To LoadTexture!"); return nullptr; }

	U64 handle;
	Resource<Texture>& texture = *textures.Request(path, handle);

	if (!texture->name.Blank()) { return { texture, handle }; }

	I32 texWidth;
	I32 texHeight;
	I32 numberOfChannels;

	File file{ path, FILE_OPEN_RESOURCE_READ };
	if (file.Opened())
	{
		String data = file.ReadAll();

		stbi_set_flip_vertically_on_load(flipImage);
		U8* textureData = stbi_load_from_memory((U8*)data.Data(), (I32)data.Size(), &texWidth, &texHeight, &numberOfChannels, STBI_rgb_alpha);

		if (!textureData)
		{
			Logger::Error("Failed To Load Texture Data!");
			stbi_image_free(textureData);
			return nullptr;
		}

		texture->width = texWidth;
		texture->height = texHeight;
		texture->depth = 1;
		texture->size = texWidth * texHeight * 4;
		texture->mipmapLevels = generateMipmaps ? (U32)Math::Floor(Math::Log2((F32)Math::Max(texWidth, texHeight))) + 1 : 1;

		stbi_image_free(textureData);

		if (!Renderer::UploadTexture(texture, textureData))
		{
			Logger::Error("Failed To Upload Texture Data!");
			return nullptr;
		}

		return { texture, handle };
	}

	Logger::Error("Failed To Open File: ", path, '!');
	return nullptr;
}

ResourceRef<Model> Resources::LoadModel(const String& path)
{
	if (path.Blank()) { Logger::Error("Blank Path Passed To LoadModel!"); return nullptr; }

	U64 handle;
	Resource<Model>& model = *models.Request(path, handle);

	if (!model->modelFilename.Blank()) { return { model, handle }; }

	const aiScene* scene = aiImportFile(path.Data(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_ValidateDataStructure | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Logger::Error("Failed To Load Model: ", path);
		return nullptr;
	}

	U32 meshCount = scene->mNumMeshes;

	for (U32 i = 0; i < meshCount; ++i)
	{
		U32 vertexCount = scene->mMeshes[i]->mNumVertices;
		U32 faceCount = scene->mMeshes[i]->mNumFaces;

		model->vertexCount += vertexCount;
		model->triangleCount += faceCount;
	}

	if (scene->HasTextures())
	{
		U32 numTextures = scene->mNumTextures;

		for (U32 i = 0; i < scene->mNumTextures; ++i)
		{
			ResourceRef<Texture> newTex = Resources::LoadTexture(scene->mTextures[i]);
			if (newTex)
			{
				model->textures.Insert({ FORMAT, "*", i }, newTex);
			}
		}
	}

	String assetDirectory = path.SubString(0, path.LastIndexOf('/'));

	aiNode* rootNode = scene->mRootNode;
	model->rootNode = { String{ rootNode->mName.C_Str() }, nullptr };
	model->nodeMap(256);
	model->boneOffsetMatrices(256);

	ProcessNode(model.type, model->rootNode, rootNode, scene, assetDirectory);

	for (const Node* node : model->nodeList)
	{
		String nodeName = node->NodeName();

		Bone** bone = model->boneList.Find([nodeName](Bone** bone) { return (*bone)->BoneName() == nodeName; });

		if (bone) { model->boneOffsetMatrices.Insert(nodeName, (*bone)->OffsetMatrix()); }
	}

	for (const MeshData& mesh : model->modelMeshes)
	{
		Buffer vertexBuffer;
		vertexBuffer.Create(BufferType::Vertex, mesh.vertices.Size() * sizeof(VertexData));
		vertexBuffer.UploadVertexData(mesh);
		model->vertexBuffers.Emplace(vertexBuffer);

		Buffer indexBuffer;
		indexBuffer.Create(BufferType::Index, mesh.indices.Size() * sizeof(U32));
		indexBuffer.UploadIndexData(mesh);
		model->indexBuffers.Emplace(indexBuffer);
	}

	U32 numAnims = scene->mNumAnimations;
	for (U32 i = 0; i < numAnims; ++i)
	{
		aiAnimation* animation = scene->mAnimations[i];

		Animation3D animClip{};
		AddAnimationChannels(animClip, animation);
		if (animClip.ClipName().Empty()) { animClip.SetClipName({ FORMAT, i }); }
		model->animClips.Emplace(animClip);
	}

	model->modelFilenamePath = path;
	U64 start = path.LastIndexOf('/');
	model->modelFilename = path.SubString(start, path.Size() - start);

	const aiMatrix4x4& transform = rootNode->mTransformation;

	model->rootTransformMatrix = {
		transform.a1, transform.b1, transform.c1, transform.d1,
		transform.a2, transform.b2, transform.c2, transform.d2,
		transform.a3, transform.b3, transform.c3, transform.d3,
		transform.a4, transform.b4, transform.c4, transform.d4
	};

	return { model, handle };
}

ModelInstance Resources::CreateModelInstance(ResourceRef<Model> model, const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	ModelInstance instance{};

	instance.model = model;
	instance.position = position;
	instance.rotation = rotation;
	instance.scale = scale;

	instance.boneMatrices.Resize(model->BoneList().Size());

	for (Matrix4& matrix : instance.boneMatrices) { matrix = Matrix4::Identity; }

	instance.UpdateModelRootMatrix();

	return instance;
}

void Resources::ProcessNode(Model& model, Node& rootNode, aiNode* aNode, const aiScene* scene, String assetDirectory)
{
	Stack<Pair<aiNode*, Node*>> nodes;
	nodes.Push({ aNode, &rootNode });

	Pair<aiNode*, Node*> currentNode;
	while (nodes.Pop(currentNode))
	{
		U32 numMeshes = currentNode.a->mNumMeshes;
		if (numMeshes > 0)
		{
			for (U32 i = 0; i < numMeshes; ++i)
			{
				aiMesh* modelMesh = scene->mMeshes[currentNode.a->mMeshes[i]];

				Mesh mesh;
				ProcessMesh(mesh, modelMesh, scene, assetDirectory, model.textures);

				model.modelMeshes.Emplace(mesh.Data());

				Vector<Bone*>& flatBones = mesh.BoneList();
				for (Bone* bone : flatBones)
				{
					Bone** b = model.boneList.Find([bone](Bone** otherBone) { return bone->BoneId() == (*otherBone)->BoneId(); });

					if (!b) { model.boneList.Push(bone); }
				}
			}
		}

		model.nodeMap.Insert(String{ currentNode.a->mName.C_Str() }, currentNode.b);
		model.nodeList.Emplace(currentNode.b);

		U32 childCount = currentNode.a->mNumChildren;

		for (U32 i = 0; i < childCount; ++i)
		{
			nodes.Push({ currentNode.a->mChildren[i], currentNode.b->AddChild(String{ currentNode.a->mChildren[i]->mName.C_Str() }) });
		}
	}
}

bool Resources::ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* scene, String assetDirectory, Hashmap<String, ResourceRef<Texture>>& textures)
{
	mesh.meshName = aiMesh->mName.C_Str();

	mesh.triangleCount = aiMesh->mNumFaces;
	mesh.vertexCount = aiMesh->mNumVertices;

	aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
	if (material)
	{
		aiString materialName = material->GetName();

		if (aiMesh->mMaterialIndex >= 0)
		{
			Vector<aiTextureType> supportedTexTypes = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR };
			for (const aiTextureType& texType : supportedTexTypes)
			{
				U32 textureCount = material->GetTextureCount(texType);
				for (U32 i = 0; i < textureCount; ++i)
				{
					aiString textureName;
					material->GetTexture(texType, i, &textureName);

					String texName = textureName.C_Str();
					mesh.mesh.textures.Insert((TextureType)texType, texName);

					if (!texName.Empty() && texName.IndexOf('*') != -1)
					{
						String texNameWithPath{ FORMAT, assetDirectory + '/' + texName };
						ResourceRef<Texture> newTex = Resources::LoadTexture(texNameWithPath);
						if (newTex)
						{
							textures.Insert(texName, newTex);
						}
					}
				}
			}
		}

		aiColor4D baseColor(0.0f, 0.0f, 0.0f, 1.0f);
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == aiReturn_SUCCESS && textures.Empty())
		{
			baseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
			mesh.mesh.usesPBRColors = true;
		}
	}

	for (U32 i = 0; i < mesh.vertexCount; ++i)
	{
		VertexData vertex;
		vertex.position.x = aiMesh->mVertices[i].x;
		vertex.position.y = aiMesh->mVertices[i].y;
		vertex.position.z = aiMesh->mVertices[i].z;

		if (aiMesh->HasVertexColors(0))
		{
			vertex.color.x = aiMesh->mColors[0][i].r;
			vertex.color.y = aiMesh->mColors[0][i].g;
			vertex.color.z = aiMesh->mColors[0][i].b;
			vertex.color.w = aiMesh->mColors[0][i].a;
		}
		else
		{
			if (mesh.mesh.usesPBRColors)
			{
				vertex.color = mesh.baseColor;
			}
			else
			{
				vertex.color = Vector4::One;
			}
		}

		if (aiMesh->HasNormals())
		{
			vertex.normal.x = aiMesh->mNormals[i].x;
			vertex.normal.y = aiMesh->mNormals[i].y;
			vertex.normal.z = aiMesh->mNormals[i].z;
		}
		else
		{
			vertex.normal = Vector3::Zero;
		}

		if (aiMesh->HasTextureCoords(0))
		{
			vertex.uv.x = aiMesh->mTextureCoords[0][i].x;
			vertex.uv.y = aiMesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.uv = Vector2::Zero;
		}

		mesh.mesh.vertices.Emplace(vertex);
	}

	for (U32 i = 0; i < mesh.triangleCount; ++i)
	{
		aiFace face = aiMesh->mFaces[i];
		mesh.mesh.indices.Push(face.mIndices[0]);
		mesh.mesh.indices.Push(face.mIndices[1]);
		mesh.mesh.indices.Push(face.mIndices[2]);
	}

	if (aiMesh->HasBones())
	{
		U32 numBones = aiMesh->mNumBones;
		for (U32 boneId = 0; boneId < numBones; ++boneId)
		{
			U32 numWeights = aiMesh->mBones[boneId]->mNumWeights;
			const aiMatrix4x4& offsetMatrix = aiMesh->mBones[boneId]->mOffsetMatrix;

			mesh.boneList.Push(new Bone(boneId, String{ aiMesh->mBones[boneId]->mName.C_Str() }, Matrix4{
				offsetMatrix.a1, offsetMatrix.b1, offsetMatrix.c1, offsetMatrix.d1,
				offsetMatrix.a2, offsetMatrix.b2, offsetMatrix.c2, offsetMatrix.d2,
				offsetMatrix.a3, offsetMatrix.b3, offsetMatrix.c3, offsetMatrix.d3,
				offsetMatrix.a4, offsetMatrix.b4, offsetMatrix.c4, offsetMatrix.d4
				}));

			for (U32 weight = 0; weight < numWeights; ++weight)
			{
				U32 vertexId = aiMesh->mBones[boneId]->mWeights[weight].mVertexId;
				F32 vertexWeight = aiMesh->mBones[boneId]->mWeights[weight].mWeight;

				Vector4Int currentIds = mesh.mesh.vertices[vertexId].boneNumber;
				Vector4 currentWeights = mesh.mesh.vertices[vertexId].boneWeight;

				for (U32 i = 0; i < 4; ++i)
				{
					if (currentWeights[i] == 0.0f)
					{
						currentIds[i] = boneId;
						currentWeights[i] = vertexWeight;
						break;
					}
				}

				mesh.mesh.vertices[vertexId].boneNumber = currentIds;
				mesh.mesh.vertices[vertexId].boneWeight = currentWeights;
			}
		}
	}

	return true;
}

void Resources::AddAnimationChannels(Animation3D& animation, aiAnimation* aiAnimation)
{
	animation.clipName = aiAnimation->mName.C_Str();
	animation.clipDuration = (F32)aiAnimation->mDuration;
	animation.clipTicksPerSecond = (F32)aiAnimation->mTicksPerSecond;

	for (U32 i = 0; i < aiAnimation->mNumChannels; ++i)
	{
		animation.animChannels.Emplace(CreateAnimationChannel(aiAnimation->mChannels[i]));
	}
}

AnimationChannel Resources::CreateAnimationChannel(aiNodeAnim* nodeAnim)
{
	AnimationChannel channel{};

	channel.nodeName = nodeAnim->mNodeName.C_Str();
	channel.preState = (AnimationBehaviour)nodeAnim->mPreState;
	channel.postState = (AnimationBehaviour)nodeAnim->mPostState;

	U32 translationCount = nodeAnim->mNumPositionKeys;
	U32 rotationCount = nodeAnim->mNumRotationKeys;
	U32 scalingCount = nodeAnim->mNumScalingKeys;

	for (U32 i = 0; i < translationCount; ++i)
	{
		channel.translationTimings.Emplace((F32)nodeAnim->mPositionKeys[i].mTime);
		channel.translations.Emplace(nodeAnim->mPositionKeys[i].mValue.x, nodeAnim->mPositionKeys[i].mValue.y, nodeAnim->mPositionKeys[i].mValue.z);
	}

	for (U32 i = 0; i < rotationCount; ++i)
	{
		channel.rotationTimings.Emplace((F32)nodeAnim->mRotationKeys[i].mTime);
		channel.rotations.Emplace(nodeAnim->mRotationKeys[i].mValue.w, nodeAnim->mRotationKeys[i].mValue.x, nodeAnim->mRotationKeys[i].mValue.y, nodeAnim->mRotationKeys[i].mValue.z);
	}

	for (U32 i = 0; i < scalingCount; ++i)
	{
		channel.scaleTimings.Emplace((F32)nodeAnim->mScalingKeys[i].mTime);
		channel.scalings.Emplace(nodeAnim->mScalingKeys[i].mValue.x, nodeAnim->mScalingKeys[i].mValue.y, nodeAnim->mScalingKeys[i].mValue.z);
	}

	for (U32 i = 0; i < channel.translationTimings.Size() - 1; ++i)
	{
		channel.inverseTranslationTimeDiffs.Emplace(1.0f / (channel.translationTimings[i + 1] - channel.translationTimings[i]));
	}

	for (U32 i = 0; i < channel.rotationTimings.Size() - 1; ++i)
	{
		channel.inverseRotationTimeDiffs.Emplace(1.0f / (channel.rotationTimings[i + 1] - channel.rotationTimings[i]));
	}

	for (U32 i = 0; i < channel.scaleTimings.Size() - 1; ++i)
	{
		channel.inverseScaleTimeDiffs.Emplace(1.0f / (channel.scaleTimings[i + 1] - channel.scaleTimings[i]));
	}

	return channel;
}

ResourceRef<Texture> Resources::WhiteTexture()
{
	return whiteTexture;
}

ResourceRef<Texture> Resources::PlaceholderTexture()
{
	return placeholderTexture;
}

void Resources::DestroyModel(Resource<Model>& model)
{
	for (Buffer& buffer : model->vertexBuffers) { buffer.Destroy(); }
	for (Buffer& buffer : model->indexBuffers) { buffer.Destroy(); }
}
