#include "Mesh.hpp"

#include "Scene.hpp"
#include "Math\Math.hpp"

//void MeshComponent::Update(Scene* scene)
//{
//	//TODO: Check if update is needed
//	//if (meshInstance.mesh)
//	//{
//	//	Memory::Copy(meshInstance.instanceData.data + sizeof(U32) + sizeof(U32), &modelMatrix, sizeof(Matrix4));
//	//
//	//	scene->UpdateInstance(meshInstance);
//	//}
//}
//
//void MeshComponent::Load(Scene* scene)
//{
//	if (meshInstance.mesh)
//	{
//		Copy((U32*)meshInstance.instanceData.data + 1, &entityID, 1);
//		Copy((Matrix4*)(meshInstance.instanceData.data + sizeof(U64)), &modelMatrix, 1);
//
//		scene->AddInstance(meshInstance);
//	}
//}
//
//void ModelComponent::Update(Scene* scene)
//{
//	//TODO: Check if update is needed
//	//if (model)
//	//{
//	//	for (U32 i = 0; i < model->meshes.Size(); ++i)
//	//	{
//	//		Matrix4 mat = model->matrices[i] * modelMatrix;
//	//
//	//		Memory::Copy(model->meshes[i].instanceData.data + sizeof(U32) + sizeof(U32), &mat, sizeof(Matrix4));
//	//
//	//		scene->UpdateInstance(model->meshes[i]);
//	//	}
//	//}
//}
//
//void ModelComponent::Load(Scene* scene)
//{
//	if (model)
//	{
//		for (U32 i = 0; i < model->meshes.Size(); ++i)
//		{
//			Matrix4 mat = model->matrices[i] * modelMatrix;
//
//			Copy((U32*)model->meshes[i].instanceData.data + 1, &entityID, 1);
//			Copy((Matrix4*)(model->meshes[i].instanceData.data + sizeof(U64)), &mat, 1);
//
//			scene->AddInstance(model->meshes[i]);
//		}
//	}
//}