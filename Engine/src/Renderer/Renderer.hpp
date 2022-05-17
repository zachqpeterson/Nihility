#pragma once

#include "Defines.hpp"

#include "Resources/Shader.hpp"

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

    virtual bool CreateShader(const Shader& shader, U8 renderpassId, U8 stageCount, const Vector<String>& stageFilenames, const Vector<ShaderStage>& stages) = 0;
    virtual void DestroyShader(const Shader& shader) = 0;
    virtual bool InitializeShader(const Shader& shader) = 0;
    virtual bool UseShader(const Shader& shader) = 0;
    virtual bool BindGlobals(const Shader& shader) = 0;
    virtual bool BindInstance(const Shader& shader, U32 instanceId) = 0;
    virtual bool ApplyGlobals(const Shader& shader) = 0;
    virtual bool ApplyInstance(const Shader& shader, bool needsUpdate) = 0;
    virtual U32 AcquireInstanceResources(const Shader& shader) = 0;
    virtual bool ReleaseInstanceResources(const Shader& shader, U32 instanceId) = 0;
    virtual bool SetUniform(const Shader& shader, const ShaderUniform& uniform, const void* value) = 0;

    void operator delete(void* p);

public:
    U64 frameNumber;
};