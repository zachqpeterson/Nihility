#include "Scene.hpp"

#include "Camera.hpp"
#include "RendererFrontend.hpp"

#include "Resources/Resources.hpp"
#include "Core/Settings.hpp"

void Scene::Create()
{
    for (Material* material : Resources::GetMaterials())
    {
        MaterialList list{};
        list.material = material;
        meshes.Push(list);
    }

    //TODO: Configure
    if (Settings::WindowWidth > (Settings::WindowHeight * 1.77777777778f))
    {
        F32 camHeight = 0.9375f;
        F32 camWidth = camHeight * ((F32)Settings::WindowWidth / (F32)Settings::WindowHeight);

        camera = new Camera({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f, { 0.0f, 0.0f, -10.0f });
    }
    else
    {
        F32 camWidth = 1.66666666667f;
        F32 camHeight = camWidth * ((F32)Settings::WindowHeight / (F32)Settings::WindowWidth);

        camera = new Camera({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f, { 0.0f, 0.0f, -10.0f });
    }
}

void Scene::Destroy()
{

}

void Scene::OnResize()
{
    //TODO: Configure
    if (Settings::WindowWidth > (Settings::WindowHeight * 1.77777777778f))
    {
        F32 camHeight = 0.9375f;
        F32 camWidth = camHeight * ((F32)Settings::WindowWidth / (F32)Settings::WindowHeight);

        camera->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);
    }
    else
    {
        F32 camWidth = 1.66666666667f;
        F32 camHeight = camWidth * ((F32)Settings::WindowHeight / (F32)Settings::WindowWidth);

        camera->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);
    }
}

bool Scene::OnRender(U64 frameNumber, U64 renderTargetIndex)
{
    String lastShaderName;

    for (MaterialList& list : meshes)
    {
        if (list.material->shader->name != lastShaderName)
        {
            if (!lastShaderName.Blank())
            {
                if (!RendererFrontend::EndRenderpass(list.material->shader->renderpass))
                {
                    Logger::Error("renderpass '{}' failed to end.", list.material->shader->renderpass->name);
                    return false;
                }
            }

            lastShaderName = list.material->shader->name;

            if (!RendererFrontend::BeginRenderpass(list.material->shader->renderpass))
            {
                Logger::Error("renderpass '{}' failed to start.", list.material->shader->renderpass->name);
                return false;
            }

            if (list.renderData.Size())
            {
                if (!RendererFrontend::UseShader(list.material->shader))
                {
                    Logger::Error("Failed to use material shader. Render frame failed.");
                    return false;
                }

                if (!list.material->shader->ApplyGlobals(camera))
                {
                    Logger::Error("Failed to use apply globals for material shader. Render frame failed.");
                    return false;
                }
            }
        }

        for (const MeshRenderData& data : list.renderData)
        {
            Material* material;
            if (data.mesh->material)
            {
                material = data.mesh->material;
            }
            else
            {
                material = Resources::DefaultMaterial();
            }

            bool needsUpdate = material->renderFrameNumber != frameNumber;

            if (!material->shader->ApplyMaterialInstance(material, needsUpdate))
            {
                Logger::Warn("Failed to apply material '{}'. Skipping draw.", material->name);
                continue;
            }

            material->renderFrameNumber = frameNumber;

            if (!material->shader->ApplyMaterialLocal(material, data.model))
            {
                Logger::Warn("Failed to apply material '{}'. Skipping draw.", material->name);
                continue;
            }

            RendererFrontend::DrawMesh(data);
        }
    }

    if (!RendererFrontend::EndRenderpass(meshes.Back().material->shader->renderpass))
    {
        Logger::Error("renderpass '{}' failed to end.", meshes.Back().material->shader->renderpass->name);
        return false;
    }

    return true;
}

void NH_API Scene::DrawMesh(Mesh* mesh, const struct Matrix4& model)
{
    MeshRenderData data;
    data.mesh = mesh;
    data.model = model;

    meshes[mesh->material->id].renderData.PushBack(data);
}

void NH_API Scene::DrawModel(Model* model)
{
    Matrix4 matrix = model->transform.World();

    for (Mesh* m : model->meshes)
    {
        MeshRenderData data;
        data.mesh = m;
        data.model = matrix;

        meshes[m->material->id].renderData.PushBack(data);
    }
}
