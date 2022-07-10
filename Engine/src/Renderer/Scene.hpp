#pragma once

#include "Defines.hpp"

#include <Containers/List.hpp>
#include <Containers/Vector.hpp>

struct MaterialList
{
    struct Material* material;
    List<struct MeshRenderData> renderData;
};

class Scene
{
public:
    void NH_API Create();
    void NH_API Destroy();

    void OnResize();
    bool OnRender(U64 frameNumber, U64 renderTargetIndex);

    void NH_API DrawMesh(struct Mesh* mesh, const struct Matrix4& model);
    void NH_API DrawModel(struct Model* model);

private:
    struct Camera* camera;
    Vector<MaterialList> meshes;
};