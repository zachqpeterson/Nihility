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

private:
	void UploadMaterial(MeshData& meshData, Mesh* mesh);

public:
	String				name{ NO_INIT };
	HashHandle			handle;

	Camera				camera{};
	Buffer* sceneConstantBuffer{ nullptr };

	Skybox* skybox{ nullptr };
	Buffer* skyboxConstantBuffer{ nullptr };
	bool				drawSkybox{ true };

	PostProcessData		postProcessData{};
	Buffer* postProcessConstantBuffer{ nullptr };
	bool				updatePostProcess{ false };

	Vector<Model*>		models{}; //TODO: Objects
};