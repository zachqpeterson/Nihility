#pragma once

#include "ResourceDefines.hpp"

#include "Containers\Hashmap.hpp"
#include "Containers\Queue.hpp"

struct Font;
struct AudioClip;
struct aiTexture;
struct aiMaterial;
struct aiMesh;
struct aiScene;
struct DataReader;
struct KTXInfo;
struct VkImage_T;
struct Rendergraph;
struct RendergraphInfo;
struct MeshUpload;
struct ModelUpload;
struct Model;
struct Mesh;
struct Scene;
struct Shader;
struct Material;
struct MaterialData;
enum KTXType;
enum KTXFormat;
enum VkFormat;

class NH_API Resources
{
public:
	static Texture* CreateTexture(const TextureInfo& info, const SamplerInfo& samplerInfo = {});
	static Texture* CreateSwapchainTexture(VkImage_T* image, VkFormat format, U8 index);
	static Shader* CreateShader(const String& name, U8 pushConstantCount = 0, PushConstant* pushConstants = nullptr);
	static Rendergraph* CreateRendergraph(RendergraphInfo& info);
	static Scene* CreateScene(const String& name, CameraType cameraType, Rendergraph* rendergraph);

	static bool RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth);
	static bool RecreateSwapchainTexture(Texture* texture, VkImage_T* image);

	static Font* LoadFont(const String& path);
	static AudioClip* LoadAudio(const String& path);
	static Texture* LoadTexture(const String& path);
	static Skybox* LoadSkybox(const String& path);
	static Material* LoadMaterial(const String& path);
	static Mesh* LoadMesh(const String& path);
	static Model* LoadModel(const String& path);
	static Scene* LoadScene(const String& path);
	static Binary LoadBinary(const String& path);
	static String LoadBinaryString(const String& path);

	static void SaveScene(const Scene* scene);
	static void SaveBinary(const String& path, U32 size, void* data);

	static Texture* AccessDummyTexture();
	static Texture* AccessTexture(const String& name);
	static Texture* AccessTexture(HashHandle handle);

	static void	DestroyTexture(Texture* texture);
	static void DestroyBinary(Binary& binary);

	static U8 MipmapCount(U16 width, U16 height);

	//Convert 3rd party asset formats to nh formats
	static String UploadFont(const String& path);
	static String UploadAudio(const String& path);
	static String UploadSkybox(const String& path);
	static String UploadTexture(const String& path, const TextureUpload& upload = {});
	static String UploadTexture(const String& name, U32 index, const aiTexture* textureInfo, TextureUsage usage);
	static String UploadTextures(const String& name, U32 index, const aiTexture* textureInfo0, const aiTexture* textureInfo1, const aiTexture* textureInfo2, U8 def0, U8 def1, U8 def2, TextureUsage usage);
	static String UploadModel(const String& path);

private:
	static bool Initialize();
	static void Shutdown();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy);

	static void Update();

	//Texture Loading
	static U8* LoadKTX(DataReader& reader, U32& faceCount, U32& faceSize, U32& resolution, VkFormat& format);
	static void GetKTXInfo(U32 internalFormat, KTXInfo& info);
	static VkFormat GetKTXFormat(KTXType type, KTXFormat format);
	static U8* LoadHDRToCube(DataReader& reader, U32& faceSize, U32& resolution, VkFormat& format);

	//Assimp Utilities
	static String ParseAssimpMaterial(const String& name, const aiMaterial* materialInfo, const aiScene* scene);
	static String ParseAssimpMesh(const String& name, const aiMesh* meshInfo);
	static void ParseAssimpModel(ModelUpload& model, const aiScene* scene);

	static Texture*							dummyTexture;

	static Hashmap<String, Texture>			textures;
	static Hashmap<String, Skybox>			skyboxes;
	static Hashmap<String, Font>			fonts;
	static Hashmap<String, AudioClip>		audioClips;
	static Hashmap<String, Shader>			shaders;
	static Hashmap<String, Rendergraph>		rendergraphs;
	static Hashmap<String, Material>		materials;
	static Hashmap<String, Mesh>			meshes;
	static Hashmap<String, Model>			models;
	static Hashmap<String, Scene>			scenes;

	static Queue<ResourceUpdate>			bindlessTexturesToUpdate;

	STATIC_CLASS(Resources);
	friend class Renderer;
	friend class UI;
	friend class Engine;
	friend struct CommandBuffer;
	friend struct Shader;
	friend struct Pipeline;
	friend struct Scene;
};

template<typename Type>
inline void Resources::CleanupHashmap(Hashmap<String, Type>& hashmap, DestroyFn<Type*> destroy)
{
	typename Hashmap<String, Type>::Iterator end = hashmap.end();
	for (auto it = hashmap.begin(); it != end; ++it)
	{
		if (it.Valid() && !it->name.Blank())
		{
			destroy(hashmap.Obtain(it->handle));
		}
	}
}