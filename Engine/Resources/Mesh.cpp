#include "Mesh.hpp"

#include "Scene.hpp"
#include "Entity.hpp"

void MeshComponent::Update(Scene* scene)
{
	if (meshInstance.mesh)
	{
		Matrix4 mat = modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

		Memory::Copy(meshInstance.instanceData.data + sizeof(U32), &mat, sizeof(Matrix4));

		scene->UpdateInstance(meshInstance);
	}
}

void MeshComponent::Load(Scene* scene)
{
	if (meshInstance.mesh)
	{
		Matrix4 mat = modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

		Memory::Copy(meshInstance.instanceData.data + sizeof(U32), &mat, sizeof(Matrix4));

		scene->AddInstance(meshInstance);
	}
}

void ModelComponent::Update(Scene* scene)
{
	if (model)
	{
		for (U32 i = 0; i < model->meshes.Size(); ++i)
		{
			Matrix4 mat = model->matrices[i] * modelMatrix * scene->GetEntity(entityID)->transform.WorldMatrix();

			Memory::Copy(model->meshes[i].instanceData.data + sizeof(U32), &mat, sizeof(Matrix4));

			scene->UpdateInstance(model->meshes[i]);
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

			Memory::Copy(model->meshes[i].instanceData.data + sizeof(U32), &mat, sizeof(Matrix4));

			scene->AddInstance(model->meshes[i]);
		}
	}
}