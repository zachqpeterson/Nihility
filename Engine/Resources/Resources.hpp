#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "ModelInstance.hpp"

#include "Containers/Hashmap.hpp"
#include "Containers/String.hpp"

struct aiAnimation;
struct aiNodeAnim;
struct aiTexture;
struct aiScene;
struct aiNode;
struct aiMesh;

class NH_API Resources
{
public:
	static ResourceRef<Texture> LoadTexture(const aiTexture* texture, bool generateMipmaps = true, bool flipImage = false);
	static ResourceRef<Texture> LoadTexture(const String& path, bool generateMipmaps = true, bool flipImage = false);
	static ResourceRef<Model> LoadModel(const String& path);

	static ModelInstance CreateModelInstance(ResourceRef<Model> model, const Vector3& position = Vector3::Zero, const Vector3& rotation = Vector3::Zero, const Vector3& scale = Vector3::One);

	static ResourceRef<Texture> WhiteTexture();
	static ResourceRef<Texture> PlaceholderTexture();

private:
	static bool Initialize();
	static void Shutdown();

	static void ProcessNode(Model& model, Node& node, aiNode* aNode, const aiScene* scene, String assetDirectory);
	static bool ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* scene, String assetDirectory, Hashmap<String, ResourceRef<Texture>>& textures);
	static void AddAnimationChannels(Animation3D& animation, aiAnimation* aiAnimation);
	static AnimationChannel CreateAnimationChannel(aiNodeAnim* nodeAnim);

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void DestroyResources(Hashmap<String, Type>& hashmap, DestroyFn<Type&> destroy);

	static void DestroyModel(Resource<Model>& model);

	static ResourceRef<Texture> whiteTexture;
	static ResourceRef<Texture> placeholderTexture;

	static Hashmap<String, Resource<Model>> models;
	static Hashmap<String, Resource<Texture>> textures;

	friend class Engine;

	STATIC_CLASS(Resources);
};

template<typename Type>
inline void Resources::DestroyResources(Hashmap<String, Type>& hashmap, DestroyFn<Type&> destroy)
{
	using Iterator = typename Hashmap<String, Type>::Iterator;
	Iterator end = hashmap.end();
	for (Iterator it = hashmap.begin(); it != end; ++it)
	{
		if (it.Valid()) { destroy(*it); }
	}

	hashmap.Destroy();
}