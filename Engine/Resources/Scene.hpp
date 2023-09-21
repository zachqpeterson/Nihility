#pragma once

#include "ResourceDefines.hpp"

struct NH_API Scene
{
public:
	void Create();
	~Scene();
	void Destroy();

	void Update();

	void AddModel(Model* model);
	void SetSkybox(const String& name);

public:
	String				name{};
	HashHandle			handle;

	Camera				camera{};

	Skybox* skybox{ nullptr };
	bool drawSkybox{ true };

	PostProcessData		postProcessData{};
	bool				updatePostProcess{ false };

	Vector<Model*>		models{}; //TODO: Objects
};