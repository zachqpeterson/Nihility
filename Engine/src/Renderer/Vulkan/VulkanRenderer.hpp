#pragma once

#include "VulkanDefines.hpp"

#include "Renderer/Renderer.hpp"
#include "Containers/Vector.hpp"

class VulkanRenderer : public Renderer
{
public:
    bool Initialize() final;
    void Shutdown() final;

    void* operator new(U64 size);
    void operator delete(void* p);



private: //VULKAN SPECIFIC FUNCTIONS
    bool CreateInstance();
    bool CreateDebugger();
    bool CreateSurface();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    void GetPlatformExtentions(Vector<const char*>* names);
    static I32 FindMemoryIndex(U32 memoryTypeBits, VkMemoryPropertyFlags memoryFlags);

    //Runtime
    void RecreateFramebuffers();
};