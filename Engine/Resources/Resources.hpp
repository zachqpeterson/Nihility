#pragma once

#include "Defines.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"

class NH_API Resources
{
public:
	//static Texture* LoadTexture(String& name);

private:
	static bool LoadBMP();
	static bool LoadPNG();
	static bool LoadJPG();
	static bool LoadQOI();

	static bool Initialize();
	static void Shutdown();

	//static Hashmap<Texture*> textures;

	STATIC_CLASS(Resources);
	friend class Engine;
};