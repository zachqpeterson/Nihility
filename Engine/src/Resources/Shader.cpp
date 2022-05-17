#include "Shader.hpp"

#include "Core/Logger.hpp"
#include "Renderer/RendererFrontend.hpp"

void Shader::Destroy()
{
    RendererFrontend::DestroyShader(*this);

    state = SHADER_STATE_NOT_CREATED;

    name.Destroy();
}

bool Shader::AddAttribute(const ShaderAttributeConfig& config)
{
    U32 size = 0;
    switch (config.type)
    {
    case SHADER_ATTRIB_TYPE_INT8:
    case SHADER_ATTRIB_TYPE_UINT8:
        size = 1; break;
    case SHADER_ATTRIB_TYPE_INT16:
    case SHADER_ATTRIB_TYPE_UINT16:
        size = 2; break;
    case SHADER_ATTRIB_TYPE_FLOAT32:
    case SHADER_ATTRIB_TYPE_INT32:
    case SHADER_ATTRIB_TYPE_UINT32:
        size = 4; break;
    case SHADER_ATTRIB_TYPE_FLOAT32_2:
        size = 8; break;
    case SHADER_ATTRIB_TYPE_FLOAT32_3:
        size = 12; break;
    case SHADER_ATTRIB_TYPE_FLOAT32_4:
        size = 16; break;
    default:
        LOG_ERROR("Unrecognized type %d, defaulting to size of 4. This probably is not what is desired.");
        size = 4; break;
    }

    attributeStride += size;

    ShaderAttribute attrib = {};
    attrib.name = config.name;
    attrib.size = size;
    attrib.type = config.type;
    attributes.Push(attrib);

    return true;
}

bool Shader::AddSampler(const ShaderUniformConfig& config)
{
    if (config.scope == SHADER_SCOPE_INSTANCE && !useInstances)
    {
        LOG_ERROR("AddSampler cannot add an instance sampler for a shader that does not use instances.");
        return false;
    }

    if (config.scope == SHADER_SCOPE_LOCAL)
    {
        LOG_ERROR("AddSampler cannot add a sampler at local scope.");
        return false;
    }

    if (!UniformNameValid(config.name) || !UniformAddStateValid())
    {
        LOG_ERROR("AddSampler cannot add a sampler with duplicate name.");
        return false;
    }

    U32 location = 0;
    if (config.scope == SHADER_SCOPE_GLOBAL)
    {
        U32 globalTextureCount = globalTextures.Size();
        if (globalTextureCount == MAX_GLOBAL_TEXTURES)
        {
            LOG_ERROR("Shader global texture count %i exceeds max of %i", globalTextureCount, MAX_GLOBAL_TEXTURES);
            return false;
        }
        location = globalTextureCount;
        globalTextures.Push(Resources::DefaultTexture());
    }
    else
    {
        if (instanceTextureCount == MAX_INSTANCE_TEXTURES)
        {
            LOG_ERROR("Shader instance texture count %i exceeds max of %i", instanceTextureCount, MAX_INSTANCE_TEXTURES);
            return false;
        }
        location = instanceTextureCount;
        ++instanceTextureCount;
    }

    // Treat it like a uniform. NOTE: In the case of samplers, out_location is used to determine the
    // hashtable entry's 'location' field value directly, and is then set to the index of the uniform array.
    // This allows location lookups for samplers as if they were uniforms as well (since technically they are).
    // TODO: might need to store this elsewhere
    if (!UniformAdd(config.name, 0, config.type, config.scope, location, true))
    {
        LOG_ERROR("Unable to add sampler uniform.");
        return false;
    }

    return true;
}

bool Shader::AddUniform(const ShaderUniformConfig& config)
{
    if (!UniformAddStateValid() || !UniformNameValid(config.name)) { return; }
    return UniformAdd(config.name, config.size, config.type, config.scope, 0, false);
}

bool Shader::UniformAdd(const String& uniformName, U32 size, const ShaderUniformType& type, const ShaderScope& scope, U32 setLocation, bool isSampler)
{
    U32 uniformCount = uniforms.Size();
    if (uniformCount == MAX_UNIFORM_COUNT)
    {
        LOG_ERROR("A shader can only accept a combined maximum of %d uniforms and samplers at global, instance and local scopes.", MAX_UNIFORM_COUNT);
        return false;
    }

    ShaderUniform entry;
    entry.index = uniformCount;
    entry.scope = scope;
    entry.type = type;

    bool isGlobal = scope == SHADER_SCOPE_GLOBAL;

    if (isSampler) { entry.location = setLocation; }
    else { entry.location = entry.index; }

    if (scope != SHADER_SCOPE_LOCAL)
    {
        entry.setIndex = (U32)scope;
        entry.offset = isSampler ? 0 : isGlobal ? globalUboSize : uboSize;
        entry.size = isSampler ? 0 : size;
    }
    else
    {
        if (entry.scope == SHADER_SCOPE_LOCAL && !useLocals)
        {
            LOG_ERROR("Cannot add a locally-scoped uniform for a shader that does not support locals.");
            return false;
        }
        
        entry.setIndex = INVALID_ID_U8;
        Range r = AlignRange(pushConstantSize, size, 4);
        entry.offset = r.offset;
        entry.size = r.size;

        pushConstantRanges[pushConstantRangeCount] = r;
        ++pushConstantRangeCount;

        pushConstantSize += r.size;
    }

    uniformLookup.Set(uniformName, entry.index);
    uniforms.Push(entry);

    if (!isSampler)
    {
        if (entry.scope == SHADER_SCOPE_GLOBAL)
        {
            globalUboSize += entry.size;
        }
        else if (entry.scope == SHADER_SCOPE_INSTANCE)
        {
            uboSize += entry.size;
        }
    }

    return true;
}

bool Shader::UniformNameValid(const String& uniformName)
{
    if (!uniformName.Length())
    {
        LOG_ERROR("Uniform name must exist.");
        return false;
    }
    
    if (uniformLookup.Get(uniformName) != INVALID_ID_U16)
    {
        LOG_ERROR("A uniform by the name '%s' already exists on shader '%s'.", uniformName, name);
        return false;
    }
    return true;
}

bool Shader::UniformAddStateValid()
{
    if (state != SHADER_STATE_UNINITIALIZED)
    {
        LOG_ERROR("Uniforms may only be added to shaders before initialization.");
        return false;
    }

    return true;
}