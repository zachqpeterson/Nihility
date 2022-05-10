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
    void GetPlatformExtentions(Vector<struct String>* names);
};