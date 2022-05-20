#pragma once

#include "Defines.hpp"
#include "Resources/Shader.hpp"

enum BuiltinRenderpass
{
    BUILTIN_RENDERPASS_WORLD = 0x01,
    BUILTIN_RENDERPASS_UI = 0x02
};

class RendererFrontend
{
public:
    static bool Initialize();
    static void Shutdown();

    static bool DrawFrame();

    static bool CreateTexture(const Vector<U8>& pixels, struct Texture* texture);
    static bool DestroyTexture(Texture* texture);

    static U8 GetRenderpassId(const String& name);
    static bool CreateShader(const Shader& shader, U8 renderpassId, U8 stageCount, const Vector<String>& stage_filenames, const Vector<ShaderStageType>& stages);
    static void DestroyShader(const Shader& shader);
    static bool InitializeShader(Shader& shader);
    static bool UseShader(const Shader& shader);
    static bool BindGlobals(const Shader& shader);
    static bool BindInstance(const Shader& shader, U32 instanceId);
    static bool ApplyGlobals(const Shader& shader);
    static bool ApplyInstance(const Shader& shader, bool needsUpdate);
    static U32 AcquireInstanceResources(const Shader& shader);
    static bool ReleaseInstanceResources(const Shader& shader, U32 instanceId);
    static bool SetUniform(Shader& shader, const ShaderUniform& uniform, const void* value);

private:
    RendererFrontend() = delete;

    static class Renderer* renderer;
};