#include "Resources.hpp"

#include "Shader.hpp"

#include "Memory/Memory.hpp"
#include "Core/File.hpp"

Texture* Resources::defaultTexture;

#define BINARIES_PATH "../assets/binaries/";
#define TEXTURES_PATH "../assets/textures/";
#define MATERIALS_PATH "../assets/materials/";
#define MODELS_PATH "../assets/models/";
#define SHADERS_PATH "../assets/shaders/";

Binary* Resources::LoadBinary(const String& name)
{
    Binary* resource = (Binary*)Memory::Allocate(sizeof(Binary), MEMORY_TAG_RESOURCE);
    resource->path = BINARIES_PATH;
    resource->path.Append(name);
    resource->name = name;

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(resource->path, FILE_MODE_READ, true))
    {
        resource->data = file->ReadAllBytes();
        file->Close();
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

Image* Resources::LoadImage(const String& name, const String& ext)
{
    Image* resource = (Image*)Memory::Allocate(sizeof(Image), MEMORY_TAG_RESOURCE);
    resource->path = TEXTURES_PATH;
    resource->path.Append(name);
    resource->name = name;

        
}

void Resources::UnloadImage(Image* resource)
{

}