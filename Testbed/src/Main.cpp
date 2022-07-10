#include <Engine.hpp>

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Math/Math.hpp>
#include <Core/Logger.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Resources/Resources.hpp>

bool init()
{
    Scene* scene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
    scene->Create();

    MeshConfig config;
    config.name = "Mesh";
    config.MaterialName = "Tile.mat";

    config.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f }, });
    config.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f }, });
    config.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f }, });
    config.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f }, });
    
    config.indices.Push(0);
    config.indices.Push(1);
    config.indices.Push(2);
    config.indices.Push(2);
    config.indices.Push(3);
    config.indices.Push(0);

    Mesh* mesh = Resources::CreateMesh(config);
    scene->DrawMesh(mesh, Matrix4::IDENTITY);

    RendererFrontend::UseScene(scene);

    return true;
}

bool update()
{
    return true;
}

void cleanup()
{
    
}

int main()
{
    Engine::Initialize("TestBed", init, update, cleanup);
}