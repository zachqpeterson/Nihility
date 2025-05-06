#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"

#include "Rendering/DescriptorSet.hpp"
#include "Rendering/PipelineLayout.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Pipeline.hpp"
#include "Rendering/Buffer.hpp"
#include "Containers/Freelist.hpp"
#include "Containers/Hashmap.hpp"
#include "Containers/String.hpp"
#include "Containers/Queue.hpp"

struct aiAnimation;
struct aiNodeAnim;
struct aiTexture;
struct aiScene;
struct aiNode;
struct aiMesh;

class NH_API Resources
{
public:
	static ResourceRef<Texture> LoadTexture(const String& path, const Sampler& sampler = {}, bool generateMipmaps = true, bool flipImage = false);

	static ResourceRef<Texture>& WhiteTexture();
	static ResourceRef<Texture>& PlaceholderTexture();

	static void CreateSprite(ResourceRef<Texture>& texture, const Transform& transform = {}, const Vector4& color = Vector4::One, const Vector2& textureCoord = Vector2::Zero, const Vector2& textureScale = Vector2::One);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void DestroyResources(Hashmap<String, Type>& hashmap, DestroyFn<Type&> destroy);

	static DescriptorSet dummySet;
	static DescriptorSet bindlessTexturesSet;
	static PipelineLayout spritePipelineLayout;
	static Shader spriteVertexShader;
	static Shader spriteFragmentShader;
	static Pipeline spritePipeline;
	static Buffer spriteVertexBuffer;
	static Buffer spriteIndexBuffer;
	static Buffer spriteInstanceBuffers[MaxSwapchainImages];
	static Vector<SpriteInstance> instances;

	static ResourceRef<Texture> whiteTexture;
	static ResourceRef<Texture> placeholderTexture;

	static Hashmap<String, Resource<Texture>> textures;

	static Queue<ResourceRef<Texture>> bindlessTexturesToUpdate;

	friend class Engine;
	friend class Renderer;

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