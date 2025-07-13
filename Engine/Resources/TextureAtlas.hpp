#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"

struct NH_API TextureAtlas
{
	void Create(ResourceRef<Texture> texture, U32 countX, U32 countY)
	{
		this->texture = texture;
		this->countX = countX;
		this->countY = countY;

		spriteWidth = 1.0f / countX;
		spriteHeight = 1.0f / countY;
	}

	ResourceRef<Texture> texture;

	F32 spriteWidth;
	F32 spriteHeight;

	U32 countX;
	U32 countY;
};