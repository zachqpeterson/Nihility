#include "ShaderSystem.hpp"

#include "Renderer/RendererFrontend.hpp"

#include "Containers/String.hpp"
#include "Resources/Texture.hpp"
#include "Resources/Shader.hpp"

Hashtable<struct String, U32> ShaderSystem::lookup;
Vector<struct Shader> ShaderSystem::shaders;
U32 ShaderSystem::currentShaderId;

void ShaderSystem::Initialize()
{
    lookup = Move(Hashtable<String, U32>(MAX_SHADER_COUNT));
    shaders.Resize(MAX_SHADER_COUNT);

    lookup.Fill(INVALID_ID);
    for (Shader& s : shaders) { s.id = INVALID_ID; }
}

void ShaderSystem::Shutdown()
{
    for (Shader& s : shaders) { s.Destroy(); }
    shaders.Destroy();
    lookup.Destroy();
}

bool ShaderSystem::CreateShader(const ShaderConfig& config)
{
    U32 id = NewShaderId();
    if (id == INVALID_ID)
    {
        LOG_ERROR("Unable to find free slot to create new shader. Aborting.");
        return false;
    }

    Shader& outShader = shaders[id];
    Memory::Zero(&outShader, sizeof(Shader));
    outShader.id = id;
    outShader.state = SHADER_STATE_NOT_CREATED;
    outShader.name = config.name;
    outShader.useInstances = config.useInstances;
    outShader.useLocals = config.useLocal;
    outShader.pushConstantRangeCount = 0;
    Memory::Zero(outShader.pushConstantRanges, sizeof(Range) * 32);
    outShader.boundInstanceId = INVALID_ID;
    outShader.attributeStride = 0;

    U64 elementCount = 1024;
    outShader.uniformLookup = Move(Hashtable<String, U16>(elementCount));

    outShader.uniformLookup.Fill(INVALID_ID_U16);

    outShader.globalUboSize = 0;
    outShader.uboSize = 0;

    outShader.pushConstantStride = 128;
    outShader.pushConstantSize = 0;

    U8 renderpassId;
    if (renderpassId = RendererFrontend::GetRenderpassId(config.renderpassName) == INVALID_ID_U8)
    {
        LOG_ERROR("Unable to find renderpass '%s'", config.renderpassName);
        return false;
    }

    if(!RendererFrontend::CreateShader(outShader, renderpassId, config.stageCount, config.stageFilenames, config.stages))
    {
        LOG_ERROR("Unable to create shader: %", outShader.name);
        return false;
    }

    outShader.state = SHADER_STATE_UNINITIALIZED;

    for (U32 i = 0; i < config.attributeCount; ++i)
    {
        outShader.AddAttribute(config.attributes[i]);
    }

    for (U32 i = 0; i < config.uniformCount; ++i)
    {
        if (config.uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER)
        {
            outShader.AddSampler(config.uniforms[i]);
        }
        else
        {
            outShader.AddSampler(config.uniforms[i]);
        }
    }

    if (!RendererFrontend::InitializeShader(outShader))
    {
        LOG_ERROR("shader_system_create: initialization failed for shader '%s'.", config.name);
        return false;
    }

    lookup.Set(config.name, outShader.id);
    //TODO: renderer_shader_destroy(outShader) if failed

    return true;
}

void ShaderSystem::UseShader(const String& name)
{
    U32 id = GetShaderId(name);
    if (id == INVALID_ID) { LOG_ERROR("No shader of name: %s", name); return; }

    UseShader(id);
}

void ShaderSystem::UseShader(U32 id)
{
    if (currentShaderId != id)
    {
        Shader& nextShader = GetShaderById(id);
        currentShaderId = id;
        if (!RendererFrontend::UseShader(nextShader))
        {
            LOG_ERROR("Failed to use shader '%s'.", nextShader.name);
            return;
        }

        if (!RendererFrontend::BindGlobals(nextShader))
        {
            LOG_ERROR("Failed to bind globals for shader '%s'.", nextShader.name);
            return;
        }
    }

    return;
}

U16 ShaderSystem::UniformIndex(const Shader& shader, const String& uniformName)
{
    if (shader.id == INVALID_ID)
    {
        LOG_ERROR("UniformIndex called with invalid shader.");
        return INVALID_ID_U16;
    }

    U16 index = INVALID_ID_U16;
    if (index = shader.uniformLookup[uniformName] == INVALID_ID_U16)
    {
        LOG_ERROR("Shader '%s' does not have a registered uniform named '%s'", shader.name, uniformName);
        return INVALID_ID_U16;
    }

    return shader.uniforms[index].index;
}

bool ShaderSystem::SetUniform(const String& uniformName, const void* value)
{
    if (currentShaderId == INVALID_ID)
    {
        LOG_ERROR("UniformSet called without a shader in use.");
        return;
    }

    Shader& s = shaders[currentShaderId];
    U16 index = UniformIndex(s, uniformName);
    return SetUniformByIndex(index, value);
}

void ShaderSystem::SetSampler(const String& samplerName, const Texture& t)
{
    SetUniform(samplerName, &t);
}

bool ShaderSystem::SetUniformByIndex(U16 index, const void* value)
{
    Shader& shader = shaders[currentShaderId];
    ShaderUniform& uniform = shader.uniforms[index];

    if (shader.boundScope != uniform.scope)
    {
        if (uniform.scope == SHADER_SCOPE_GLOBAL)
        {
            RendererFrontend::BindGlobals(shader);
        }
        else if (uniform.scope == SHADER_SCOPE_INSTANCE)
        {
            RendererFrontend::BindInstance(shader, shader.boundInstanceId);
        }

        shader.boundScope = uniform.scope;
    }

    return RendererFrontend::SetUniform(shader, uniform, value);
}

bool ShaderSystem::SetSamplerByIndex(U16 index, const Texture& t)
{
    return SetUniformByIndex(index, &t);
}

bool ShaderSystem::ApplyGlobal()
{
    return RendererFrontend::ApplyGlobals(shaders[currentShaderId]);
}

bool ShaderSystem::ApplyInstance(bool needsUpdate)
{
    return RendererFrontend::ApplyInstance(shaders[currentShaderId], needsUpdate);
}

bool ShaderSystem::BindInstance(U32 instanceId)
{
    Shader& s = shaders[currentShaderId];
    s.boundInstanceId = instanceId;
    return RendererFrontend::BindInstance(s, instanceId);
}

U32 ShaderSystem::GetShaderId(const String& name)
{
    return lookup[name];
}

Shader& ShaderSystem::GetShaderById(U32 id)
{
    return shaders[id];
}

Shader& ShaderSystem::GetShaderByName(const String& name)
{
    shaders[GetShaderId(name)];
}

U32 ShaderSystem::NewShaderId()
{
    U32 i = 0;
    for (Shader& s : shaders)
    {
        if (s.id == INVALID_ID) { return i; }
        ++i;
    }

    return INVALID_ID;
}