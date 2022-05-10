#pragma once

#include "Defines.hpp"

#include "Renderer/Renderer.hpp"

class VulkanRenderer : public Renderer
{
public:
    bool Initialize() final;
    void Shutdown() final;

    void* operator new(U64 size);
    void operator delete(void* p);



private: //VULKAN SPECIFIC FUNCTIONS

};