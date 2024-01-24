#include "Mesh.hpp"

#include "Scene.hpp"
#include "Entity.hpp"

void MeshComponent::Update(Scene* scene)
{
	if (meshInstance.mesh)
	{
		Matrix4 mat = modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

		meshInstance.instanceData.model = mat;

		scene->UpdateMesh(meshInstance);
	}
}

void MeshComponent::Load(Scene* scene)
{
	if (meshInstance.mesh)
	{
		Matrix4 mat = modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

		meshInstance.instanceData.model = mat;

		scene->AddMesh(meshInstance);
	}
}

void ModelComponent::Update(Scene* scene)
{
	if (model)
	{
		for (U32 i = 0; i < model->meshes.Size(); ++i)
		{
			Matrix4 mat = model->matrices[i] * modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

			model->meshes[i].instanceData.model = mat;

			scene->UpdateMesh(model->meshes[i]);
		}
	}
}

void ModelComponent::Load(Scene* scene)
{
	if (model)
	{
		for (U32 i = 0; i < model->meshes.Size(); ++i)
		{
			Matrix4 mat = model->matrices[i] * modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

			model->meshes[i].instanceData.model = mat;

			scene->AddMesh(model->meshes[i]);
		}
	}
}