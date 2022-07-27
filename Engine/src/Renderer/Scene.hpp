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
    void NH_API Create(CameraType cameraType);
    void NH_API Destroy();

    void OnResize();
    bool OnRender(U64 frameNumber, U64 renderTargetIndex);

    void NH_API DrawMesh(struct Mesh* mesh, const Matrix4& matrix = Matrix4::IDENTITY);
    void NH_API DrawModel(struct Model* model, const Matrix4& matrix = Matrix4::IDENTITY);

    struct Camera* GetCamera() { return camera; }

private:
    struct Camera* camera;
    Vector<MaterialList> meshes;
};