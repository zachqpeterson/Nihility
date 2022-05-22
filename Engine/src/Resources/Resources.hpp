#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Math/Math.hpp"

#undef LoadImage

enum ImageType
{
    IMAGE_TYPE_BMP,
    IMAGE_TYPE_PNG,
    IMAGE_TYPE_JPG,
    IMAGE_TYPE_TGA,
};

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
    U32 width;
    U32 height;
    U8 channelCount;

    U32 id;
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
    Transform3 transform;
};

class Resources
{
public:
    static bool Initialize();
    static void Shutdown();

    static NH_API Binary* LoadBinary(const String& name);
    static NH_API void UnloadBinary(Binary* resource);
    static NH_API Image* LoadImage(const String& name, ImageType type);
    static NH_API void UnloadImage(Image* resource);
    static NH_API Texture* LoadTexture(const String& name, ImageType type);
    static NH_API void UnloadTexture(Texture* resource);

    static NH_API struct Texture* DefaultTexture() { return defaultTexture; }
    static NH_API struct Texture* DefaultDiffuse() { return defaultDiffuse; }
    static NH_API struct Texture* DefaultSpecular() { return defaultSpecular; }
    static NH_API struct Texture* DefaultNormal() { return defaultNormal; }

private:
    Resources() = delete;

    static void LoadBMP(Image* image, struct File* file);
    static void LoadPNG(Image* image, File* file);
    static void LoadJPG(Image* image, File* file);
    static void LoadTGA(Image* image, File* file);

    static Texture* defaultTexture;
    static Texture* defaultDiffuse;
    static Texture* defaultSpecular;
    static Texture* defaultNormal;
};