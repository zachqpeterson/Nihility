#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"
#include "Math/Math.hpp"

#include <Containers/Vector.hpp>

struct Shader;
struct Camera;
struct TextureMap;

struct Material
{
	void ApplyGlobals(Camera* camera);
	void ApplyInstances(bool update);

	U32 id{ U32_MAX };
	String name;
	U32 generation{ 0 };
	U32 instance{ 0 };
	F32 shininess{ 0.0f };
	U64 renderFrameNumber{ 0 };

	Shader* shader{ nullptr };
	Vector4 diffuseColor; //TODO: Color struct
	Vector<TextureMap> globalTextureMaps;
	Vector<TextureMap> instanceTextureMaps;
};