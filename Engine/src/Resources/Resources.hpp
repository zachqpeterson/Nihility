#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Math/Math.hpp"

enum TextureUse
{
    TEXTURE_USE_UNKNOWN = 0x00,
    TEXTURE_USE_MAP_DIFFUSE = 0x01,
    TEXTURE_USE_MAP_SPECULAR = 0x02,
    TEXTURE_USE_MAP_NORMAL = 0x03
};

struct Binary
{
    String name;
    String path;
    Vector<U8> data;
};

struct Image
{
    String name;
    String path;
    U32 width;
    U32 height;
    U8 channelCount;
    Vector<U8> pixels;
};

struct Texture
{
    String name;
    String path;
    U32 id;
    U32 width;
    U32 height;
    U8 channelCount;
    bool hasTransparency;
    U32 generation;
    void* internalData;
};

struct TextureMap
{
    Texture* texture;
    TextureUse use;
};

struct MaterialConfig
{
    String name;
    String path;
    String shaderName;
    bool autoRelease;
    Vector4 diffuseColor; //TODO: Color struct
    F32 shininess;
    String diffuseMapName;
    String specularMapName;
    String normalMapName;
};

struct Material
{
    String name;
    String path;
    U32 id;
    U32 generation;
    U32 internalId;
    Vector4 diffuseColor; //TODO: Color struct
    TextureMap diffuseMap;
    TextureMap specularMap;
    TextureMap normalMap;
    F32 shininess;
    U32 shaderId;
    U32 renderFrameNumber;
};

struct Mesh
{
    String name;
    String path;
    U32 id;
    U32 internalId;
    U32 generation;
    Material* material;
};

struct Model
{
    Vector<Mesh*> meshes;
    Transform transform;
};

class Resources
{
public:
    static bool Initialize();
    static void Destroy();

    static NH_API Binary* LoadBinary(const String& name);
    static NH_API void UnloadBinary(Binary* resource);
    static NH_API Image* LoadImage(const String& name, const String& ext);
    static NH_API void UnloadImage(Image* resource);

    static NH_API struct Texture* DefaultTexture() { return defaultTexture; }

private:
    Resources() = delete;

    static Texture* defaultTexture;
};