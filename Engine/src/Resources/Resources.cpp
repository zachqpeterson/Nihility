#include "Resources.hpp"

#include "Texture.hpp"
#include "Memory/Memory.hpp"

Texture* Resources::defaultTexture;


Resource* Resources::Load(const String& path, ResourceType type)
{
    Resource* resource = (Resource*)Memory::Allocate(sizeof(Resource), MEMORY_TAG_RESOURCE);
    resource->path = path;

    return resource;
}

void Resources::Unload(Resource* resource)
{
    Memory::Zero(resource->data, resource->size);
    resource->name.Destroy();
    resource->path.Destroy();
}