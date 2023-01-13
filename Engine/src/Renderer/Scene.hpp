#pragma once

#include "Defines.hpp"

#include <Containers/List.hpp>
#include <Containers/Vector.hpp>
#include "Math/Math.hpp"

enum CameraType
{
	CAMERA_TYPE_ORTHOGRAPHIC,
	CAMERA_TYPE_PERSPECTIVE
};

struct Material;
struct MeshRenderData;

struct MaterialList
{
	Material* material;
	Vector<MeshRenderData> renderData;
	U64 meshCount;
};

struct GameObject2D;
struct UIElement;
struct Model;
struct Camera;

class Scene
{
public:
	NH_API void Create(CameraType cameraType);
	NH_API void Destroy();

	bool OnRender(U64 frameNumber, U64 renderTargetIndex);

	NH_API void DrawGameObject(GameObject2D* gameObject);
	NH_API void UndrawGameObject(GameObject2D* gameObject);
	NH_API void DrawModel(Model* model);
	NH_API void UndrawModel(Model* model);

	Camera* GetCamera() { return camera; }

private:
	Camera* camera;
	Vector<MaterialList> meshes;
	List<GameObject2D*> gameObjects;
	List<Model*> models;
	List<UIElement*> elements;

	friend class UI;
};