#pragma once

#include "Defines.hpp"

enum ShaderStageType
{
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_FRAGMENT,
    SHADER_STAGE_GEOMETRY,
    SHADER_STAGE_COMPUTE
};

class Renderer
{
public:
    virtual ~Renderer() {};

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual bool BeginFrame() = 0;
    virtual bool EndFrame() = 0;
    virtual bool BeginRenderpass(U8 renderpassId) = 0;
    virtual bool EndRenderpass(U8 renderpassId) = 0;
    virtual void DrawMesh() = 0;

    void operator delete(void* p);

public:
    U64 frameNumber;
};