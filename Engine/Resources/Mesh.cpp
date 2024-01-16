#include "Mesh.hpp"

#include "Scene.hpp"
#include "Entity.hpp"

void MeshComponent::Update(Scene* scene)
{
	meshInstance.instanceData.model = meshInstance.model * scene->GetEntity(entityID)->transform.WorldMatrix();

	scene->UpdateMesh(meshInstance);
}

void MeshComponent::Load(Scene* scene)
{
	meshInstance.instanceData.model = meshInstance.model *scene->GetEntity(entityID)->transform.WorldMatrix();

	scene->AddMesh(meshInstance);
}

void ModelComponent::Update(Scene* scene)
{
	for (MeshInstance& instance : model->meshes)
	{
		Entity* entity = scene->GetEntity(entityID);
		const Matrix4& mat = entity->transform.WorldMatrix();

		instance.instanceData.model = instance.model * mat;

		scene->UpdateMesh(instance);
	}
}

void ModelComponent::Load(Scene* scene)
{
	for (MeshInstance& instance : model->meshes)
	{
		instance.instanceData.model = instance.model *scene->GetEntity(entityID)->transform.WorldMatrix();

		scene->AddMesh(instance);
	}
}