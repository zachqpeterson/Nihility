#include "Mesh.hpp"

#include "Scene.hpp"

void MeshComponent::Load(Scene* scene)
{
	scene->AddMesh(meshInstance);
}

void ModelComponent::Load(Scene* scene)
{
	for (MeshInstance& instance : model->meshes)
	{
		scene->AddMesh(instance);
	}
}