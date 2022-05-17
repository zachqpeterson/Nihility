#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"

enum ResourceType
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_MESH,
    RESOURCE_TYPE_CUSTOM
};

struct Resource
{
    U32 loaderId;
    String name;
    String path;
    U64 size;
    void* data;
};

class Resources
{
public:
    static bool Initialize();
    static void Destroy();

    static NH_API Resource Load(const String& path, ResourceType type);
    static NH_API void Unload(Resource& resource);

    //TODO: Default texture

private:
    Resources() = delete;
};