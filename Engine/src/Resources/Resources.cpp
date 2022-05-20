#include "Resources.hpp"

#include "Shader.hpp"

#include "Memory/Memory.hpp"
#include "Core/File.hpp"
#include "Renderer/RendererFrontend.hpp"

#undef LoadImage

Texture* Resources::defaultTexture;
Texture* Resources::defaultDiffuse;
Texture* Resources::defaultSpecular;
Texture* Resources::defaultNormal;

#define BINARIES_PATH "../assets/binaries/";
#define TEXTURES_PATH "../assets/textures/";
#define MATERIALS_PATH "../assets/materials/";
#define MODELS_PATH "../assets/models/";
#define SHADERS_PATH "../assets/shaders/";

#pragma pack(push, 1)
struct BMPInfo
{
    U32 biSize;
    I32 biWidth;
    I32 biHeight;
    U16 biPlanes;
    U16 biBitCount;
    U32 biCompression;
    U32 biSizeImage;
    I32 biXPelsPerMeter;
    I32 biYPelsPerMeter;
    U32 biClrUsed;
    U32 biClrImportant;
};

struct BMPHeader
{
    U16 bfType;
    U32 bfSize;
    U16 bfReserved1;
    U16 bfReserved2;
    U32 bfOffBits;
};

struct TGAHeader
{
    U8  idlength;
    U8  colourmaptype;
    U8  datatypecode;
    I16 colourmaporigin;
    I16 colourmaplength;
    U8  colourmapdepth;
    I16 x_origin;
    I16 y_origin;
    I16 width;
    I16 height;
    U8  bitsperpixel;
    U8  imagedescriptor;
};
#pragma pack(pop)

bool Resources::Initialize()
{
    //TODO: Load defaults
    defaultTexture = LoadTexture("defaultTexture.bmp", IMAGE_TYPE_BMP);

    return true;
}

void Resources::Shutdown()
{
    UnloadTexture(defaultTexture);
}

Binary* Resources::LoadBinary(const String& name)
{
    Binary* resource = (Binary*)Memory::Allocate(sizeof(Binary), MEMORY_TAG_RESOURCE);
    resource->path = BINARIES_PATH;
    resource->path.Append(name);
    resource->name = name;

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(resource->path, FILE_MODE_READ, true))
    {
        U64 size;
        resource->data.SetArray(file->ReadAllBytes(size), size, MEMORY_TAG_RESOURCE);
        file->Close();
    }
    else
    {
        LOG_ERROR("Couldn't open file: %s", (const char*)name);
    }

    return resource;
}

void Resources::UnloadBinary(Binary* resource)
{
    resource->name.Destroy();
    resource->path.Destroy();
    resource->data.Destroy();
    resource = nullptr;
}

Image* Resources::LoadImage(const String& name, ImageType type)
{
    Image* resource = (Image*)Memory::Allocate(sizeof(Image), MEMORY_TAG_RESOURCE);
    resource->path = TEXTURES_PATH;
    resource->path.Append(name);
    resource->name = name;
    resource->channelCount = 4;

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(resource->path, FILE_MODE_READ, true))
    {
        switch (type)
        {
        case IMAGE_TYPE_BMP:
            LoadBMP(resource, file);
            break;
        case IMAGE_TYPE_PNG:
            LoadPNG(resource, file);
            break;
        case IMAGE_TYPE_JPG:
            LoadJPG(resource, file);
            break;
        case IMAGE_TYPE_TGA:
            LoadTGA(resource, file);
            break;
        }

        file->Close();
    }
    else { LOG_ERROR("Couldn't open file: %s", (const char*)name); }

    return resource;
}

void Resources::UnloadImage(Image* resource)
{
    resource->name.Destroy();
    resource->path.Destroy();
    resource->pixels.Destroy();
    resource = nullptr;
}

void Resources::LoadBMP(Image* image, File* file)
{
    BMPHeader* header = (BMPHeader*)file->ReadBytes(sizeof(BMPHeader));

    if (header->bfType != 0x4D42)
    {
        LOG_ERROR("Image file: %s is not a BMP!", (const char*)image->name);
        file->Close();
        return;
    }

    BMPInfo* info = (BMPInfo*)file->ReadBytes(sizeof(BMPInfo));
    image->width = info->biWidth;
    image->height = info->biHeight;

    file->Seek(header->bfOffBits);
    image->pixels.SetArray(file->ReadBytes(info->biSizeImage), info->biSizeImage, MEMORY_TAG_RESOURCE);

    //TODO: Test speed of both methods
    U8 temp;
    for (auto it0 = image->pixels.begin(), it1 = it0 + 2; it0 != image->pixels.end(); it0 += 4, it1 += 4)
    {
        temp = *it0;
        *it0 = *it1;
        *it1 = temp;
    }

    //U8 temp;
    //for (U32 i = 0; i < info->biSizeImage; i += 4)
    //{
    //    temp = image->pixels[i];
    //    image->pixels[i] = image->pixels[i + 2];
    //    image->pixels[i + 2] = temp;
    //}
}

void Resources::LoadPNG(Image* image, File* file)
{
    LOG_ERROR("The PNG file format is not supported.");
}

void Resources::LoadJPG(Image* image, File* file)
{
    LOG_ERROR("The JPG file format is not supported.");
}

void Resources::LoadTGA(Image* image, File* file)
{
    TGAHeader* header = (TGAHeader*)file->ReadBytes(sizeof(TGAHeader));

    if (!header)
    {
        LOG_ERROR("Image file: %s is not a TGA!", (const char*)image->name);
        file->Close();
        return;
    }

    LOG_ERROR("The TGA file format is not supported.");

    image->width = header->width;
    image->height = header->height;
}

Texture* Resources::LoadTexture(const String& name, ImageType type)
{
    Image* image = LoadImage(name, type);

    Texture* texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_TEXTURE);

    texture->name = image->name;
    texture->path = image->path;
    texture->width = image->width;
    texture->height = image->height;
    texture->generation = 0;
    texture->channelCount = image->channelCount;
    texture->hasTransparency = false;

    U64 size = texture->width * texture->height * texture->channelCount;
    for (int i = 0; i < size; i += 4)
    {
        if (image->pixels[i] < 255)
        {
            texture->hasTransparency = true;
            break;
        }
    }

    RendererFrontend::CreateTexture(image->pixels, texture);

    UnloadImage(image);

    return texture;
}

void Resources::UnloadTexture(Texture* resource)
{
    RendererFrontend::DestroyTexture(resource);

    resource->name.Destroy();
    resource->path.Destroy();
    Memory::Free(resource, sizeof(Texture), MEMORY_TAG_TEXTURE);
    resource = nullptr;
}
