#pragma once

#include "Math/Math.hpp"
#include "Resources/Resources.hpp"

struct MeshRenderData
{
    Matrix4 model = Matrix4::IDENTITY;
    Mesh* mesh = nullptr;
};