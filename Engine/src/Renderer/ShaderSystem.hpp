#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"

struct ShaderSystemConfig
{
    U16 maxShaderCount;
    U8 maxUniformCount;
    U8 maxGlobalTextures;
    U8 maxInstanceTextures;
};

enum ShaderState
{
    SHADER_STATE_NOT_CREATED,
    SHADER_STATE_UNINITIALIZED,
    SHADER_STATE_INITIALIZED,
};

struct ShaderUniform
{
    U64 offset;
    U16 location;
    U16 index;
    U16 size;
    U8 setIndex;
    ShaderScope scope;
    ShaderUniformType type;
};

struct ShaderAttribute
{
    String name;
    ShaderAttributeType type;
    U32 size;
};

struct Shader
{
    U32 id;

    String name;
    bool useInstances;
    bool useLocals;

    U64 requiredUboAlignment;

    U64 globalUboSize;
    U64 globalUboStride;
    U64 globalUboOffset;

    U64 uboSize;

    U64 uboStride;

    U64 pushConstantSize;
    U64 pushConstantStride;

    Vector<Texture*> globalTextures;

    U8 instanceTextureCount;

    ShaderScope boundScope;

    U32 boundInstanceId;
    U32 boundUboOffset;

    Hashtable uniformLookup;

    Vector<ShaderUniform> uniforms;

    Vector<ShaderAttribute> attributes;

    ShaderState state;

    U8 pushConstantRangeCount;
    range pushConstantRanges[32];
    U16 attributeStride;

    void* internalData;
};

class ShaderSystem
{
public:
    bool Initialize();
    
private:

};