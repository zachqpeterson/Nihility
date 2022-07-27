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

struct MaterialList
{
    struct Material* material;
    List<struct MeshRenderData> renderData;
};

class Scene
{
public:
    NH_API void Create(CameraType cameraType);
    NH_API void Destroy();

    void OnResize();
    bool OnRender(U64 frameNumber, U64 renderTargetIndex);

    NH_API void DrawMesh(struct Mesh* mesh, const Matrix4& matrix = Matrix4::IDENTITY);
    NH_API void DrawModel(struct Model* model, const Matrix4& matrix = Matrix4::IDENTITY);
    NH_API void DrawGameObject(struct GameObject2D* gameObject);

    struct Camera* GetCamera() { return camera; }

private:
    struct Camera* camera;
    Vector<MaterialList> meshes;
};