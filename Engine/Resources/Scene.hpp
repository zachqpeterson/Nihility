#pragma once

#include "ResourceDefines.hpp"

struct NH_API Scene
{
public:
	void Create(CameraType cameraType);
	~Scene();
	void Destroy();

	bool Update();

	void AddModel(Model* model);
	void SetSkybox(Skybox* newSkybox);

public:
	String				name{};
	HashHandle			handle;

	Camera				camera{};

	Skybox*				skybox{ nullptr };

	Vector<Model*>		models{}; //TODO: Objects
};