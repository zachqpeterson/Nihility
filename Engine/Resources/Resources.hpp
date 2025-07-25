#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"
#include "Material.hpp"
#include "Font.hpp"

#include "Audio/Audio.hpp"
#include "Rendering/DescriptorSet.hpp"
#include "Rendering/PipelineLayout.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Pipeline.hpp"
#include "Rendering/Buffer.hpp"
#include "Containers/Freelist.hpp"
#include "Containers/Hashmap.hpp"
#include "Containers/String.hpp"
#include "Containers/Queue.hpp"

class NH_API Resources
{
public:
	static ResourceRef<Texture> LoadTexture(const String& path, const Sampler& sampler = {}, bool generateMipmaps = true);
	static ResourceRef<Font> LoadFont(const String& path);
	static ResourceRef<AudioClip> LoadAudio(const String& path);

	static String UploadResource(const String& path);
	static String UploadTexture(const String& path);
	static String UploadFont(const String& path);
	static String UploadAudio(const String& path);

	static ResourceRef<Texture>& WhiteTexture();
	static ResourceRef<Texture>& PlaceholderTexture();

	static const DescriptorSet& DummyDescriptorSet();
	static const DescriptorSet& BindlessTexturesDescriptorSet();

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	template<typename Type> using DestroyFn = void(*)(Type);
	template<typename Type> static void DestroyResources(Hashmap<String, Type>& hashmap, DestroyFn<Type&> destroy);

	static DescriptorSet dummySet;
	static DescriptorSet bindlessTexturesSet;

	static ResourceRef<Texture> whiteTexture;
	static ResourceRef<Texture> placeholderTexture;

	static Hashmap<String, Resource<Texture>> textures;
	static Hashmap<String, Resource<Font>> fonts;
	static Hashmap<String, Resource<AudioClip>> audioClips;

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