#pragma once

#include "Defines.hpp"

#include "Containers/Vector.hpp"
#include "Containers/String.hpp"
#include "Containers/Hashtable.hpp"

//TODO: Don't hard code
#define MAX_SHADER_COUNT 1024u
#define MAX_UNIFORM_COUNT 128u
#define MAX_GLOBAL_TEXTURES 31u
#define MAX_INSTANCE_TEXTURES 31u

enum ShaderStageType
{
    SHADER_STAGE_VERTEX = 0x00000001,
    SHADER_STAGE_GEOMETRY = 0x00000002,
    SHADER_STAGE_FRAGMENT = 0x00000004,
    SHADER_STAGE_COMPUTE = 0x0000008
};

enum ShaderAttributeType
{
    SHADER_ATTRIB_TYPE_FLOAT32 = 0U,
    SHADER_ATTRIB_TYPE_FLOAT32_2 = 1U,
    SHADER_ATTRIB_TYPE_FLOAT32_3 = 2U,
    SHADER_ATTRIB_TYPE_FLOAT32_4 = 3U,
    SHADER_ATTRIB_TYPE_MATRIX_4 = 4U,
    SHADER_ATTRIB_TYPE_INT8 = 5U,
    SHADER_ATTRIB_TYPE_UINT8 = 6U,
    SHADER_ATTRIB_TYPE_INT16 = 7U,
    SHADER_ATTRIB_TYPE_UINT16 = 8U,
    SHADER_ATTRIB_TYPE_INT32 = 9U,
    SHADER_ATTRIB_TYPE_UINT32 = 10U,
};

enum ShaderUniformType
{
    SHADER_UNIFORM_TYPE_FLOAT32 = 0U,
    SHADER_UNIFORM_TYPE_FLOAT32_2 = 1U,
    SHADER_UNIFORM_TYPE_FLOAT32_3 = 2U,
    SHADER_UNIFORM_TYPE_FLOAT32_4 = 3U,
    SHADER_UNIFORM_TYPE_INT8 = 4U,
    SHADER_UNIFORM_TYPE_UINT8 = 5U,
    SHADER_UNIFORM_TYPE_INT16 = 6U,
    SHADER_UNIFORM_TYPE_UINT16 = 7U,
    SHADER_UNIFORM_TYPE_INT32 = 8U,
    SHADER_UNIFORM_TYPE_UINT32 = 9U,
    SHADER_UNIFORM_TYPE_MATRIX_4 = 10U,
    SHADER_UNIFORM_TYPE_SAMPLER = 11U,
    SHADER_UNIFORM_TYPE_CUSTOM = 255U
};

enum ShaderScope
{
    SHADER_SCOPE_GLOBAL = 0,
    SHADER_SCOPE_INSTANCE = 1,
    SHADER_SCOPE_LOCAL = 2
};

enum ShaderState
{
    SHADER_STATE_NOT_CREATED,
    SHADER_STATE_UNINITIALIZED,
    SHADER_STATE_INITIALIZED,
};

struct ShaderAttributeConfig
{
    String name;
    U8 size;
    ShaderAttributeType type;
};

struct ShaderUniformConfig
{
    String name;
    U8 size;
    U32 location;
    ShaderUniformType type;
    ShaderScope scope;
};

struct ShaderConfig
{
    String name;

    bool useInstances;
    bool useLocal;

    U8 attributeCount;
    Vector<ShaderAttributeConfig> attributes;

    U8 uniformCount;
    Vector<ShaderUniformConfig> uniforms;

    String renderpassName;

    U8 stageCount;
    Vector<ShaderStageType> stages;
    Vector<String> stageNames;
    Vector<String> stageFilenames;
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
public:
    void Destroy();

    bool AddAttribute(const ShaderAttributeConfig& config);
    bool AddSampler(const ShaderUniformConfig& config);
    bool AddUniform(const ShaderUniformConfig& config);

public:
    bool UniformAdd(const String& uniformName, U32 size, const ShaderUniformType& type, const ShaderScope& scope, U32 setLocation, bool isSampler);
    bool UniformNameValid(const String& uniformName);
    bool UniformAddStateValid();

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

    Vector<struct Texture*> globalTextures;

    U8 instanceTextureCount;

    ShaderScope boundScope;

    U32 boundInstanceId;
    U32 boundUboOffset;

    Hashtable<String, U16> uniformLookup;

    Vector<ShaderUniform> uniforms;

    Vector<ShaderAttribute> attributes;

    ShaderState state;

    U8 pushConstantRangeCount;
    Range pushConstantRanges[32];
    U16 attributeStride;

    void* internalData;
};