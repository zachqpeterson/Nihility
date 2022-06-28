#pragma once

#include "Defines.hpp"

#include "Containers/Vector.hpp"
#include "Containers/String.hpp"
#include "Containers/Array.hpp"
#include "Containers/HashMap.hpp"

enum ShaderStageType
{
    SHADER_STAGE_VERTEX = 0x00000001,
    SHADER_STAGE_GEOMETRY = 0x00000002,
    SHADER_STAGE_FRAGMENT = 0x00000004,
    SHADER_STAGE_COMPUTE = 0x0000008
};

enum FieldType
{
    FIELD_TYPE_FLOAT32 = 0U,
    FIELD_TYPE_FLOAT32_2 = 1U,
    FIELD_TYPE_FLOAT32_3 = 2U,
    FIELD_TYPE_FLOAT32_4 = 3U,
    FIELD_TYPE_MATRIX_4 = 10U,
    FIELD_TYPE_INT8 = 4U,
    FIELD_TYPE_UINT8 = 5U,
    FIELD_TYPE_INT16 = 6U,
    FIELD_TYPE_UINT16 = 7U,
    FIELD_TYPE_INT32 = 8U,
    FIELD_TYPE_UINT32 = 9U,
    FIELD_TYPE_SAMPLER = 11U,
    FIELD_TYPE_CUSTOM = 255U
};

enum ShaderScope
{
    SHADER_SCOPE_GLOBAL,
    SHADER_SCOPE_INSTANCE,

    SHADER_SCOPE_MAX
};

struct Uniform
{
    String name;
    U64 offset;
    U16 location;
    U16 size;
    U8 setIndex;
    U8 bindingIndex;
    FieldType type;
};

struct Attribute
{
    String name;
    FieldType type;
    U16 size;
};

struct PushConstant
{
    String name;
    FieldType type;
    U16 size;
    U64 offset;
};

struct Shader
{
public:
    void Destroy();

    void AddAttribute(Attribute attribute);
    bool AddUniform(Uniform uniform);
    bool AddPushConstant(PushConstant pushConstant);

    bool ApplyGlobals(struct Camera* camera);
    bool ApplyMaterialInstance(struct Material* material, bool needsUpdate);
    bool ApplyMaterialLocal(struct Material* material, const Matrix4& model);

public:
    String name;

    bool useInstances;
    bool useLocals;
    I32 renderOrder;

    struct Renderpass* renderpass;

    Vector<ShaderStageType> stages;
    Vector<String> stageFilenames;

    void* internalData;

private:
    U16 attributeStride;
    Vector<Attribute> attributes;

    Array<Vector<Uniform>, 2> uniforms;
    
    U64 globalUboSize;
    U64 globalUboStride;
    U64 globalUboOffset;
    Vector<struct TextureMap*> globalTextureMaps;

    U64 instanceUboSize;
    U64 instanceUboStride;
    U64 requiredUboAlignment;

    U32 boundInstanceId;
    U8 instanceTextureCount;
    
    U64 pushConstantSize;
    U64 pushConstantStride;
    Vector<PushConstant> pushConstants;
    Vector<Range> pushConstantRanges;

    friend class VulkanShader;
};